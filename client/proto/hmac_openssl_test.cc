#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdio.h>
#include <string>


bool Hmac(const std::string& key, const std::string& value,
          unsigned char* output) {
  return HMAC(EVP_sha256(),
              key.c_str(), key.size(),
              // Why static_cast here?  Chrome hmac_opeenl.cc needs that too.
              // I gues we're going from signed to unsigned?
              reinterpret_cast<const unsigned char*>(value.c_str()),
              value.size(),

              output,
              NULL);
}

int main() {
  printf("hi\n");
  EVP_sha256();

  std::string key("key");
  std::string value("value");
  //std::string digest(32);  // 32 bytes
  unsigned char digest[32 + 1];

  /*
  ScopedOpenSSLSafeSizeBuffer<EVP_MAX_MD_SIZE> result(digest, digest_length);
  return !!::HMAC(hash_alg_ == SHA1 ? EVP_sha1() : EVP_sha256(),
                  vector_as_array(&plat_->key), plat_->key.size(),
                  reinterpret_cast<const unsigned char*>(data.data()),
                  data.size(), result.safe_buffer(), NULL);
  */

  bool result = Hmac(key, value, digest);
  printf("result: %d\n", result);
  printf("digest: \n");
  for (int i = 0; i < 32; ++i) {
    printf("%x ", digest[i]);
  }
}
