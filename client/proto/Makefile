_tmp/rappor.pb.h: rappor.proto
	protoc --cpp_out _tmp rappor.proto

_tmp/rappor_test: rappor_test.cc _tmp/rappor.pb.h
# NOTE: -lprotobuf comes after source files
	g++ \
    -I _tmp \
    -o _tmp/rappor_test \
    _tmp/rappor.pb.cc rappor_test.cc \
    -lprotobuf 

clean:
	rm _tmp/* || true
