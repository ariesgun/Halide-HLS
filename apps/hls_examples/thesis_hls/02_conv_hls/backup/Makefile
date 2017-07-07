#### Halide flags
HALIDE_BIN_PATH := ../../../..
HALIDE_SRC_PATH := ../../../..
include ../../../support/Makefile.inc

#### HLS flags
include ../Makefile.inc
HLS_LOG = vivado_hls.log

.PHONY: all run_hls
all: $(BIN)/out.png
run_hls: $(HLS_LOG)


$(BIN)/conv_exec: conv_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/pipeline_hls.cpp $(BIN)/hls_target.cpp: $(BIN)/conv_exec
	@-mkdir -p $(BIN)
	HL_DEBUG_CODEGEN=3 $^ -n pipeline_hls -f pipeline_hls -o $(BIN) -e cpp_hls,h target=$(HL_TARGET)-hls-c_plus_plus_name_mangling

$(BIN)/pipeline_native.o: $(BIN)/conv_exec
	@-mkdir -p $(BIN)
	HL_DEBUG_CODEGEN=0 $^ -n pipeline_native -f pipeline_native -o $(BIN) -e o,h target=$(HL_TARGET)

$(BIN)/pipeline_cuda.o: $(BIN)/conv_exec
	@-mkdir -p $(BIN)
	HL_DEBUG_CODEGEN=0 $^ -o $(BIN) target=$(HL_TARGET)

$(BIN)/run: $(BIN)/pipeline_hls.cpp $(BIN)/hls_target.cpp $(BIN)/pipeline_native.o run.cpp
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I -O1 -DNDEBUG $(HLS_CXXFLAGS) -g -Wall -Werror $^ -lpthread -ldl -o $@ $(IMAGE_IO_FLAGS)

$(BIN)/run_cuda: pipeline_native.o pipeline_cuda.o run_cuda.cpp
	$(CXX) -O3 $(CXXFLAGS) -Wall -Werror $^ -lpthread -ldl -o $@  $(IMAGE_IO_FLAGS) `pkg-config --libs opencv`

$(BIN)/out.png: $(BIN)/run
	$(BIN)/run ./in.png $(BIN)/out.png

$(HLS_LOG): ../hls_support/run_hls.tcl pipeline_hls.cpp run.cpp
	RUN_PATH=$(realpath ./) \
	RUN_ARGS= \
	vivado_hls -f $< -l $(HLS_LOG)

$(BIN)/out_cuda.png: run_cuda
	CUDA_LAUNCH_BLOCKING=1 HL_NUM_THREADS=4 ./run_cuda ../../../images/benchmark_1080p_rgb.png

clean:
	rm -rf $(BIN)
