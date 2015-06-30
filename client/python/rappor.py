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

"""RAPPOR client library.

Privacy is ensured without a third party by only sending RAPPOR'd data over the
network (as opposed to raw client data).

Note that we use SHA1 for the Bloom filter hash function.
"""
import csv
import hashlib
import hmac
import json
import random
import struct
import sys


class Error(Exception):
  pass


def log(msg, *args):
  if args:
    msg = msg % args
  print >>sys.stderr, msg


class Params(object):
  """RAPPOR encoding parameters.

  These affect privacy/anonymity.  See the paper for details.
  """
  def __init__(self):
    self.num_bloombits = 16      # Number of bloom filter bits (k)
    self.num_hashes = 2          # Number of bloom filter hashes (h)
    self.num_cohorts = 64        # Number of cohorts (m)
    self.prob_p = 0.50           # Probability p
    self.prob_q = 0.75           # Probability q
    self.prob_f = 0.50           # Probability f

    self.flag_oneprr = False     # One PRR for each user/word pair

  # For testing
  def __eq__(self, other):
    return self.__dict__ == other.__dict__

  def __repr__(self):
    return repr(self.__dict__)

  def to_json(self):
    """Convert this instance to JSON.

    The names are be compatible with the apps/api server.
    """
    return json.dumps({
        'numBits': self.num_bloombits,
        'numHashes': self.num_hashes,
        'numCohorts': self.num_cohorts,
        'probPrr': self.prob_f,
        'probIrr0': self.prob_p,
        'probIrr1': self.prob_q,
    })

  # NOTE:
  # - from_csv is currently used in sum_bits.py
  # - to_csv is in rappor_sim.print_params
  @staticmethod
  def from_csv(f):
    """Read the RAPPOR parameters from a CSV file.

    Args:
      f: file handle

    Returns:
      Params instance.

    Raises:
      rappor.Error: when the file is malformed.
    """
    c = csv.reader(f)
    ok = False
    p = Params()
    for i, row in enumerate(c):

      if i == 0:
        if row != ['k', 'h', 'm', 'p', 'q', 'f']:
          raise Error('Header %s is malformed; expected k,h,m,p,q,f' % row)

      elif i == 1:
        try:
          # NOTE: May raise exceptions
          p.num_bloombits = int(row[0])
          p.num_hashes = int(row[1])
          p.num_cohorts = int(row[2])
          p.prob_p = float(row[3])
          p.prob_p = float(row[4])
          p.prob_q = float(row[5])
        except (ValueError, IndexError) as e:
          raise Error('Row is malformed: %s' % e)
        ok = True

      else:
        raise Error('Params file should only have two rows')

    if not ok:
      raise Error("Expected second row with params")

    return p


class SimpleRandom(object):
  """Returns N 32-bit words where each bit has probability p of being 1."""

  def __init__(self, prob_one, num_bits, rand=None):
    self.prob_one = prob_one
    self.num_bits = num_bits
    self.rand = rand or random.Random()

  def __call__(self):
    p = self.prob_one
    rand_fn = self.rand.random  # cache it for speed

    r = 0
    for i in xrange(self.num_bits):
      bit = rand_fn() < p
      r |= (bit << i)  # using bool as int
    return r


class _RandFuncs(object):
  """Base class for randomness."""

  def __init__(self, params, rand=None):
    """
    Args:
      params: RAPPOR parameters
      rand: optional object satisfying the random.Random() interface.
    """
    self.rand = rand or random.Random()
    self.num_bits = params.num_bloombits


class SimpleRandFuncs(_RandFuncs):

  def __init__(self, params, rand=None):
    _RandFuncs.__init__(self, params, rand=rand)

    # PRR probability
    self.f_gen = SimpleRandom(params.prob_f, self.num_bits, rand=rand)

    # IRR probabilities
    self.p_gen = SimpleRandom(params.prob_p, self.num_bits, rand=rand)
    self.q_gen = SimpleRandom(params.prob_q, self.num_bits, rand=rand)
    self.uniform_gen = SimpleRandom(0.5, self.num_bits, rand=rand)


# TODO:
#
# - IRR should be based on a client_secret
#   - use hmac
#
# fastrand: Don't need it anymore?
#
# TODO: we approximate the 'f' PRR probability with multiple bits of
# hmac-sha256.
#
# PRR uses hmac-sha256
# IRR uses fastrand/etc.
# p_gen


def get_rappor_masks(secret, word, params, rand_funcs):
  """Call 3 random functions.  Seed deterministically beforehand if oneprr.

  TODO:
  - inline this function
  - Rewrite this to be clearer.  We can use a completely different Random()
    instance in the case of oneprr.
  - Expose it in the simulation.  It doesn't appear to be exercised now.

  Returns:
    assigned_cohort: integer cohort
    uniform: bits set with probability 1/2
    f_mask: bits set with probability f
  """
  if params.flag_oneprr:
    stored_state = rand_funcs.rand.getstate()  # Store state
    rand_funcs.rand.seed(secret + word)  # Consistently seeded

  uniform = rand_funcs.uniform_gen()
  # fastrand generate numbers up to 64 bits, and returns None if more are
  # requested.
  if uniform is None:
    raise AssertionError('Too many bits (k = %d)' % params.num_bloombits)
  f_mask = rand_funcs.f_gen()

  if params.flag_oneprr:                    # Restore state
    rand_funcs.rand.setstate(stored_state)

  return uniform, f_mask


