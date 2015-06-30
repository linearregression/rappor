// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <stdio.h>
#include <vector>

#include "rappor.pb.h"
#include "rappor.h"
#include "libc_rand.h"
#include "openssl_impl.h"

// Like atoi, but with basic (not exhaustive) error checking.
bool StringToInt(const char* s, int* result) {
  bool success = true;
  char* end;  // mutated by strtol

  *result = strtol(s, &end, 10);  // base 10
  // If strol didn't consume any characters, it failed.
  if (end == s) {
    success = false;
  }
  return success;
}

// Assuming the client string is an integer like "42", return the cohort.
int GetCohort(const std::string& client_str, int num_cohorts) {
  int client;
  bool success = StringToInt(client_str.c_str(), &client);
  if (!success) {
    return -1;  // error signaled by invalid cohort
  }
  return client % num_cohorts;
}


void BitsToString(rappor::Bits b, std::string* output, int num_bytes) {
  // Copy IRR into a string, which can go in a protobuf.
  output->assign(num_bytes, '\0');
  for (int i = 0; i < num_bytes; ++i) {
    rappor::log("i: %d", i);
    // "little endian" string
    (*output)[i] = b & 0xFF;  // last byte
    b >>= 8;
  }
}

void PrintBitString(const std::string& s) {
  // print significant bit first
  for (int i = s.size() - 1; i >= 0; --i) {
    unsigned char byte = s[i];
    for (int j = 7; j >= 0; --j) {
      bool bit = byte & (1 << j);
      std::cout << (bit ? "1" : "0");
    }
  }
}

// TODO: Params as flags?  rappor_sim.py does this.  Or read CSV / JSON file.

int main(int argc, char** argv) {
  if (argc != 2) {
    rappor::log("Usage: rappor_encode <cohort>");
    exit(1);
  }
  // TODO: num_cohorts.  Then client name is hashed?

  int num_cohorts;
  bool success = StringToInt(argv[1], &num_cohorts);
  rappor::log("OK %d", success);
  if (!success) {
    rappor::log("Invalid number of cohorts: '%s'", argv[1]);
    exit(1);
  }
  rappor::log("cohorts %d", num_cohorts);

  // atoi is lame, can't distinguish 0 from error!
  /*
  if (cohort == 0) {
    rappor::log("Cohort must be an integer greater than 0.");
    exit(1);
  }
  */

  rappor::ReportList reports;

  int num_bits = 8;
  int num_bytes = num_bits / 8;

  int num_hashes = 2;
  rappor::Params params;
  params.set_num_bits(num_bits);
  params.set_num_hashes(num_hashes);

  rappor::LibcRandGlobalInit();  // seed
  rappor::LibcRand libc_rand(params.num_bits(), 0.25 /*p*/, 0.75 /*q*/);

  // NOTE: If const char*, it crashes, I guess because of temporary.
  // std::string constructor is not EXPLICIT -- gah.
  std::string client_secret("secret");

  int cohort_tmp = 3;  // TODO: replace

  const char* metric_name = "home-page";

  // Create an encoder for each cohort.
  std::vector<rappor::Encoder*> encoders(num_cohorts);

  for (int i = 0; i < num_cohorts; ++i) {
    encoders[i] = new rappor::Encoder(
        i /*cohort*/, num_bits, num_hashes,
        0.50 /* prob_f */, rappor::Md5, rappor::Hmac, client_secret, 
        libc_rand);

    assert(encoders[i]->IsValid());  // bad instantiation
  }

  // maybe have rappor_encode and rappor_demo
  // demo shows how to encode multiple metrics

  std::string line;

  // CSV header
  std::cout << "client,cohort,bloom,prr,rappor\n";

  while (true) {
    std::getline(std::cin, line);  // no trailing newline
    rappor::log("Got line %s", line.c_str());

    if (line.empty()) {
      break;
    }

    size_t i = line.find(',');
    if (i == std::string::npos) {
      rappor::log("Expected , in line %s", line.c_str());
      break;
    }

    std::string client_str = line.substr(0, i);  // everything before comma
    std::string value = line.substr(i + 1);  // everything after

    int cohort = GetCohort(client_str, num_cohorts);
    if (cohort == -1) {
      rappor::log("Error calculating cohort for %s", client_str.c_str());
      break;
    }

    rappor::log("CLIENT %s VALUE %s COHORT %d", client_str.c_str(),
        value.c_str(), cohort);

    rappor::Bits prr;
    rappor::Bits irr;
    bool ok = encoders[i]->Encode(line, &prr, &irr);

    // NOTE: Are there really encoding errors?
    if (!ok) {
      rappor::log("Error encoding string %s", line.c_str());
      break;
    }

    std::string irr_str;
    BitsToString(irr, &irr_str, num_bytes);

    std::string prr_str;
    BitsToString(prr, &prr_str, num_bytes);

    reports.add_report(irr_str);

    std::cout << client_str;
    std::cout << ',';
    std::cout << cohort;
    std::cout << ',';
    PrintBitString(prr_str);
    std::cout << ',';
    PrintBitString(irr_str);

    std::cout << "\n";
  }

  rappor::ReportListHeader* header = reports.mutable_header();
  // client params?
  header->set_metric_name(metric_name);
  header->set_cohort(cohort_tmp);
  header->mutable_params()->CopyFrom(params);

  rappor::log("report list %s", reports.DebugString().c_str());

  // Cleanup
  for (int i = 0; i < num_cohorts; ++i) {
    delete encoders[i];
  }
}
