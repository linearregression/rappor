#!/usr/bin/python
"""
find_cliques.py
"""

import sys
import pprint


class Error(Exception):
  pass

a0 = [
  ('ab', 'cd'),
  ('ab', 'le'),
  ('ij', 'kl'),
  ('qr', 'st'),
  ('yy', 'cd'),
  ]


a1 = [
  ('st', 'uv'),
  ('kl', 'mn'),
  ('cd', 'ef'),
  ('cd', 'xx'),
  ]

a2 = [
  ('uv', 'wx'),
  ('mn', 'op'),
  ('mn', 'jj'),

  ('ef', 'gh'),
  ('ef', 'zz'),
  ]

a = [a0, a1, a2]

def Adjacent(last, edges):
  for left, right in edges:
    if left == last:
      yield right

def EnumerateStep(paths, edges):
  new_paths = []
  for p in paths:
    print p
    last = p[-1]  # last node
    print last
    # NOTE: last is not unique.  Cache the list 'Adjacent?'
    # Or just make an Adjacency list.
    for right in Adjacent(last, edges):
      new_paths.append(p + [right])
  return new_paths


def EnumerateCandidates(a):
  paths = [list(e) for e in a[0]]
  for i in xrange(1, len(a)):
    edges = a[i]
    paths = EnumerateStep(paths, edges)
  return paths


def main(argv):
  """Returns an exit code."""

  paths = EnumerateCandidates(a)
  pprint.pprint(paths)

  # TODO: now find ones without complete subgraph
  return


if __name__ == '__main__':
  try:
    main(sys.argv)
  except Error, e:
    print >> sys.stderr, e.args[0]
    sys.exit(1)
