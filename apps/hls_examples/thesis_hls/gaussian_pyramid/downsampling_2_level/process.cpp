#include <cstdio>
#include <chrono>

#include "gaussian_pyramid.h"
#include "pipeline_hls.h"

#include "halide_benchmark.h"
#include "HalideBuffer.h"
#include "halide_image_io.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc < 7) {
        printf("Usage: ./process input.png levels alpha beta timing_iterations output.png\n"
               "e.g.: ./process input.png 8 1 1 10 output.png\n");
        return 0;
    }

    Buffer<uint16_t> input = load_image(argv[1]);
    int levels = atoi(argv[2]);
    float alpha = atof(argv[3]), beta = atof(argv[4]);
    Buffer<uint16_t> output(input.width()/4, input.height()/4, 3);
    int timing = atoi(argv[5]);

    // Timing code
    double best = benchmark(timing, 1, [&]() {
        gaussian_pyramid(input, levels, alpha/(levels-1), beta, output);
    });
    printf("%gus\n", best * 1e6);


    //gaussian_pyramid(input, levels, alpha/(levels-1), beta, output);

    pipeline_hls(input, levels, alpha/(levels-1), beta, output);

    save_image(output, argv[6]);

    return 0;
}
