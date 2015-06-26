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

init() {
  mkdir --verbose -p _tmp
}

rappor-test() {
  make _tmp/rappor_test
  _tmp/rappor_test "$@"
}

rappor-encode() {
  seq 10 | rappor-test 5 
}

empty-input() {
  echo -n '' | rappor-test 5 
}

# This outputs an HMAC and MD5 value.  Compare with Python/shell below.
hmac-openssl-test() {
  make _tmp/hmac_openssl_test
  _tmp/hmac_openssl_test
}

test-hmac-sha256() {
  #echo -n foo | sha256sum
  python -c '
import hashlib
import hmac
import sys

secret = sys.argv[1]
body = sys.argv[2]
m = hmac.new(secret, body, digestmod=hashlib.sha256)
print m.hexdigest()
' "key" "value"
}

test-md5() {
  echo -n value | md5sum
}

"$@"
