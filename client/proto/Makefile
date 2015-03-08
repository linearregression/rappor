_tmp/rappor.pb.cc: rappor.proto
	protoc --cpp_out _tmp rappor.proto

_tmp/rappor_test: rappor_test.cc _tmp/rappor.pb.cc
	# NOTE: -lprotobuf comes after source files
	g++ \
    -I_tmp \
    -o _tmp/rappor_test \
    _tmp/rappor.pb.cc rappor_test.cc \
    -lprotobuf 
