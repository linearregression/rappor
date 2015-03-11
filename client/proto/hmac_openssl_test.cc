#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdio.h>
#include <string>

typedef unsigned char Sha256Digest[32];

bool Hmac(const std::string& key, const std::string& value,
          Sha256Digest output) {
  return HMAC(EVP_sha256(),
              key.c_str(), key.size(),
              // std::string has 'char', OpenSSL wants unsigned char.
              reinterpret_cast<const unsigned char*>(value.c_str()),
              value.size(),
              output,
              NULL);
}

int main() {
  std::string key("key");
  std::string value("value");
  Sha256Digest digest;

  bool result = Hmac(key, value, digest);
  printf("result: %d\n", result);
  printf("digest:\n");

  const int n = sizeof(digest);
  for (int i = 0; i < n; ++i) {
    printf("%x", digest[i]);
  }
  printf("\n");
}
