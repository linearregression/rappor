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
#include <stdarg.h>  // va_list, etc.

#include "rappor.pb.h"
#include "rappor.h"

void log(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
}

int main() {
  rappor::RapporReports r;
  r.set_cohort(10);
  log("hi %d", r.cohort());

  rappor::Params p;
  p.set_num_cohorts(128);
  log("params %s", p.DebugString().c_str());

  rappor::Encoder encoder(p);

  // what should this return?
  encoder.Encode("foo");
}
