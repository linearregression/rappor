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

#include <stdio.h>

#include "rappor.pb.h"
#include "rappor.h"
#include "libc_rand.h"

// TODO: Should this take params as flags?
//
// Then read strings from stdin
// Assign them cohorts?
// And then show outputs
//
// Outputs should be the report set type?
// - single protobuf?


int main() {
  rappor::ReportList reports;
  reports.add_report("dummy");

  rappor::Params p;
  p.set_num_cohorts(128);
  p.set_num_bits(8);
  p.set_num_hashes(2);
  rappor::log("params %s", p.DebugString().c_str());

  rappor::LibcRandGlobalInit();  // seed
  rappor::LibcRand libc_rand(p.num_bits(), 0.50, 0.50, 0.75);

  rappor::log("p_bits: %x", libc_rand.p_bits());

  rappor::log("hi %s", reports.report(0).c_str());

  rappor::Encoder encoder("home-page", 1, p, libc_rand);

  // what should this return?
  // single report.
  // Then aggregate them all into a report, with params.
  std::string out;
  encoder.Encode("foo", &out);

  rappor::log("Output: %s", out.c_str());
}
