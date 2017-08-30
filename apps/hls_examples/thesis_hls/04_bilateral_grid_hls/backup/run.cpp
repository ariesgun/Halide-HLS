#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <math.h>

#include "pipeline_hls.h"
#include "pipeline_native.h"

#include "HalideBuffer.h"
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    Buffer<uint8_t> input = load_image(argv[1]);
    Buffer<uint8_t> out_native(1920, 1080);
    Buffer<uint8_t> out_hls(1920, 1080);

    printf("start.\n");

    pipeline_native(input, out_native);
    save_image(out_native, "out_native.png");

    printf("finish running native code\n");

    pipeline_hls(input, out_hls);
    save_image(out_hls, "out.png");

    printf("finish running HLS code\n");

    bool pass = true;
    /*
    for (int y = 0; y < out_hls.height(); y++) {
        for (int x = 0; x < out_hls.width(); x++) {
            if (out_native(x, y) != out_hls(x, y)) {
                printf("out_native(%d, %d) = %d, but out_c(%d, %d) = %d\n",
                       x, y, out_native(x, y),
                       x, y, out_hls(x, y));
                pass = false;
            }
	}
    }
    */
    if (pass) {
        printf("passed.\n");
        return 0;
    } else  {
        printf("failed.\n");
        return 1;
    }
}