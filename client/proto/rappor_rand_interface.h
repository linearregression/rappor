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

#ifndef RAPPOR_RAND_INTERFACE_H_
#define RAPPOR_RAND_INTERFACE_H_

#include <string>

namespace rappor {

// Interface that the encoder requires.  Applications should implement this
// according to their own requirements.
class RandInterface {
 public:
  // NOTE: unsigned int for now.  Caller has to make sure it bits match
  // the internal bloom filter size?
  // 
  // TODO: How about define the ByteVector type here?

  virtual unsigned int f_bits() const = 0;
  virtual unsigned int p_bits() const = 0;
  virtual unsigned int q_bits() const = 0;
  virtual unsigned int uniform() const = 0;
};

// Rename above to RandomBitInterface?

class DeterministicRandInterface {
 public:
  // NOTE: unsigned int for now.  Caller has to make sure it bits match
  // the internal bloom filter size?
  // 
  // TODO: How about define the ByteVector type here?

  void Init(const std::string& secret);  // secret per client

  // Seed with the DRBG state to the value, so you always get the same value,
  // but it's unpredictable without knowing the secret.
  unsigned int RandomBits(const std::string value);  // state
};

}  // namespace rappor

#endif  // RAPPOR_RAND_INTERFACE_H_
