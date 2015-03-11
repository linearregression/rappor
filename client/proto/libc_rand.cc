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

#include "libc_rand.h"

#include <assert.h>
#include <stdint.h>  // uint64_t
//#include <stdio.h>  // printf
#include <stdlib.h>  // srand
#include <time.h>  // time

// Copied from client/python/fastrand.c
uint64_t randbits(float p1, int num_bits) {
  uint64_t result = 0;
  int threshold = (int)(p1 * RAND_MAX);
  int i;
  for (i = 0; i < num_bits; ++i) {
    uint64_t bit = (rand() < threshold);
    result |= (bit << i);
  }
  return result;
}

static bool gInitialized = false;


static void SeedWithTime() {
  // This only has second resolution, and isn't good enough.
  //int seed = time(NULL);

  timespec ts;
  // clock_gettime(CLOCK_MONOTONIC, &ts); // Works on FreeBSD
  clock_gettime(CLOCK_REALTIME, &ts); // Works on Linux

  srand(ts.tv_nsec);  // seed with nanoseconds
}

namespace rappor {

void LibcRandGlobalInit() {
  SeedWithTime();
  gInitialized = true;
}

//
// LibcDeterministicRand
//

unsigned int LibcDeterministicRand::f_bits() const {
  assert(gInitialized);
  return randbits(f_, num_bits_);
}

unsigned int LibcDeterministicRand::uniform() const {
  assert(gInitialized);
  return randbits(0.5, num_bits_);
}

// NOTE: The libc RNG is global, so we are mutating the LibcRand state too!
// This probably isn't good, but this is insecure anyway.
void LibcDeterministicRand::seed(const std::string& seed) {
  assert(gInitialized);

  // hash the seed string into an integer, then pass to libc.
  int h = 5381;
  for (int i = 0; i < seed.size(); ++i) {
    h = (h << 5) + h + seed[i];
  }
  srand(h);
}

//
// LibcRand
//

unsigned int LibcRand::p_bits() const {
  assert(gInitialized);
  SeedWithTime();  // non-deterministic IRR
  return randbits(p_, num_bits_);
}

unsigned int LibcRand::q_bits() const {
  assert(gInitialized);
  SeedWithTime();  // non-deterministic IRR
  return randbits(q_, num_bits_);
}

}  // namespace rappor
