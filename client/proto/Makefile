_tmp/rappor.pb.h: rappor.proto
	protoc --cpp_out _tmp rappor.proto

# TODO: Instead of listing header files here, use gcc -M and Make features.
_tmp/rappor_test: _tmp/rappor.pb.h rappor.cc rappor.h \
                  rappor_deps.h libc_rand.cc libc_rand.h \
									openssl_impl.cc rappor_test.cc 
# NOTES:
# -lprotobuf comes after source files
# -lrt for high resolution clock_gettime
# -lcrypto from openssl
	$(CXX) \
    -I _tmp \
    -o _tmp/rappor_test \
    _tmp/rappor.pb.cc rappor.cc libc_rand.cc openssl_impl.cc rappor_test.cc \
    -lprotobuf \
    -lcrypto \
    -lrt

_tmp/hmac_drbg_test: byte_vector_utils.cc byte_vector_utils.h
	$(CXX) \
    -o _tmp/hmac_drbg_test \
    byte_vector_utils.cc hmac_openssl.cc

# c++0x for std::array
		#-std=c++0x 
_tmp/hmac_openssl_test: hmac_openssl_test.cc openssl_impl.cc
	$(CXX) \
    -o _tmp/hmac_openssl_test \
    hmac_openssl_test.cc openssl_impl.cc \
	  -lcrypto
# OpenSSL provides libssl or libcrypto

clean:
	rm _tmp/* || true
