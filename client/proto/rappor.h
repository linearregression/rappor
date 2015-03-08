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

#ifndef RAPPOR_H_
#define RAPPOR_H_

#include <string>

#include "rappor.pb.h"
#include "rappor_rand_interface.h"

namespace rappor {

class Encoder {
 public:
   // TODO:
   // - Take random interface?  Or hard-code params
   // - Is hash function hard-coded?

  Encoder(
      const std::string& metric_name, int cohort,
      const Params& params, const RandInterface& rand);

  // something like StringPiece would be better here
  // or const char*?
  bool Encode(const std::string& value);

 private:
  const Params& params_;
  const RandInterface& rand_;
};

// For debug logging
void log(const char* fmt, ...);

}  // namespace rappor

#endif  // RAPPOR_H_
