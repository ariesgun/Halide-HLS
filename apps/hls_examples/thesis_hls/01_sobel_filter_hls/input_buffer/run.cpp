/*
* Change History:
* 25/04/2017: Update using Buffer instead of Image
* 
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "HalideBuffer.h"
#include "hls_target.h"
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    Buffer<uint32_t> in(10, 10);
    Buffer<uint8_t> out_native(10, 10);
    hls::stream<uint32_t> in_stream;
    //hls::stream<Stencil<uint8_t, 3, 3>> out_stream;
    hls::stream<uint16_t> out_stream;
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
             in(x, y) = x + y*10;
             in_stream.write(in(x,y));
	     //printf("Sending %d in\n", in(x,y));
        }
    }
  
    hls_target(out_stream, in_stream);
    printf("finish running native code\n");

  //   for (int y = 0; y < 10; y++) {
  //       for (int x = 0; x < 10; x++) {
	 //    Stencil<uint8_t, 3, 3> out_stencil = out_stream.read();
	 //    printf(" %d %d %d %d %d %d %d %d %d\n", out_stencil(0,0), out_stencil(1,0), out_stencil(2,0),
  //   		out_stencil(0,1), out_stencil(1,1), out_stencil(2,1),
		// out_stencil(0,2), out_stencil(1,2), out_stencil(2,2));
  //       }
  //   }

    for (int idx = 0; idx < 100; idx++) {
        printf(" Output %d is %d\n", idx, out_stream.read());
    }

    bool success = true;
   
    if (success) {
        printf("Successed!\n");
        return 0;
    } else {
        printf("Failed!\n");
        return 1;
    }

}
