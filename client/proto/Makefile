_tmp/rappor.pb.h: rappor.proto
	protoc --cpp_out _tmp rappor.proto

_tmp/rappor_test: _tmp/rappor.pb.h rappor.cc rappor.h \
                  rappor_rand_interface.h libc_rand.cc libc_rand.h \
									rappor_test.cc 
# NOTES:
# -lprotobuf comes after source files
# -lrt for high resolution clock_gettime
	g++ \
    -I _tmp \
    -o _tmp/rappor_test \
    _tmp/rappor.pb.cc rappor.cc libc_rand.cc rappor_test.cc \
    -lprotobuf \
    -lrt

clean:
	rm _tmp/* || true
