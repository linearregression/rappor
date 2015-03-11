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

rappor-test() {
  make _tmp/rappor_test
  _tmp/rappor_test
}

hmac-drbg-test() {
  make _tmp/hmac_drbg_test
  _tmp/hmac_drbg_test
}

download() {
  wget https://chromium.googlesource.com/chromium/src/+archive/master/components/rappor.tar.gz
}

extract() {
  mkdir _tmp/chrome
  cd _tmp/chrome
  tar xvf ../../rappor.tar.gz
}

copy() {
  cp _tmp/chrome/byte_vector_utils* .
}

"$@"
