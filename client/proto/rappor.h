// Copyright 2015 Google Inc. All rights reserved.
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
#include "rappor_deps.h"  // for dependency injection

namespace rappor {

class Encoder {
 public:
  // TODO:
  // - HmacFunc and Md5Func should be partially computed already
  //   - pass in objects that you can call update() on
  //   - NaCl uses hashblocks.  Can you do that?
  //   - clone state?
  // - Params -> ClientParams?
  //   - this has cohorts, while AnalysisParams has num_cohorts

  Encoder(
      // turn it into md5 state
      int cohort,

      // num_hashes, num_bits are the ones being used
      int num_bits, int num_hashes,

      float prob_f, Md5Func* md5_func,  // bloom

      HmacFunc* hmac_func, const std::string& client_secret,  // PRR

      const IrrRandInterface& irr_rand);  // IRR

  // Check this immediately after instantiating.  We are not using exceptions.
  bool IsValid() const;

  bool Encode(const std::string& value, std::string* output) const;

 private:
  const int cohort_;
  const int num_bits_;
  const int num_hashes_;
  const float prob_f_;
  Md5Func* md5_func_;
  HmacFunc* hmac_func_;
  const std::string& client_secret_;
  const IrrRandInterface& irr_rand_;
  int num_bytes_;
  bool is_valid_;
  uint64_t debug_mask_;
};

// For debug logging
void log(const char* fmt, ...);

}  // namespace rappor

#endif  // RAPPOR_H_
