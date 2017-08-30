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
#include "pipeline_native_2.h"
#include "pipeline_native_3.h"
#include "pipeline_native_float.h"
#include "pipeline_hls.h"
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace Halide::Runtime;

const unsigned char gaussian2d[5][5] = {
    {1,     3,     6,     3,     1},
    {3,    15,    25,    15,     3},
    {6,    25,    44,    25,     6},
    {3,    15,    25,    15,     3},
    {1,     3,     6,     3,     1}
};

int main(int argc, char **argv) {
    Buffer<uint8_t> in       = load_image(argv[1]);
    Buffer<float> in2        = load_image(argv[1]);

    Buffer<uint8_t> out_native(in.width(), in.height());
    Buffer<uint8_t> out_native_2(in.width(), in.height());
    Buffer<uint8_t> out_native_3(in.width(), in.height());
    Buffer<float>   out_native_f(in.width(), in.height());
    Buffer<float>   out_native_float(in.width(), in.height());
    Buffer<float>   out_hls_float(in.width(), in.height());

    Buffer<uint8_t> weight(5, 5);
    for (int y = 0; y < 5; y++)
        for (int x = 0; x < 5; x++)
            weight(x, y) = gaussian2d[y][x];

    printf("start.\n");

    printf("running manual-fixed-point implementation\n");
    pipeline_native(in, weight, 0, out_native);
    for (int y = 0; y < in.height(); y++)
        for (int x = 0; x < in.width(); x++)
            out_native_f(x, y) = out_native(x, y) / 255.0f;
    save_image(out_native_f, argv[2]);
    pipeline_native_2(in, weight, 0, out_native_2);
    for (int y = 0; y < in.height(); y++)
        for (int x = 0; x < in.width(); x++)
            out_native_f(x, y) = out_native_2(x, y) / 255.0f;
    save_image(out_native_f, argv[3]);
    pipeline_native_3(in, weight, 0, out_native_3);
    for (int y = 0; y < in.height(); y++)
        for (int x = 0; x < in.width(); x++)
            out_native_f(x, y) = out_native_3(x, y) / 255.0f;
    save_image(out_native_f, argv[4]);
    printf("finish running manual-fixed-point implementation\n\n");

    printf("Running floating-point implementation\n");
    pipeline_native_float(in2, weight, 0, out_native_float);
    save_image(out_native_float, argv[3]);
    printf("finish running floating-point implementation\n\n");

    printf("Running fixed-point implementation\n");
    pipeline_hls(in2, weight, 0, out_hls_float);
    save_image(out_hls_float, argv[4]);
    printf("finish running fixed-point implementation\n\n");

    bool success = true;
    float min_error_float = 1.0f, max_error_float = 0.0f; // error for manual fixed
    float min_error_fixed = 1.0f, max_error_fixed = 0.0f; // error for fixed-point 
    
    double diff_1 = 0.0f;
    double diff_2 = 0.0f;
    double diff_3 = 0.0f;
    double diff_4 = 0.0f;
    double sum_temp = 0.0f;

    for (int y = 0; y < out_native.height(); y++) {
        for (int x = 0; x < out_native.width(); x++) {
	        if (true || out_native(x, y) != in2(x, y)) {
      //               printf("out_native_float(%d, %d) = %f, but out_native_fixed(%d, %d) = %f, out_hls_fixed(%d, %d) = %f\n",
			   // x, y, out_native_float(x, y),
			   // x, y, out_native(x, y) / 255.0f,
      //          x, y, out_hls_float(x, y));     

                    double error_float = out_native(x,y) / 255.0f - out_native_float(x,y);\
                    double error_float_2 = out_native_2(x,y) / 255.0f - out_native_float(x,y);
                    double error_float_3 = out_native_3(x,y) / 255.0f - out_native_float(x,y);
                    
                    if (error_float < 0) {
                        error_float = -1 * error_float;
                    }
                    double error_fixed = out_hls_float(x,y) - out_native_float(x,y);
                    if (error_fixed < 0) {
                        error_fixed = -1 * error_fixed;
                    }

                    // printf("    Error float-manual_fixed %f, float-fixed-point %f\n", error_float, error_fixed );
                    min_error_float = (error_float < min_error_float) ? error_float : min_error_float;
                    max_error_float = (error_float > max_error_float) ? error_float : max_error_float;

                    min_error_fixed = (error_fixed < min_error_fixed) ? error_fixed : min_error_fixed;
                    max_error_fixed = (error_fixed > max_error_fixed) ? error_fixed : max_error_fixed;
                    success = false;

                    diff_1 += error_float * error_float;
                    diff_2 += error_fixed * error_fixed;
                    diff_3 += error_float_2 * error_float_2;
                    diff_4 += error_float_3 * error_float_3;
                    sum_temp += out_native_float(x, y) * out_native_float(x, y);
            }
        }
    }

    double MSE = sum_temp / diff_1;
    double MSE_2 = sum_temp / diff_2;
    double MSE_3 = sum_temp / diff_3;
    double MSE_4 = sum_temp / diff_4;

    printf(" SNR Float %lf: %lf\n",   MSE,   10*log10(MSE));
    printf(" SNR Float 2 %lf: %lf\n", MSE_3, 10*log10(MSE_3));
    printf(" SNR Float 3 %lf: %lf\n", MSE_4, 10*log10(MSE_4));
    printf(" SNR Fixed %lf: %lf\n",   MSE_2, 10*log10(MSE_2));

    printf(" Max error float %f; Min error Float %f\n", max_error_float, min_error_float);
    printf(" Max error fixed %f; Min error fixed %f\n", max_error_fixed, min_error_fixed);
    
    if (success) {
        printf("Successed!\n");
        return 0;
    } else {
        printf("Failed!\n");
        return 1;
    }

}
