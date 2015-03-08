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
// based on their own.
class RapporRandInterface {
 public:
  virtual unsigned int f_bits() = 0;
  virtual unsigned int p_bits() = 0;
  virtual unsigned int q_bits() = 0;
};

}  // namespace rappor

#endif  // RAPPOR_RAND_INTERFACE_H_
