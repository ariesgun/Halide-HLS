/*
* Change History:
* 24/04/2017: Update the generator class with the new enhanced generator and with updated 
*             ImageParam or GeneratorParam
* 
*/

#include "Halide.h"

using namespace Halide;

namespace {

class GaussianConv2: public Halide::Generator<GaussianConv2> {

public:
	Input<Buffer<uint8_t>>     input {"input", 2};
	Input<Buffer<uint8_t>>     weight{"weight", 2};

	Input<uint8_t>             bias{"bias"};
	
	Output<Buffer<uint8_t>>    output{"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-2, 5, -2, 5);

		// Define a 9x9 Gaussian Blur with a repeat-edge boundary condition.
        float sigma = 1.5f;

        // kernel(x, y) = cast<uint32_t>((exp(-(x*x + y*y)/(2*sigma*sigma)) / (float)(2*M_PI*sigma*sigma)) * 65535);
        kernel_f(x) = exp(-(x*x)/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);
        // Normalize and convert to 8-bit integer;
        kernel(x) = cast<uint8_t>(kernel_f(x) * 255 /
        					(kernel_f(0) + kernel_f(1)*2 + kernel_f(2)*2));

        // define the algorithm
        clamped(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        
        //conv1 = clamped;
        conv1(x, y) += cast<uint32_t>(clamped(x, y)) * kernel(win.x) * kernel(win.y);// * kernel(win.y);
        conv1_shifted(x, y) = cast<uint8_t> (conv1(x, y) >> 16);

        // unroll the reduction
        //conv1.update(0).unroll(c).unroll(win.x).unroll(win.y);
        // hw_output(x,y) = conv1_shifted(x, y);
        hw_output = convolve55_rd(conv1_shifted);
        output(x, y) = cast<uint8_t> (hw_output(x, y));
       
	}

	void schedule() {
		weight.dim(0).set_bounds(0, 5);
		weight.dim(1).set_bounds(0, 5);

        weight.dim(0).set_stride(1);
        weight.dim(1).set_stride(5);

		if (get_target().has_hls_feature()) {

			std::cout << "\ncompiling HLS code..." << std::endl;

			//kernel.compute_root();
			
			clamped.compute_root(); // prepare the input for the whole image
			
			// HLS schedule: make a hw pipeline producing 'hw_output', taking
	        // inputs of 'clamped', buffering intermediates at (output, xo) loop
	        // level
	        hw_output.compute_root();
	        hw_output.tile(x, y, xo, yo, xi, yi, 1920, 1080).reorder(xi, yi, xo, yo);
	        //hw_output.unroll(xi, 2);
	        hw_output.accelerate({clamped}, xi, xo);  // define the inputs and the output
	        conv1_shifted.linebuffer();
	        conv1.unroll(x).unroll(y);
	        
	        // unroll the reduction
	        //local_sum.update(0).unroll(win.x).unroll(win.y);

        	// unroll the reduction
        	conv1.update(0).unroll(win.x).unroll(win.y);

		} else {
			kernel.compute_root();
			
			output.tile(x, y, xo, yo, xi, yi, 256, 256);
	        output.fuse(xo, yo, xo).parallel(xo);

	        output.vectorize(xi, 8);
	        conv1_shifted.compute_at(output, xo).vectorize(x, 8);

	        // unroll the reduction
	        //local_sum.update(0).unroll(win.x).unroll(win.y);
		}
	}

	Func local_sum{"local_sum"}, res{"res"};
	Func kernel_f{"kernel_f"}, kernel{"kernel"}, clamped{"clamped"}, conv1{"conv1"}, conv1_shifted{"conv1_shifted"};
    Func hw_output{"hw_output"};
	Var x{"x"}, y{"y"}, c{"c"};
	Var xo{"xo"}, xi{"xi"}, yi{"yi"}, yo{"yo"};

private:
	Func convolve55_rd(Func in) {
        
        local_sum(x, y) = cast<uint16_t>(bias);

        local_sum(x, y) += cast<uint16_t>(in(x+win.x, y+win.y)) * weight(win.x+2, win.y+2);
        res(x, y) = cast<uint8_t>(local_sum(x, y) >> 8);

        return res;
    }
};

HALIDE_REGISTER_GENERATOR(GaussianConv2, "gaussian_conv_2");

} // namespace
