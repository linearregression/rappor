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

#ifndef LIBC_RAND_H_
#define LIBC_RAND_H_

#include "rappor_rand_interface.h"

namespace rappor {

// call this once per application.
void LibcRandGlobalInit();

// Interface that the encoder requires.  Applications should implement this
// based on their own.
class LibcRand : public RandInterface {
 public:
  // TODO: rename these
  LibcRand(int num_bits, float p, float q)
      : RandInterface(p, q),
        num_bits_(num_bits) {
  }
  virtual unsigned int p_bits() const;
  virtual unsigned int q_bits() const;
 private:
  int num_bits_;
};

class LibcDeterministicRand : public DeterministicRandInterface {
 public:
  LibcDeterministicRand(int num_bits, float f)
      : num_bits_(num_bits), f_(f) {
  }
  virtual unsigned int f_bits() const;
  virtual unsigned int uniform() const;
  virtual void seed(const std::string& seed);  // mutates internal state
 private:
  int num_bits_;
  float f_;
};

}  // namespace rappor

#endif  // LIBC_RAND_H_
