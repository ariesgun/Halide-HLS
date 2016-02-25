#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <math.h>

#include "pipeline_hls.h"
#include "pipeline_native.h"

#include "halide_image.h"
#include "halide_image_io.h"

using namespace Halide::Tools;

int main(int argc, char **argv) {

    float k = 0.04;
    int threshold = 10;

    Image<uint8_t> input = load_image(argv[1]);
    Image<uint8_t> out_native(input.width(), input.height(), input.channels());
    Image<uint8_t> out_hls(input.width(), input.height(), input.channels());

    printf("start.\n");

    pipeline_native(input, k, threshold, out_native);
    save_image(out_native, "out.png");

    printf("finished running native code\n");

    pipeline_hls(input, k, threshold,  out_hls);
    save_image(out_hls, "out_hls.png");

    printf("finished running HLS code\n");

    bool success = true;
    for (int y = 0; y < out_native.height(); y++) {
        for (int x = 0; x < out_native.width(); x++) {
            if (fabs(out_native(x, y, 0) - out_hls(x, y, 0)) > 1e-4) {
                printf("out_native(%d, %d, 0) = %d, but out_c(%d, %d, 0) = %d\n",
                       x, y, out_native(x, y, 0),
                       x, y, out_hls(x, y, 0));
		success = false;
            }
	}
    }
    if (success)
        return 0;
    else return 1;
}
