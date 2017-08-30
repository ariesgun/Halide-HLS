/*
* Change History:
* 24/04/2017: Update the generator class with the new enhanced generator and with updated 
*             ImageParam or GeneratorParam
* 
*/

#include "Halide.h"

using namespace Halide;

namespace {

class GaussianConv: public Halide::Generator<GaussianConv> {

public:
	Input<Buffer<uint16_t>>   input {"input", 2};
	Input<Buffer<uint8_t>>   weight{"weight", 2};

	Input<float>             bias{"bias"};
	
	Output<Buffer<uint16_t>>    output{"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-2, 5, -2, 5);

		// Define a 9x9 Gaussian Blur with a repeat-edge boundary condition.
        float sigma = 1.5f;
        // kernel_f(x, y) = (exp(-(x*x + y*y)/(2*sigma*sigma)) / (float)(2*M_PI*sigma*sigma));
        // // kernel(x, y) = cast<uint8_t>(kernel_f(x, y) * 255/ 
        kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);
        // 			(kernel_f(0, 0) + kernel_f(1, 0) * 4 + kernel_f(2, 0) * 4 + kernel_f(1, 1) * 4 + kernel_f(1, 2) * 4 +
        // 			 kernel_f(2, 1) * 4 + kernel_f(2, 2) * 4));

        // Normalize and convert to 8bit fixed point.
        // Kernel values will inlined into  the blurring kernel as constant
        kernel(x) = cast<uint8_t>(kernel_f(x) * 255 /
                                  (kernel_f(0) + kernel_f(1)*2 + kernel_f(2)*2));

        // define the algorithm
        clamped = BoundaryConditions::repeat_edge(input);
        
        //conv1 = clamped;
        sum(x, y) += cast<uint32_t>(clamped(x+win.x, y+win.y)) * kernel(win.x) * kernel(win.y);
        conv1(x, y) = cast<uint16_t>(sum(x, y) >> 16);

        hw_output(x, y) = conv1(x, y);
        output(x, y) = hw_output(x, y);
       
	}

	void schedule() {
		if (get_target().has_hls_feature()) {

			std::cout << "\ncompiling HLS code..." << std::endl;

			clamped.compute_root(); // prepare the input for the whole image
			
			// HLS schedule: make a hw pipeline producing 'hw_output', taking
	        // inputs of 'clamped', buffering intermediates at (output, xo) loop
	        // level
	        hw_output.compute_root();
	        hw_output.tile(x, y, xo, yo, xi, yi, 1920, 1080).reorder(xi, yi, xo, yo);
	        //hw_output.unroll(xi, 2);
	        hw_output.accelerate({clamped}, xi, xo);  // define the inputs and the output
	        sum.unroll(x).unroll(y);
	        conv1.linebuffer();

        	// unroll the reduction
        	sum.update(0).unroll(win.x).unroll(win.y);

		} else {
			kernel.compute_root();
			
			output.tile(x, y, xo, yo, xi, yi, 256, 256);
	        output.fuse(xo, yo, xo).parallel(xo);

	        output.vectorize(xi, 8);
	        conv1.compute_at(output, xo).vectorize(x, 8);

	    }
	}

	Func local_sum{"local_sum"}, res{"res"};
	Func kernel_f{"kernel_f"};
	Func kernel{"kernel"}, clamped{"clamped"}, conv1{"conv1"}, sum{"sum"};
    Func hw_output{"hw_output"};
	Var x{"x"}, y{"y"}, c{"c"};
	Var xo{"xo"}, xi{"xi"}, yi{"yi"}, yo{"yo"};

	std::vector<Argument> args;

};

HALIDE_REGISTER_GENERATOR(GaussianConv, "gaussian_conv");

} // namespace
