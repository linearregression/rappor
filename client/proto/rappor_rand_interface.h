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

// NOTE: All 3 need num_bits?

// Instead of the above
class PermanentHashInterface {
 public:
  virtual void Hash(const std::string& value) = 0;
 protected:
  PermanentHashInterface(const std::string& secret, float f)
      : secret_(secret), f_(f) {
  }
  const std::string& secret_;
  float f_;
};

// Example implementations:
// - hmac from OpenSSL
// - hmac from NaCl
// - libc rand: seed with srand(hash(value)), then call rand()

// TODO: Rename to InstantaneousHashInterface

class RandInterface {
 public:
  virtual unsigned int p_bits() const = 0;
  virtual unsigned int q_bits() const = 0;
 protected:
  RandInterface(float p, float q)
      : p_(p), q_(q) {
  }
  float p_;
  float q_;
};

// Example implementations:
// - OS specific randomness
//   - /dev/urandom on Unix
//   - Windows-specific random interface
// - libc rand
// - HMAC DRBG (using the same secret)

class BloomInterface {
 public:
  // Return an interface with 'num_hashes' bits
  virtual void Hash(const std::string& value) = 0;
 protected:
  BloomInterface(int num_hashes)
      : num_hashes_(num_hashes) {
  }
  int num_hashes_;
};

// One strategy, have 3 interfaces:
// - Bloom filter
// - PRR
// - IRR

// Other strategy:
// - just ask for:
//   - md5
//   - hmac-sha1
//   - secret
//
// Then you can do all 3 of them
//   Bloom filter gets md5
//   PRR gets hmac-sha1 and secret
//   IRR could get a real interface -- /dev/urandom or other
//
// There's no point in providing LibcDeterministicRand.  You need OpenSSL or
// NaCl for hash functions anyway!  It's overly general.

}  // namespace rappor

#endif  // RAPPOR_RAND_INTERFACE_H_
