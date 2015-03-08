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
class LibcRand : public RapporRandInterface {
 public:
  // TODO: rename these
  LibcRand(int num_bits, float f, float p, float q)
      : num_bits_(num_bits), f_(f), p_(p), q_(q) {
  }
  virtual unsigned int f_bits();
  virtual unsigned int p_bits();
  virtual unsigned int q_bits();
 private:
  int num_bits_;
  float f_;
  float p_;
  float q_;
};

}  // namespace rappor

#endif  // LIBC_RAND_H_
