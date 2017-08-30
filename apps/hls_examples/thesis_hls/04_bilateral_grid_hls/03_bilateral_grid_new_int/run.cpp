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


int main(int argc, char **argv) {

    if (argc < 5) {
        printf("Usage: ./filter input.png output.png range_sigma timing_iterations\n"
               "e.g. ./filter input.png output.png 0.1 10\n");
        return 0;
    }

    // float r_sigma = atof(argv[4]);
    //int timing_iterations = atoi(argv[5]);

    Buffer<uint8_t> in = load_image(argv[1]);
    Buffer<uint8_t> out_native(in.width(), in.height(), in.channels());
    Buffer<uint8_t> out_hls   (in.width(), in.height(), in.channels());

    // Timing code. Timing doesn't include copying the input data to
    // the gpu or copying the output back.
    //double min_t = benchmark(timing_iterations, 10, [&]() {
    //    bilateral_grid(input, r_sigma, output);
    //});
    //printf("Time: %gms\n", min_t * 1e3);

    printf("start.\n");

    pipeline_native(in, out_native);
    save_image(out_native, argv[2]);

    printf("finish running native code\n");

    pipeline_hls(in, out_hls);
    save_image(out_hls, argv[3]);

    printf("finish running HLS code\n");

    bool success = true;
    for (int y = 0; y < out_native.height(); y++) {
        for (int x = 0; x < out_native.width(); x++) {
	    for (int c = 0; c < out_native.channels(); c++) {
	        if (out_native(x, y, c) - out_hls(x, y, c) > 0.000000001f) {
                    printf("out_native(%d, %d, %d) = %d, but out_c(%d, %d, %d) = %d with diff : %d\n",
			   x, y, c, out_native(x, y, c),
			   x, y, c, out_hls(x, y, c), out_native(x, y, c) - out_hls(x, y, c));
                    success = false;
                }
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
