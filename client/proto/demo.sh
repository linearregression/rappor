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

"$@"
