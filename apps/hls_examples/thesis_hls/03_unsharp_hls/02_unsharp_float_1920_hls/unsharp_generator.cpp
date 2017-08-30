/*
* Change History:
* 24/04/2017: Update the generator class with the new enhanced generator and with updated 
*             ImageParam or GeneratorParam
* 
*/

#include "Halide.h"

using namespace Halide;

namespace {

class UnsharpFilter: public Halide::Generator<UnsharpFilter> {

public:
	Input<Buffer<float>>     input {"input", 2};	
	Output<Buffer<float>>    output{"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-4, 9);

		// Define a 9x9 Gaussian Blur with a repeat-edge boundary condition.
        float sigma = 1.5f;

        kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);
        // normalize and convert to 8bit fixed point
        kernel(x) = kernel_f(x) / 
        			(kernel_f(0) + kernel_f(1) * 2 + kernel_f(2) * 2 + kernel_f(3) * 2 + kernel_f(4) * 2);

        // define the algorithm
        clamped(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        gray(x, y) = clamped(x, y);
        // 2D filter: direct map
        blur_y(x, y) += gray(x, y+win.x) * kernel(win.x);
        blur_x(x, y) += blur_y(x + win.x, y) * kernel(win.x);
 
        sharpen(x, y) = clamp(clamped(x, y) - blur_x(x, y), 0.0f, 1.0f);
        hw_output(x, y) = clamp(sharpen(x, y) + clamped(x, y), 0.0f, 1.0f); 
        // hw_output(x, y) = blur_x(x, y);
        output(x, y) = cast<float>(hw_output(x, y));
       
	}

	void schedule() {
		if (get_target().has_hls_feature()) {

			std::cout << "\ncompiling HLS code..." << std::endl;

        	hw_output.compute_root();
	        hw_output.tile(x, y, xo, yo, xi, yi, 1920, 1080).reorder(xi, yi, xo, yo);
	        clamped.compute_root();

	        blur_y.update(0).unroll(win.x);
	        blur_x.update(0).unroll(win.x);

	        hw_output.accelerate({clamped}, xi, xo);
	        blur_y.linebuffer();
	        gray.linebuffer();
	        //gray.linebuffer().fifo_depth(sharpen, 20);
	        // sharpen.linebuffer();
	        clamped.fifo_depth(hw_output, 1920*9); // hw input bounds

		} else {
			output.tile(x, y, xo, yo, xi, yi, 64, 64);
			
			sharpen.compute_at(output, xo).vectorize(x, 8);
	        output.vectorize(xi, 8).reorder(xi, yi, xo, yo);

	        blur_y.update(0).unroll(win.x);
	        blur_x.update(0).unroll(win.x);

	        output.fuse(xo, yo, xo).parallel(xo);
		}
	}

	Func blur_x{"blur_x"}, blur_y{"blur_y"}, gray{"gray"};
	Func kernel_f{"kernel_f"}, kernel{"kernel"}, clamped{"clamped"};
	Func sharpen{"sharpen"};
    Func hw_output{"hw_output"};
	Var x{"x"}, y{"y"}, c{"c"};
	Var xo{"xo"}, xi{"xi"}, yi{"yi"}, yo{"yo"};

	std::vector<Argument> args;
};

HALIDE_REGISTER_GENERATOR(UnsharpFilter, "unsharp_filter");

} // namespace
