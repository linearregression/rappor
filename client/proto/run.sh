#!/bin/bash
#
# Usage:
#   ./run.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

setup() {
  # need libprotobuf-dev for headers to compile against.
  sudo apt-get install protobuf-compiler libprotobuf-dev
}

build() {
  mkdir -p _tmp
  protoc --cpp_out _tmp rappor.proto
}

compile-test() {
  # for unordered_{map,set}
  # -std=c++0x \
  g++ \
    -I_tmp \
    -lprotobuf \
    -o rappor_test \
    _tmp/rappor.pb.cc rappor_test.cc
}

rappor-test() {
  compile-test
  ./rappor_test
}

"$@"
