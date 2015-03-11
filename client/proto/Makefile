_tmp/rappor.pb.h: rappor.proto
	protoc --cpp_out _tmp rappor.proto

# TODO: Instead of listing header files here, use gcc -M and Make features.
_tmp/rappor_test: _tmp/rappor.pb.h rappor.cc rappor.h \
                  rappor_rand_interface.h libc_rand.cc libc_rand.h \
									rappor_test.cc 
# NOTES:
# -lprotobuf comes after source files
# -lrt for high resolution clock_gettime
	$(CXX) \
    -I _tmp \
    -o _tmp/rappor_test \
    _tmp/rappor.pb.cc rappor.cc libc_rand.cc rappor_test.cc \
    -lprotobuf \
    -lrt

_tmp/hmac_drbg_test: byte_vector_utils.cc byte_vector_utils.h
	$(CXX) \
    -o _tmp/hmac_drbg_test \
    byte_vector_utils.cc

clean:
	rm _tmp/* || true
