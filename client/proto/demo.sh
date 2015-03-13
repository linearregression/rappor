#!/bin/bash
#
# Usage:
#   ./demo.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

# Generate files in line mode

# We will have 64 cohorts
gen-sim-input() {
  #../../tests/gen_sim_input.py -h
  ../../tests/gen_sim_input.py -e -l 1000 -o _tmp/exp.txt
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

# We are currently using 64 cohorts
encode-all() {
  for cohort in $(seq 10); do
    echo "Cohort $cohort"
    encode-cohort $cohort
  done
  { echo 'client,cohort,rappor'; cat _tmp/cohort_*.csv; } > _tmp/exp_out.csv
  wc -l _tmp/exp_out.csv
}

"$@"
