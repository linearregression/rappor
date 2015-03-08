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
    const std::string& metric_name, int cohort,
    const Params& params, const RandInterface& rand)
  : params_(params),
    rand_(rand),
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
  ByteVector bytes;
  //std::string bytes;
  //bytes.reserve(num_bytes_);

  // First do hashing

  for (int i = 0; i < params_.num_hashes(); ++i) {
    // TODO: need more than one hash function
    int h = 5381;
    for (int j = 0; j < value.size(); ++j) {
      h = (h << 5) + h + value[j];
    }
    unsigned int bit_to_set = h % params_.num_bits();
    log("Hash %d: %d, set bit %d", i, h, bit_to_set);

    bytes |= 1 << bit_to_set;
  }

  // Now do PRR.

  log("f_bits: %x", rand_.f_bits());


  // Now do IRR
  log("p_bits: %x", rand_.p_bits());
  log("q_bits: %x", rand_.q_bits());

  // Copy it into a string, which can go in a protobuf.
  output->reserve(num_bytes_);
  for (int i = 0; i < num_bytes_; ++i) {
    //output[num_bytes_ - 1 - i] = bytes & 0xFF;  // last byte

    // "little endian" string
    output[i] = bytes & 0xFF;  // last byte
    bytes >>= 8;
  }
}

}  // namespace rappor