def cohort_to_bytes(cohort):
  # https://docs.python.org/2/library/struct.html
  # - Big Endian (>) for consistent network byte order.
  # - L means 4 bytes when using >
  return struct.pack('>L', cohort)


# TODO: move md5 out of here, so it's only calculated once.  Should return an
# array of length "num_hashes".

def which_bit(word, cohort, hash_num, num_bloombits):
  """Returns the bit to set in the Bloom filter.

  NOTE: This is called by hash_candidates.py to make the map file.
  """
  value = cohort_to_bytes(cohort) + word  # Cohort is 4 byte prefix.
  md5 = hashlib.md5(value)

  digest = md5.digest()

  # Each has is a byte, which means we could have up to 256 bit Bloom filters.
  # There are 16 bytes in an MD5, in which case we can have up to 16 hash
  # functions per Bloom filter.
  if hash_num > len(digest):
    raise RuntimeError("Can't have more than %d hashes" % md5)

  log('Cohort %d', cohort)
  log('MD5 %s', md5.hexdigest())

  return ord(digest[hash_num]) % num_bloombits


def make_bloom_bits(word, cohort, num_hashes, num_bloombits):
  """Compute Bloom Filter."""
  bloom_bits = 0
  for hash_num in xrange(num_hashes):
    bit_to_set = which_bit(word, cohort, hash_num, num_bloombits)
    bloom_bits |= (1 << bit_to_set)
  return bloom_bits


def bit_string(irr, num_bloombits):
  """Like bin(), but uses leading zeroes, and no '0b'."""
  s = ''
  bits = []
  for bit_num in xrange(num_bloombits):
    if irr & (1 << bit_num):
      bits.append('1')
    else:
      bits.append('0')
  return ''.join(reversed(bits))


class Encoder(object):
  """Obfuscates values for a given user using the RAPPOR privacy algorithm."""

  def __init__(self, params, cohort, secret, rand_funcs=None):
    """
    Args:
      params: RAPPOR Params() controlling privacy
      cohort: cohort, for PRR.  In the simulator, each user
        gets its own Encoder instance.
      secret: For PRR to be a deterministic function of the reported value.
      rand_funcs: randomness, can be deterministic for testing.
    """
    self.params = params  # RAPPOR params
    self.cohort = cohort
    self.secret = secret

    self.rand_funcs = rand_funcs or SimpleRandFuncs(params)
    self.p_gen = self.rand_funcs.p_gen
    self.q_gen = self.rand_funcs.q_gen

  def _internal_encode(self, word):
    """Helper function for simulation / testing.

    Returns:
      The PRR and the IRR.  The PRR should never be sent over the network.
    """
    params = self.params
    num_bits = params.num_bloombits

    bloom_bits = make_bloom_bits(word, self.cohort, params.num_hashes,
                                 num_bits)

    uniform, f_mask = get_rappor_masks(self.secret, word, params,
                                       self.rand_funcs)

    # Both bit manipulations below use the following fact:
    # To set c = a if m = 0 or b if m = 1
    # c = (a & not m) | (b & m)

    # Call bit i of the Bloom Filter B_i.  Then bit i of the PRR is defined as:
    #
    # 1   with prob f/2
    # 0   with prob f/2
    # B_i with prob 1-f

    # uniform bits are 1 with probability 1/2, and f_mask bits are 1 with
    # probability f.  So in the expression below:
    #
    # - Bits in (uniform & f_mask) are 1 with probability f/2.
    # - (bloom_bits & ~f_mask) clears a bloom filter bit with probability
    # f, so we get B_i with probability 1-f.
    # - The remaining bits are 0, with remaining probability f/2.

    prr = (uniform & f_mask) | (bloom_bits & ~f_mask)
    #log('U %s / F %s', bit_string(uniform, num_bits),
    #    bit_string(f_mask, num_bits))

    #log('B %s / PRR %s', bit_string(bloom_bits, num_bits),
    #    bit_string(prr, num_bits))

    # Compute instantaneous randomized response:
    # If PRR bit is set, output 1 with probability q
    # If PRR bit is not set, output 1 with probability p
    p_bits = self.p_gen()
    q_bits = self.q_gen()

    irr = (p_bits & ~prr) | (q_bits & prr)

    return bloom_bits, prr, irr  # IRR is the rappor

  def encode(self, word):
    """Encode a string with RAPPOR.

    Args:
      word: the string that should be privately transmitted.

    Returns:
      A number that is the IRR (Instantaneous Randomized Response).
    """
    _, _, irr = self._internal_encode(word)
    return irr
