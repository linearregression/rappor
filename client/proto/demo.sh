#!/bin/bash
#
# Usage:
#   ./demo.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

readonly RAPPOR_SRC=$(cd ../.. && pwd)

# Generate files in line mode

# We will have 64 cohorts
gen-sim-input() {
  #../../tests/gen_sim_input.py -h
  ../../tests/gen_sim_input.py -e -l 1000 -o $RAPPOR_SRC/_tmp/cpp.txt
}

# We want a 'client,cohort,rappor' exp_out.csv file

encode-cohort() {
  local cohort=$1

  # Disregard logs on stderr
  # Client is stubbed out

  cat _tmp/exp.txt \
    | _tmp/rappor_test $cohort 2>/dev/null \
    | awk -v cohort=$cohort -v client=0 '{print client "," cohort "," $1 }' \
    > _tmp/cohort_$cohort.csv
}

true-inputs() {
  cat _tmp/exp.txt | sort | uniq > $RAPPOR_SRC/_tmp/exp_true_inputs.txt
}

candidates() {
  cp _tmp/exp_true_inputs.txt $RAPPOR_SRC/_tmp/exp_candidates.txt
}

histogram() {
  python -c '
import collections
import csv
import sys

counter = collections.Counter()
with open(sys.argv[1]) as in_file:
  for line in in_file:
    counter[line.strip()] += 1

with open(sys.argv[2], "w") as out_file:
  c = csv.writer(out_file)
  c.writerow(("string", "count"))
  for value, count in counter.iteritems():
    c.writerow((value, str(count)))
' $RAPPOR_SRC/_tmp/cpp.txt $RAPPOR_SRC/_tmp/cpp_hist.csv
}

# We are currently using 64 cohorts
encode-all() {
  for cohort in $(seq 10); do
    echo "Cohort $cohort"
    encode-cohort $cohort
  done
  local out=$RAPPOR_SRC/_tmp/cpp_out.csv
  { echo 'client,cohort,rappor'; cat _tmp/cohort_*.csv; } > $out
  wc -l $out
}

root-demo() {
  ../../demo.sh "$@"
}

# TODO: Port this
#
# I think you have to cd
#
# And then generate a dist like "exp_cpp_"

run-cpp() {
  cd $RAPPOR_SRC

  banner "Hashing Candidates ($dist)"
  hash-candidates $dist

  banner "Summing bits ($dist)"
  sum-bits $dist

  # TODO:
  # guess-candidates  # cheat and get them from the true input
  # hash-candidates  # create map file

  banner "Analyzing RAPPOR output ($dist)"
  analyze $dist "Distribution Comparison ($dist)"
}


"$@"
