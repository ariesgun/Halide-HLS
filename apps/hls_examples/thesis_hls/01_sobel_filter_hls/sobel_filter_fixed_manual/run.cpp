/*
* Change History:
* 25/04/2017: Update using Buffer instead of Image
* 
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "HalideBuffer.h"
#include "pipeline_native.h"
#include "pipeline_hls.h"
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace Halide::Runtime;

const signed char kernelx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};
const signed char kernely[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};


int main(int argc, char **argv) {
    Buffer<uint8_t> in     = load_image(argv[1]);
    Buffer<uint8_t> in_ref = load_image(argv[4]);
    Buffer<int8_t> kernel_x(3, 3);
    Buffer<int8_t> kernel_y(3, 3);

    Buffer<uint8_t> out_native(in.width(), in.height());
    Buffer<uint8_t> out_hls   (in.width(), in.height());

    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            kernel_x(x, y) = kernelx[y][x];

    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            kernel_y(x, y) = kernely[y][x];

    // Check Buffer
    printf("Input buffer\n");
    printf("min: %d %d %d \n", in.min(0), in.min(1), in.min(2));
    printf("extent: %d %d %d\n", in.extent(0), in.extent(1), in.extent(2));
    printf("stride: %d %d %d\n", in.stride(0), in.stride(1), in.stride(2));

    printf("Output buffer\n");
    printf("min: %d %d %d \n", out_hls.min(0), out_hls.min(1), out_hls.min(2));
    printf("extent: %d %d %d\n", out_hls.extent(0), out_hls.extent(1), out_hls.extent(2));
    printf("stride: %d %d %d\n", out_hls.stride(0), out_hls.stride(1), out_hls.stride(2));

    printf("start.\n");

    pipeline_native(in, kernel_x, kernel_y, out_native);
    save_image(out_native, argv[2]);

    printf("finish running native code\n");

    pipeline_hls(in, kernel_x, kernel_y, out_hls);
    
    save_image(out_hls, argv[3]);
    printf("finish running HLS code\n");

    bool success = true;
    
    for (int y = 0; y < out_native.height() / 5; y++) {
        for (int x = 0; x < out_native.width() / 5; x++) {
	        if (out_hls(x, y) != in_ref(x, y)) {
                    printf("out_native(%d, %d) = %d, in_ref(%d, %d) = %d, but out_c(%d, %d) = %d\n",
			   x, y, out_native(x, y), x, y, in_ref(x,y),
			   x, y, out_hls(x, y));
                    success = false;
                
            }
        }
    }
    

    if (success) {
        printf("Successed!\n");
        return 0;
    } else {
        printf("Failed!\n");
        return 1;
    }

}
