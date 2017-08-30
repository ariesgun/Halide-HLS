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
    Buffer<float> in = load_image(argv[1]);
    Buffer<float> out_native(1920, 1080, in.channels());
    Buffer<float> out_hls   (1920, 1080, in.channels());

    printf("start.\n");

    // for (int j = 0; j < 1280; j++) {
    //     for (int i = 0; i < 720; i++) {
    //         temp(i, j) = in(i+800, j+800);
    //     }
    // }
    // save_image(temp, "in2.png");

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
	        if (out_native(x, y, c) - out_hls(x, y, c) > 0.001f) {
                    printf("out_native(%d, %d, %d) = %f, but out_c(%d, %d, %d) = %f\n",
			   x, y, c, out_native(x, y, c),
			   x, y, c, out_hls(x, y, c));
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
