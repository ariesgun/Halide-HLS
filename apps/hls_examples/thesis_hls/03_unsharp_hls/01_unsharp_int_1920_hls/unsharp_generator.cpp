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
	Input<Buffer<uint8_t>>     input {"input", 2};	
	Output<Buffer<uint8_t>>    output{"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-4, 9, -4, 9);

		// Define a 9x9 Gaussian Blur with a repeat-edge boundary condition.
        float sigma = 1.5f;

        // kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);
        kernel_f(x, y) = (exp(-(x*x + y*y)/(2*sigma*sigma)) / (float)(2*M_PI*sigma*sigma));
        // // normalize and convert to 8bit fixed point
        // kernel(x) = cast<uint8_t>(kernel_f(x) * 255 /
        //                           (kernel_f(0) + kernel_f(1)*2 + kernel_f(2)*2
        //                            + kernel_f(3)*2 + kernel_f(4)*2));

         kernel(x, y) = cast<uint8_t>(kernel_f(x, y) * 255/ 
         			(( (kernel_f(4,4) * 2 + kernel_f(3,4) * 2 + kernel_f(2,4) * 2 + kernel_f(1,4)*2 + kernel_f(0,4)) + 
         			  (kernel_f(4,3) * 2 + kernel_f(3,3) * 2 + kernel_f(2,3) * 2 + kernel_f(1,3)*2 + kernel_f(0,3)) +
         			  (kernel_f(4,2) * 2 + kernel_f(3,2) * 2 + kernel_f(2,2) * 2 + kernel_f(1,2)*2 + kernel_f(0,2)) +
         			  (kernel_f(4,1) * 2 + kernel_f(3,1) * 2 + kernel_f(2,1) * 2 + kernel_f(1,1)*2 + kernel_f(0,1)) ) * 2 +
         			( kernel_f(4,0) * 2 + kernel_f(3,0) * 2 + kernel_f(2,0) * 2 + kernel_f(1,0) * 2 + kernel_f(0,0)) ));
         			  
        // define the algorithm
        clamped(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
		gray(x, y) = clamped(x, y);	        
        // 2D filter: direct map
        sum_x(x, y) += cast<uint16_t>(gray(x+win.x, y+win.y)) * kernel(win.x, win.y);
        blur_x(x, y) = cast<uint8_t>(sum_x(x, y) >> 8);
        
        sharpen(x, y) = cast<uint8_t>(clamp(2 * cast<uint16_t>(gray(x, y)) - blur_x(x, y), 0, 255));

        ratio(x, y) = cast<uint8_t>(clamp(cast<uint16_t>(sharpen(x, y)) * 32 / max(clamped(x, y), 1), 0, 255));

        hw_output(x, y) = cast<uint8_t>(clamp(cast<uint16_t>(ratio(x, y)) * clamped(x, y) / 32, 0, 255));

        output(x, y) = hw_output(x, y);
       
	}

	void schedule() {
		if (get_target().has_hls_feature()) {

			std::cout << "\ncompiling HLS code..." << std::endl;

	        hw_output.compute_root();
	        hw_output.tile(x, y, xo, yo, xi, yi, 1920, 1080).reorder(xi, yi, xo, yo);
	        clamped.compute_root();

	        sum_x.update(0).unroll(win.x).unroll(win.y);

	        hw_output.accelerate({clamped}, xi, xo);
	        gray.linebuffer().fifo_depth(ratio, 20);
	        // blur_x.linebuffer();
	        ratio.linebuffer();
	        clamped.fifo_depth(hw_output, 1920*9); // hw input bounds

		} else {
			output.tile(x, y, xo, yo, xi, yi, 256, 256);
			
			ratio.compute_at(output, xo).vectorize(x, 8);
	        output.vectorize(xi, 8).reorder(xi, yi, xo, yo);

	        sum_x.update(0).unroll(win.x).unroll(win.y);

	        output.fuse(xo, yo, xo).parallel(xo);
		}
	}

	Func sum_x{"sum_x"}, blur_x{"blur_x"}, gray{"gray"};
	Func kernel_f{"kernel_f"}, kernel{"kernel"}, clamped{"clamped"};
	Func sharpen{"sharpen"}, ratio{"ratio"};
    Func hw_output{"hw_output"};
	Var x{"x"}, y{"y"}, c{"c"};
	Var xo{"xo"}, xi{"xi"}, yi{"yi"}, yo{"yo"};

	std::vector<Argument> args;
};

HALIDE_REGISTER_GENERATOR(UnsharpFilter, "unsharp_filter");

} // namespace
