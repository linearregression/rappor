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

hmac-openssl-test() {
  make _tmp/hmac_openssl_test
  _tmp/hmac_openssl_test
}

get() {
  wget --no-clobber --directory _tmp "$@"
}

download() {
  get https://chromium.googlesource.com/chromium/src/+archive/master/components/rappor.tar.gz
  get https://chromium.googlesource.com/chromium/src/+archive/master/crypto.tar.gz
}

extract() {
  mkdir -p _tmp/chrome _tmp/crypto

  pushd _tmp/chrome
  tar xvf ../rappor.tar.gz
  popd

  pushd _tmp/crypto
  tar xvf ../crypto.tar.gz
  popd
}

copy() {
  #cp -v _tmp/chrome/byte_vector_utils* .
  #cp -v _tmp/crypto/{hmac.h,hmac_openssl.cc} .
  cp -v _tmp/crypto/hmac.cc .
}

count() {
  wc -l hmac* byte_vector_utils*
}

"$@"
