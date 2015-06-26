#!/bin/bash
#
# Usage:
#   ./demo.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

readonly RAPPOR_SRC=$(cd ../.. && pwd)

# Params from rappor_test
gen-params() {
  cat >$RAPPOR_SRC/_tmp/cpp_params.csv <<EOF
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

  cat _tmp/exp_cpp_reports.csv \
    | client/proto/_tmp/rappor_test $cohort 2>/dev/null #\
    #| awk -v cohort=$cohort -v client=0 '{print client "," cohort "," $1 }' \
    #> _tmp/cohort_$cohort.csv
}

true-inputs() {
  local dist=cpp
  cat $RAPPOR_SRC/_tmp/${dist}.txt | sort | uniq \
    > $RAPPOR_SRC/_tmp/${dist}_true_inputs.txt
}

candidates() {
  local dist=cpp
  cp \
    $RAPPOR_SRC/_tmp/${dist}_true_inputs.txt \
    $RAPPOR_SRC/_tmp/${dist}_candidates.txt
}

readonly NUM_COHORTS=64

histogram() {
  python -c '
import collections
import csv
import sys

num_cohorts = int(sys.argv[1])  # multiply counts by this number

counter = collections.Counter()
with open(sys.argv[2]) as in_file:
  for line in in_file:
    counter[line.strip()] += 1

with open(sys.argv[3], "w") as out_file:
  c = csv.writer(out_file)
  c.writerow(("string", "count"))
  for value, count in counter.iteritems():
    c.writerow((value, str(count * num_cohorts)))
' 64 $RAPPOR_SRC/_tmp/cpp.txt $RAPPOR_SRC/_tmp/cpp_hist.csv
}

# We are currently using 64 cohorts
encode-all() {
  local max=$(expr $NUM_COHORTS - 1)
  for cohort in $(seq 0 $max); do
    echo "Cohort $cohort"
    encode-cohort $cohort
  done
  local out=$RAPPOR_SRC/_tmp/cpp_out.csv
  { echo 'client,cohort,rappor'; cat _tmp/cohort_*.csv; } > $out
  wc -l $out
}

run-cpp() {
  cd $RAPPOR_SRC
  local dist=cpp  # fake one

  echo "Hashing Candidates ($dist)"
  analysis/tools/hash_candidates.py \
    $RAPPOR_SRC/_tmp/cpp_params.csv \
    < $RAPPOR_SRC/_tmp/cpp_params.csv \

  echo "Summing bits ($dist)"
  ./demo.sh sum-bits $dist

  # TODO:
  # guess-candidates  # cheat and get them from the true input
  # hash-candidates  # create map file

  echo "Analyzing RAPPOR output ($dist)"
  ./demo.sh analyze $dist "Distribution Comparison ($dist)"

}

# FAILING
analyze() {
  cd $RAPPOR_SRC
  local dist=cpp  # fake one

  echo "Analyzing RAPPOR output ($dist)"
  time ./demo.sh analyze $dist "Distribution Comparison ($dist)" || true
}

"$@"
