#ifndef HALIDE_CODEGEN_HLS_TARGET_HLS_TARGET_H
#define HALIDE_CODEGEN_HLS_TARGET_HLS_TARGET_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <hls_stream.h>
#include "Stencil_with_fixed.h"
#include "Stencil_fixed.h"

void hls_target(
hls::stream<AxiPackedStencil<ap_fixed<20,1> , 1, 1> > &arg_0,
//hls::stream<AxiPackedStencilFixed<20, 1, 1, 1> > &arg_0,
hls::stream<AxiPackedStencil<ap_fixed<29,1> , 1, 1> > &arg_1);

#endif

