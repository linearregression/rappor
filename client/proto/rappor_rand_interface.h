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

// Interfaces that the encoder requires.  Applications should implement emhis
// according to their own requirements.
//
// libc_rand.h provides insecure implementations.

// TODO: Real implementations
// 
// - DeterministicRandInterface is HMAC DRBG
// - RandInterface could also be this, or it could be /dev/urandom
namespace rappor {

// Alternative API:
//
// f_bits, uniform = det_rand.GetMasks(value)
//
// underneath, this is implemented with
// 1. hmac_drbg.seed(value)
// 2. extract enough bits to get f prob, then uniform

class DeterministicRandInterface {
 public:
  // NOTE: unsigned int for now.  Caller has to make sure it bits match
  // the internal bloom filter size?
  // TODO: How about define the ByteVector type here?

  virtual unsigned int f_bits() const = 0;
  virtual unsigned int uniform() const = 0;
  virtual void seed(const std::string& seed) = 0;  // mutates internal state
};

class RandInterface {
 public:
  virtual unsigned int p_bits() const = 0;
  virtual unsigned int q_bits() const = 0;
};

}  // namespace rappor

#endif  // RAPPOR_RAND_INTERFACE_H_
