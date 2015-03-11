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

#ifndef RAPPOR_DEPS_H_
#define RAPPOR_DEPS_H_

#include <string>

namespace rappor {

typedef unsigned char Md5Digest[16];
typedef unsigned char Sha256Digest[32];

// rappor:Encoder needs an MD5 function for the bloom filter, and an HMAC
// function for the PRR.
//
// And a random function for the IRR.
//   NOTE: libc rand returns a float between 0 and 1.
// Maybe you just need to return p_bits and q_bits.

// NOTE: is md5 always sufficient?  Maybe you should have a generic hash.
// string -> string?  But you need to know how many bits there are.
// num_hashes * log2(num_bits) == 2 * log2(8) = 6, or 2 * log2(128) = 14.

typedef bool Md5Func(const std::string& value, Md5Digest output);
typedef bool HmacFunc(const std::string& key, const std::string& value,
                      Sha256Digest output);

class IrrRandInterface {
 public:
  virtual unsigned int p_bits() const = 0;
  virtual unsigned int q_bits() const = 0;
 protected:
  IrrRandInterface(float p, float q)
      : p_(p), q_(q) {
  }
  float p_;
  float q_;
};



}  // namespace rappor

#endif  // RAPPOR_DEPS_H_

