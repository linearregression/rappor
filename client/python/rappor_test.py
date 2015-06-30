#!/usr/bin/python
#
# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
rappor_test.py: Tests for rappor.py

NOTE! This contains tests that might fail with very small
probability (< 1 in 10,000 times). This is implicitly required
for testing probability. Such tests start with the stirng "testProbFailure."
"""

import cStringIO
import copy
import math
import random
import unittest

import rappor  # module under test


class RapporParamsTest(unittest.TestCase):

  def setUp(self):
    self.typical_instance = rappor.Params()
    ti = self.typical_instance  # For convenience
    ti.num_cohorts = 64        # Number of cohorts
    ti.num_hashes = 2          # Number of bloom filter hashes
    ti.num_bloombits = 16      # Number of bloom filter bits
    ti.prob_p = 0.40           # Probability p
    ti.prob_q = 0.70           # Probability q
    ti.prob_f = 0.30           # Probability f

    # TODO: Move this to constructor, or add a different constructor
    ti.flag_oneprr = False     # One PRR for each user/word pair

  def tearDown(self):
    pass

  def testFromCsv(self):
    f = cStringIO.StringIO('k,h,m,p,q,f\n32,2,64,0.5,0.75,0.6\n')
    params = rappor.Params.from_csv(f)
    self.assertEqual(32, params.num_bloombits)
    self.assertEqual(64, params.num_cohorts)

    # Malformed header
    f = cStringIO.StringIO('k,h,m,p,q\n32,2,64,0.5,0.75,0.6\n')
    self.assertRaises(rappor.Error, rappor.Params.from_csv, f)

    # Missing second row
    f = cStringIO.StringIO('k,h,m,p,q,f\n')
    self.assertRaises(rappor.Error, rappor.Params.from_csv, f)

    # Too many rows
    f = cStringIO.StringIO('k,h,m,p,q,f\n32,2,64,0.5,0.75,0.6\nextra')
    self.assertRaises(rappor.Error, rappor.Params.from_csv, f)

  def testGetRapporMasksWithoutOnePRR(self):
    params = copy.copy(self.typical_instance)
    params.prob_f = 0.5  # For simplicity

    num_words = params.num_bloombits // 32 + 1
    rand = MockRandom()
    rand_funcs = rappor.SimpleRandFuncs(params, rand)

    f_bits, mask_indices = rappor.get_rappor_masks(
        'secret', 'abc', params, rand_funcs)

    self.assertEquals(0x000db6d, f_bits)  # dependent on 3 MockRandom values
    self.assertEquals(0x0006db6, mask_indices)

  def testGetBloomBits(self):
    for cohort in xrange(0, 64):
      b = rappor.get_bloom_bits('foo', cohort, 2, 16)
      print 'cohort', cohort, 'bloom', b

  def testCohortToBytes(self):
    b = rappor.cohort_to_bytes(1)
    print repr(b)
    self.assertEqual(4, len(b))

  def testGetRapporMasksWithOnePRR(self):
    # Set randomness function to be used to sample 32 random bits
    # Set randomness function that takes two integers and returns a
    # random integer cohort in [a, b]

    params = copy.copy(self.typical_instance)
    params.flag_oneprr = True

    num_words = params.num_bloombits // 32 + 1
    rand = MockRandom()
    rand_funcs = rappor.SimpleRandFuncs(params, rand)

    # First two calls to get_rappor_masks for identical inputs
    # Third call for a different input
    print '\tget_rappor_masks 1'
    f_bits_1, mask_indices_1 = rappor.get_rappor_masks(
        "0", "abc", params, rand_funcs)
    print '\tget_rappor_masks 2'
    f_bits_2, mask_indices_2 = rappor.get_rappor_masks(
        "0", "abc", params, rand_funcs)
    print '\tget_rappor_masks 3'
    f_bits_3, mask_indices_3 = rappor.get_rappor_masks(
        "0", "abcd", params, rand_funcs)

    # First two outputs should be identical, i.e., identical PRRs
    self.assertEquals(f_bits_1, f_bits_2)
    self.assertEquals(mask_indices_1, mask_indices_2)

    # Third PRR should be different from the first PRR
    self.assertNotEqual(f_bits_1, f_bits_3)
    self.assertNotEqual(mask_indices_1, mask_indices_3)

    # Now testing with flag_oneprr false
    params.flag_oneprr = False
    f_bits_1, mask_indices_1 = rappor.get_rappor_masks(
        "0", "abc", params, rand_funcs)
    f_bits_2, mask_indices_2 = rappor.get_rappor_masks(
        "0", "abc", params, rand_funcs)

    self.assertNotEqual(f_bits_1, f_bits_2)
    self.assertNotEqual(mask_indices_1, mask_indices_2)

  def testEncoder(self):
    """Expected bloom bits is computed as follows.

    f_bits = 0xfff0000f and mask_indices = 0x0ffff000 from
    testGetRapporMasksWithoutPRR()

    q_bits = 0xfffff0ff from mock_rand.randomness[] and how get_rand_bits works
    p_bits = 0x000ffff0 from -- do --

    bloom_bits_array is 0x0000 0048 (3rd bit and 6th bit, from
    testSetBloomArray, are set)

    Bit arithmetic ends up computing
    bloom_bits_prr = 0x0ff00048
    bloom_bits_irr= = 0x0ffffff8
    """
    params = copy.copy(self.typical_instance)
    params.prob_f = 0.5
    params.prob_p = 0.5
    params.prob_q = 0.75

    rand_funcs = rappor.SimpleRandFuncs(params, MockRandom())
    e = rappor.Encoder(params, 0, 'secret', rand_funcs=rand_funcs)

    irr = e.encode("abc")

    self.assertEquals(0x000ffff, irr)


class MockRandom(object):
  """Returns one of three random values in a cyclic manner.

  Mock random function that involves *some* state, as needed for tests that
  call randomness several times. This makes it difficult to deal exclusively
  with stubs for testing purposes.
  """

  def __init__(self):
    self.counter = 0
    # SimpleRandom will call self.random() below for each bit, which will
    # return these 3 values in sequence.
    self.randomness = [0.0, 0.6, 0.0]
    self.n = len(self.randomness)

  def seed(self, seed):
    self.counter = hash(seed) % self.n
    #print 'SEED', self.counter

  def getstate(self):
    #print 'GET STATE', self.counter
    return self.counter

  def setstate(self, state):
    #print 'SET STATE', state
    self.counter = state

  def randint(self, a, b):
    return a + self.counter

  def random(self):
    rand_val = self.randomness[self.counter]
    self.counter = (self.counter + 1) % self.n
    return rand_val


if __name__ == "__main__":
  unittest.main()
