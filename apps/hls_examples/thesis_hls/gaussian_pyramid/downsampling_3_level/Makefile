#### Halide flags
HALIDE_BIN_PATH := ../../../..
HALIDE_SRC_PATH := ../../../..
include ../../../support/Makefile.inc

IMAGES = ../../../images

#### HLS flags
include ../Makefile.inc

BIN ?= bin

all: $(BIN)/process

$(BIN)/gaussian_pyramid_exec: gaussian_pyramid_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/pipeline_hls.cpp $(BIN)/hls_target.cpp: $(BIN)/gaussian_pyramid_exec
	@-mkdir -p $(BIN)
	HL_DEBUG_CODEGEN=3 $^ -n pipeline_hls -f pipeline_hls -o $(BIN) -e cpp_hls,h,html target=$(HL_TARGET)-hls-c_plus_plus_name_mangling

$(BIN)/gaussian_pyramid.a: $(BIN)/gaussian_pyramid_exec
	@-mkdir -p $(BIN)
	HL_DEBUG_CODEGEN=1 $^ -o $(BIN)  target=$(HL_TARGET)

$(BIN)/process: $(BIN)/pipeline_hls.cpp $(BIN)/hls_target.cpp process.cpp $(BIN)/gaussian_pyramid.a
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) $(HLS_CXXFLAGS) -Wall -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS)

$(BIN)/out.png: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process rgb.png 4 1 1 10 $(BIN)/out.png

# Build rules for generating a visualization of the pipeline using HalideTraceViz
$(BIN)/viz/gaussian_pyramid.a: $(BIN)/gaussian_pyramid_exec
	@-mkdir -p $(BIN)/viz
	$^ -o $(BIN)/viz target=$(HL_TARGET)-trace_loads-trace_stores-trace_realizations pyramid_levels=6

$(BIN)/process_viz: process.cpp $(BIN)/viz/gaussian_pyramid.a
	@-mkdir -p $(BIN)/viz
	$(CXX) $(CXXFLAGS) -I$(BIN)/viz -Wall -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS)

../../bin/HalideTraceViz:
	$(MAKE) -C ../../ bin/HalideTraceViz

$(BIN)/gaussian_pyramid.mp4: $(BIN)/process_viz ../../bin/HalideTraceViz
	@-mkdir -p $(BIN)
	bash viz.sh

clean:
	rm -rf $(BIN)
