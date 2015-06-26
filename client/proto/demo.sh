#!/bin/bash
#
# Usage:
#   ./demo.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

readonly RAPPOR_SRC=$(cd ../.. && pwd)

readonly DIST=exp_cpp

# Params from rappor_test
gen-params() {
  cat >$RAPPOR_SRC/_tmp/${DIST}_params.csv <<EOF
k,h,m,p,q,f
16,2,64,0.5,0.75,0.5
EOF
}

# Generate files in line mode

# We will have 64 cohorts
gen-reports() {
  cd $RAPPOR_SRC

  #../../tests/gen_sim_input.py -h
  local num_unique_values=100
  local num_clients=10000
  local values_per_client=10
  tests/gen_reports.R exp $num_unique_values $num_clients $values_per_client \
    _tmp/exp_cpp_reports.csv
}

print-true-inputs() {
  cd $RAPPOR_SRC
  ./regtest.sh print-true-inputs 100 > _tmp/exp_cpp_true_inputs.txt
}

# Print candidates from true inpputs
make-candidates() {
  local dist=exp_cpp
  cp \
    $RAPPOR_SRC/_tmp/${dist}_true_inputs.txt \
    $RAPPOR_SRC/_tmp/${dist}_candidates.txt
}

make-map() {
  local dist=$DIST

  cd $RAPPOR_SRC
  export PYTHONPATH=$RAPPOR_SRC/client/python

  analysis/tools/hash_candidates.py \
    _tmp/${dist}_params.csv \
    < _tmp/${dist}_candidates.txt \
    > _tmp/${dist}_map.csv
}

rappor-sim() {
  local num_cohorts=64  # matches params

  make _tmp/rappor_test
  cd $RAPPOR_SRC

  time cat _tmp/exp_cpp_reports.csv \
    | client/proto/_tmp/rappor_test $num_cohorts 2>/dev/null \
    > _tmp/exp_cpp_out.csv
}

sum-bits() {
  cd $RAPPOR_SRC
  export PYTHONPATH=$RAPPOR_SRC/client/python
  analysis/tools/sum_bits.py \
    _tmp/${DIST}_params.csv \
    < _tmp/${DIST}_out.csv \
    > _tmp/${DIST}_counts.csv
}

# This part is like rappor_sim.py, but in C++.
# We take a "client,string" CSV (no header) and want a 'client,cohort,rappor'
# exp_cpp_out.csv file
#
# TODO: rappor_test.cc can generate a new client every time I guess.

encode-cohort() {
  make _tmp/rappor_test
  cd $RAPPOR_SRC

  local cohort=$1

  # Disregard logs on stderr
  # Client is stubbed out

  time cat _tmp/exp_cpp_reports.csv \
    | client/proto/_tmp/rappor_test $cohort 2>/dev/null #\
    > _tmp/cohort_$cohort.csv
    #| awk -v cohort=$cohort -v client=0 '{print client "," cohort "," $1 }' \
}

encode-demo() {
  make _tmp/rappor_test
  cd $RAPPOR_SRC
  local out=_tmp/encode_demo.txt
  local num_cohorts=4  # matches params
  time head -n 100 _tmp/exp_cpp_reports.csv \
    | client/proto/_tmp/rappor_test $num_cohorts > $out
  echo
  echo OUTPUT
  cat $out
}

test-rappor-test() {
  set -x
  make _tmp/rappor_test 
  #_tmp/rappor_test bad
  _tmp/rappor_test 3
}

readonly NUM_COHORTS=64

histogram() {
  python -c '
import collections
import csv
import sys

counter = collections.Counter()
with open(sys.argv[1]) as in_file:
  for line in in_file:
    line = line.strip()
    try:
      value = line.split(",")[1]  # second row
    except IndexError:
      print value
      raise
    counter[value] += 1

with open(sys.argv[2], "w") as out_file:
  c = csv.writer(out_file)
  c.writerow(("string", "count"))
  for value, count in counter.iteritems():
    c.writerow((value, str(count)))
' $RAPPOR_SRC/_tmp/${DIST}_reports.csv $RAPPOR_SRC/_tmp/${DIST}_hist.csv
}

compare-dist() {
  cd $RAPPOR_SRC
  local dist=exp_cpp  # fake one

  local case_dir=_tmp
  local instance_dir=_tmp/1
  local out_dir=${instance_dir}_report

  # Temporary hack until analyze.R has better syntax.
  mkdir -p $instance_dir
  cp --verbose _tmp/${DIST}_counts.csv $instance_dir
  cp --verbose _tmp/${DIST}_hist.csv $instance_dir

  mkdir --verbose -p $out_dir

  echo "Analyzing RAPPOR output ($dist)"
  tests/analyze.R -t "exp cpp" \
    $case_dir/$dist \
    $instance_dir/$dist \
    $out_dir
}

"$@"
