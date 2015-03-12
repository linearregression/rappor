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
#include <stdlib.h>  // itoa
#include <stdarg.h>  // va_list, etc.

#include "rappor.h"

namespace rappor {

void log(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
}

Encoder::Encoder(
    const std::string& metric_name, int cohort, const Params& params,
    DeterministicRandInterface* det_rand,
    const RandInterface& rand)
  : cohort_(cohort),
    params_(params),
    rand_(rand),
    det_rand_(det_rand),
    num_bytes_(0),
    is_valid_(true) {
  // number of bytes in bloom filter
  if (params_.num_bits() % 8 == 0) {
    num_bytes_ = params_.num_bits() / 8;
    log("num bytes: %d", num_bytes_);
  } else {
    is_valid_ = false;
  }
}

bool Encoder::IsValid() const {
  return is_valid_;
}

//typedef std::vector<uint8_t> ByteVector;
// We don't need more than 8 bytes for now
typedef uint64_t ByteVector; 

bool Encoder::Encode(const std::string& value, std::string* output) const {
  ByteVector bloom;

  // First do hashing.

  for (int i = 0; i < params_.num_hashes(); ++i) {
    // TODO: need more than one hash function
    int h = 5381;
    for (int j = 0; j < value.size(); ++j) {
      h = (h << 5) + h + value[j];
    }
    unsigned int bit_to_set = h % params_.num_bits();
    log("Hash %d: %d, set bit %d", i, h, bit_to_set);

    bloom |= 1 << bit_to_set;
  }

  // Do PRR.

  // Seed it every time, for deterministic PRR.  This is equivalent to
  // memoization, as described in the paper, and is memory-efficient.
  det_rand_->seed(value);
  ByteVector f_bits = det_rand_->f_bits();
  log("f_bits: %x", f_bits);

  ByteVector uniform = det_rand_->uniform();
  log("uniform: %x", uniform);
  
  // first term: 1 with (1/2 + f/2) probability
  // second term: 0 with 1/2 probability, B with 1/2 probability
  ByteVector prr = (f_bits & uniform) | (bloom & ~uniform);
  log("prr: %x", uniform);

  // Do IRR.

  ByteVector p_bits = rand_.p_bits();
  ByteVector q_bits = rand_.q_bits();

  log("p_bits: %x", p_bits);
  log("q_bits: %x", q_bits);

  ByteVector irr = (p_bits & ~prr) | (q_bits & prr);

  log("irr: %x", irr);

  // Copy IRR into a string, which can go in a protobuf.

  output->reserve(num_bytes_);
  for (int i = 0; i < num_bytes_; ++i) {
    //output[num_bytes_ - 1 - i] = bytes & 0xFF;  // last byte

    // "little endian" string
    output[i] = irr & 0xFF;  // last byte
    irr >>= 8;
  }
}

// The number of bits for one hash function is log2(number of bloom filter
// bits).
//
// This function also validates that num_bits is a whole number of bytes.

int HashPartWidth(int bloom_width) {
  switch (bloom_width) {
    case 8:   return 3;
    case 16:  return 4;
    case 32:  return 5;
    case 64:  return 6;
    case 128: return 7;
    default:  return -1;  // ERROR
  }
}

Encoder2::Encoder2(
    const std::string& metric_name, int cohort, const Params& params,
    float prob_f,
    Md5Func* md5_func,
    HmacFunc* hmac_func, const std::string& client_secret,
    const IrrRandInterface& irr_rand)
    : cohort_(cohort),
      params_(params),
      prob_f_(prob_f),
      md5_func_(md5_func),
      hmac_func_(hmac_func),
      client_secret_(client_secret),
      irr_rand_(irr_rand),
      num_bytes_(0),
      is_valid_(true) {

  hash_part_width_ = HashPartWidth(params.num_bits());

  // Validity constraints:
  //
  // bits fit in an integral type uint64_t:
  //   num_bits < 64 (or sizeof(ByteVector) * 8)
  // md5 is long enough:
  //   128 > ( num_hashes * log2(num_bits) )
  // sha256 is long enough:
  //   256 > num_bits + (prob_f resolution * num_bits)

  // number of bytes in bloom filter
  if (params_.num_bits() % 8 == 0) {
    num_bytes_ = params_.num_bits() / 8;
    log("num bytes: %d", num_bytes_);
  } else {
    is_valid_ = false;
  }
}

bool Encoder2::IsValid() const {
  return is_valid_;
}

void PrintMd5(Md5Digest md5) {
  // GAH!  sizeof(md5) does NOT work.  Because that's a pointer.
  for (int i = 0; i < sizeof(Md5Digest); ++i) {
    //printf("[%d]\n", i);
    printf("%02x", md5[i]);
  }
  printf("\n");
}

void PrintSha256(Sha256Digest h) {
  // GAH!  sizeof(md5) does NOT work.  Because that's a pointer.
  for (int i = 0; i < sizeof(Sha256Digest); ++i) {
    //printf("[%d]\n", i);
    printf("%02x", h[i]);
  }
  printf("\n");
}

bool Encoder2::Encode(const std::string& value, std::string* output) const {
  ByteVector bloom = 0;

  // First do hashing.

  Md5Digest md5;
  // TODO: Instead of hashing the cohort every time, have a stateful thing in
  // the constructor, and clone the state.  It's the MD5 context.
  //
  // Chrome has
  // hash_seed_offset_ in BloomFilter class, set to (cohort *
  // num_hashes)
  // Can you just concatenate that number as a string?
  //
  // Another thing we could do is use hmac(secret) ?

  char cohort[10];
  sprintf(cohort, "%d", cohort_);
  std::string cohort_str(cohort);
  md5_func_(value + cohort_str, md5);
  PrintMd5(md5);

  int num_bits = params_.num_bits();

  // We don't need the full precision
  // Another option: use each byte.  3-7 bits each is fine.

  for (int i = 0; i < params_.num_hashes(); ++i) {
    // 1 byte per hash for now
    int bit_to_set = md5[i] % num_bits;
    bloom |= 1 << bit_to_set;
    log("Hash %d, set bit %d", i, bit_to_set);
  }

  // Do PRR.

  // Create HMAC(secret, value), and use its bits to construct f and uniform
  // bits.

  Sha256Digest sha256;
  hmac_func_(client_secret_, value, sha256);

  printf("sha256:\n");
  PrintSha256(sha256);

  // sould this be a byte-chunk iterator?
  // 1 to 7 byte strides out of 32 bytes?
  // Or you could just have: uniform, and then 3 probabilities

  // Here we are relying on the fact that there are 4 uint64_t's in the
  // Sha256Digest.
  uint64_t* pieces = reinterpret_cast<uint64_t*>(sha256);

  uint64_t uniform = pieces[0];  // 50% changes
  uint64_t f_bits = pieces[1];

  if (prob_f_ == .5f) {
    ;  // done
  } else if (prob_f_ == 0.25f) {
    f_bits &= pieces[2];
  } else if (prob_f_ == 0.75f) {
    f_bits |= pieces[2];
  } else {
    printf("f should be 0.25, 0.5, or 0.75 -- got %f", prob_f_);
    assert(false);
  }

  // truncate to the right width
  // TODO: should be function of num_bits
  uniform &= 0xFF;  // one byte
  f_bits &= 0xFF;  // one byte

  // NOTE: Could change format string
  log("f_bits: %08x", f_bits);
  log("uniform: %08x", uniform);
  
  // first term: 1 with (1/2 + f/2) probability
  // second term: 0 with 1/2 probability, B with 1/2 probability
  // NOTE: bloom is already 8 bits, while the others are 64 bits.
  uint64_t prr = (f_bits & uniform) | (bloom & ~uniform);
  log("prr: %08x", prr);

  // Do IRR.

  ByteVector p_bits = irr_rand_.p_bits();
  ByteVector q_bits = irr_rand_.q_bits();

  // Make them too
  p_bits &= 0xFF;  // one byte
  q_bits &= 0xFF;  // one byte

  log("p_bits: %x", p_bits);
  log("q_bits: %x", q_bits);

  ByteVector irr = (p_bits & ~prr) | (q_bits & prr);

  log("irr: %x", irr);

  // Copy IRR into a string, which can go in a protobuf.

  output->reserve(num_bytes_);
  for (int i = 0; i < num_bytes_; ++i) {
    //output[num_bytes_ - 1 - i] = bytes & 0xFF;  // last byte

    // "little endian" string
    output[i] = irr & 0xFF;  // last byte
    irr >>= 8;
  }
}

}  // namespace rappor
