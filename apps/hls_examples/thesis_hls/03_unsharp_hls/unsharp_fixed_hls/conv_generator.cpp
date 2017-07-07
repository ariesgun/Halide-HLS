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
	Input<Buffer<float>>     input {"input", 2};
	Input<Buffer<uint8_t>>   weight{"weight", 2};

	Input<float>             bias{"bias"};
	
	Output<Buffer<float>>    output{"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-2, 5, -2, 5);

		// Define a 9x9 Gaussian Blur with a repeat-edge boundary condition.
        float sigma = 1.5f;

        kernel(x, y) = (exp(-(x*x + y*y)/(2*sigma*sigma)) / (float)(2*M_PI*sigma*sigma));

        // define the algorithm
        clamped(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        
        //conv1 = clamped;
        conv1(x, y) += clamped(x+win.x, y+win.y) * kernel(win.x, win.y);

        // unroll the reduction
        //conv1.update(0).unroll(c).unroll(win.x).unroll(win.y);
        //hw_output(x,y) = conv1(x,y);
        hw_output = convolve55_rd(conv1);
        output(x, y) = cast<float> (hw_output(x, y));
       
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
	        conv1.linebuffer();
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
	        conv1.compute_at(output, xo).vectorize(x, 8);

	        // unroll the reduction
	        //local_sum.update(0).unroll(win.x).unroll(win.y);
		}
	}

	void type_schedule() {
		
		if (get_target().has_hls_feature()) {

			clamped.cast_to(FixedPoint(20, 1)); // FLoat to signed fixed point conversion
			kernel.cast_to(FixedPoint(21, 1));
			conv1.cast_to(FixedPoint(41, 1));
			local_sum.cast_to(FixedPoint(52, 9));
			res.cast_to(FixedPoint(60, 1));
			hw_output.cast_to(FixedPoint(60, 1));
		}
		
	}

	Func local_sum{"local_sum"}, res{"res"};
	Func kernel{"kernel"}, clamped{"clamped"}, conv1{"conv1"};
    Func hw_output{"hw_output"};
	Var x{"x"}, y{"y"}, c{"c"};
	Var xo{"xo"}, xi{"xi"}, yi{"yi"}, yo{"yo"};

	std::vector<Argument> args;

private:
	Func convolve55_rd(Func in) {
        
        local_sum(x, y) = bias;

        local_sum(x, y) += in(x+win.x, y+win.y) * weight(win.x+2, win.y+2);
        //res(x, y, c) = cast<uint8_t>(local_sum(x, y, c) >> 8);
        res(x, y) = local_sum(x, y) / 256;

        return res;
    }
};

HALIDE_REGISTER_GENERATOR(GaussianConv, "gaussian_conv");

} // namespace
