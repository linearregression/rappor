#include <openssl/evp.h>  // EVP_sha256
#include <openssl/hmac.h>  // HMAC
#include <openssl/md5.h>  // MD5
#include <openssl/sha.h>  // SHA256_DIGEST_LENGTH
#include <stdio.h>
#include <string>

// NOTE: If using C++11 (-std=c++0x), it's safer to do this:
// typedef std::array<unsigned char, 32> Sha256Digest;

typedef unsigned char Sha256Digest[SHA256_DIGEST_LENGTH];

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

typedef unsigned char Md5Digest[MD5_DIGEST_LENGTH];  // 16

bool Md5(const std::string& value, Md5Digest output) {
  // std::string has 'char', OpenSSL wants unsigned char.
  MD5(reinterpret_cast<const unsigned char*>(value.c_str()),
      value.size(), output);
  return true;
}

int main() {
  std::string key("key");
  std::string value("value");
  Sha256Digest sha256;

  bool result = Hmac(key, value, sha256);
  printf("result: %d\n", result);
  printf("digest:\n");

  const int n = sizeof(sha256);
  for (int i = 0; i < n; ++i) {
    printf("%02x", sha256[i]);
  }
  printf("\n");

  Md5Digest md5;

  bool ok = Md5(value, md5);
  printf("ok: %d\n", ok);

  for (int i = 0; i < sizeof(md5); ++i) {
    printf("%02x", md5[i]);
  }
  printf("\n");
}
