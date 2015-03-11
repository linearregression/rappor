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

#include <stdio.h>

#include "rappor.pb.h"
#include "rappor.h"
#include "libc_rand.h"

// TODO: Should this take params as flags?
//
// Then read strings from stdin
// Assign them cohorts?
// And then show outputs
//
// Outputs should be the report set type?
// - single protobuf?


int main() {
  rappor::ReportList reports;

  rappor::Params p;
  p.set_num_bits(8);
  p.set_num_hashes(2);

  rappor::LibcRandGlobalInit();  // seed
  rappor::LibcDeterministicRand libc_d_rand(p.num_bits(), 0.50);
  rappor::LibcRand libc_rand(p.num_bits(), 0.50, 0.75);

  int cohort = 9;
  const char* metric_name = "home-page";
  rappor::Encoder encoder(metric_name, cohort, p, libc_rand, &libc_d_rand);

  // TODO: loop over stdin

  for (int i = 0; i < 5; ++i) {
    std::string out;
    encoder.Encode("foo", &out);
    rappor::log("Output: %x", out.c_str());
    reports.add_report(out);
  }

  rappor::ReportListHeader* header = reports.mutable_header();
  // client params?
  header->set_metric_name(metric_name);
  header->set_cohort(cohort);
  header->mutable_params()->CopyFrom(p);

  rappor::log("report list %s", reports.DebugString().c_str());
}
