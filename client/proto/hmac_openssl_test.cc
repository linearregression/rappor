#include <stdio.h>

#include "openssl_impl.h"

int main() {
  std::string key("key");
  std::string value("value");
  rappor::Sha256Digest sha256;

  bool result = rappor::Hmac(key, value, sha256);
  printf("result: %d\n", result);
  printf("digest:\n");

  const int n = sizeof(sha256);
  for (int i = 0; i < n; ++i) {
    printf("%02x", sha256[i]);
  }
  printf("\n");

  rappor::Md5Digest md5;

  bool ok = rappor::Md5(value, md5);
  printf("ok: %d\n", ok);

  for (int i = 0; i < sizeof(md5); ++i) {
    printf("%02x", md5[i]);
  }
  printf("\n");
}
