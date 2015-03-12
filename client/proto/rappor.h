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
#include "rappor_deps.h"  // for dependency injection

namespace rappor {

class Encoder {
 public:
  // TODO:
  // - Take HmacFunc, Md5Func, IrrInterface
  // - Params -> ClientParams?
  //   - this has cohorts, while AnalysisParams has num_cohorts

  Encoder(
      const std::string& metric_name, int cohort, const Params& params,
      DeterministicRandInterface* det_rand,
      const RandInterface& rand);
  // Check this immediately after instantiating.  We are not using exceptions.
  bool IsValid() const;

  // something like StringPiece would be better here
  // or const char*?
  // output: shoudl be empty string
  bool Encode(const std::string& value, std::string* output) const;

 private:
  int cohort_;
  const Params& params_;
  DeterministicRandInterface* det_rand_;
  const RandInterface& rand_;
  int num_bytes_;
  bool is_valid_;
};

class Encoder2 {
 public:
  // TODO:
  // - HmacFunc and Md5Func should be partially computed already
  //   - pass in objects that you can call update() on
  //   - NaCl uses hashblocks.  Can you do that?
  //   - clone state?
  // - Params -> ClientParams?
  //   - this has cohorts, while AnalysisParams has num_cohorts

  Encoder2(
      // not used
      const std::string& metric_name, 

      // turn it into md5 state
      int cohort,

      // num_hashes, num_bits are the ones being used
      const Params& params,

      float prob_f, Md5Func* md5_func,  // bloom

      HmacFunc* hmac_func, const std::string& client_secret,  // PRR

      const IrrRandInterface& irr_rand);  // IRR

  // Check this immediately after instantiating.  We are not using exceptions.
  bool IsValid() const;

  bool Encode(const std::string& value, std::string* output) const;

 private:
  int cohort_;
  const Params& params_;
  float prob_f_;
  Md5Func* md5_func_;
  HmacFunc* hmac_func_;
  const std::string& client_secret_;
  const IrrRandInterface& irr_rand_;
  int num_bytes_;
  int hash_part_width_;  // log2(num_bits)
  bool is_valid_;
};

// For debug logging
void log(const char* fmt, ...);

}  // namespace rappor

#endif  // RAPPOR_H_
