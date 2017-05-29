/*
* Change History:
* 25/04/2017: Update using Buffer instead of Image
* 
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <hls_stream.h>

#include "HalideBuffer.h"
#include "pipeline_native.h"
#include "pipeline_hls.h"
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    Buffer<uint8_t> in     = load_image(argv[1]);

    Buffer<uint8_t> out_native(in.width(), in.height(), in.channels());
    Buffer<uint8_t> out_hls   (in.width(), in.height(), in.channels());

    printf("start.\n");

    pipeline_native(in, out_native);
    save_image(out_native, argv[2]);

    printf("finish running native code\n");

    //pipeline_hls(in, out_hls);

    hls::stream<uint16_t> output_stream("output");
    hls::stream<uint8_t>  input_stream("input");

    for (int j = 0; j < in.height(); j++) {
        for (int i = 0; i < in.width(); i++) {
            input_stream.write(in(i, j, 0));
        }
    }

    hls_target(input_stream, output_stream);
    
    //save_image(out_hls, argv[3]);
    printf("finish running HLS code\n");

    bool success = true;
    
    for (int y = 0; y < out_native.height() / 5; y++) {
        for (int x = 0; x < out_native.width() / 5; x++) {
            out_hls(x, y, 0) = output_stream.read();
	        if (out_native(x, y, 0) != out_hls(x, y, 0)) {
                    printf("out_native(%d, %d) = %d, but out_c(%d, %d) = %d\n",
			   x, y, out_native(x, y, 0), 
			   x, y, out_hls(x, y, 0));
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
