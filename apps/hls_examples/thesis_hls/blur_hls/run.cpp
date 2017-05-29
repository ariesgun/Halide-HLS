/*
* Change History:
* 25/04/2017: Update using Buffer instead of Image
* 
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <hls_stream.h>

#include "HalideBuffer.h"
#include "pipeline_native.h"
#include "pipeline_hls.h"
#include "halide_image_io.h"
#include "hls_target.h"

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
            //printf("write %d\n", in(i, j, 0));
            input_stream.write(in(i, j, 0));
        }
    }

    hls_target(output_stream, input_stream);
    
    //save_image(out_hls, argv[3]);
    printf("finish running HLS code\n");

    bool success = true;
    
    for (int y = 0; y < out_native.height(); y++) {
        for (int x = 0; x < out_native.width(); x++) {
            out_hls(x, y, 0) = output_stream.read();
            out_native(x, y, 1) = 0;
            out_native(x, y, 2) = 0;
	        if (out_native(x, y, 0) != out_hls(x, y, 0)) {
                    printf("out_native(%d, %d) = %d, but out_c(%d, %d) = %d\n",
			   x, y, out_native(x, y, 0), 
			   x, y, out_hls(x, y, 0));
                   success = false;
                
            }
        }
    }

    std::string outputBinFileName = "expected_output.bin";
    std::ofstream outputBinFile(outputBinFileName.c_str(), std::ios::in | std::ios::binary);

    // Now compare the results of the function with the actual golden output
    if(outputBinFile.is_open()){
        //check every value separately
        for (int y = 0; y < out_native.height(); y++) {
            for (int x = 0; x < out_native.width(); x++) {
                outputBinFile.write((char *)&out_hls(x, y, 0), sizeof(uint8_t));
            }
        }

    //if the input file is not readable exit the testbench
    } else {
        std::cout << "Unable to open file:" << outputBinFileName << "\n The program will exit" << std::endl;
    }

    save_image(out_native, argv[2]);
    save_image(out_hls, argv[3]);
    

    if (success) {
        printf("Successed!\n");
        return 0;
    } else {
        printf("Failed!\n");
        return 1;
    }

}
