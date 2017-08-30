/*
* Change History:
* 24/04/2017: Update the generator class with the new enhanced generator and with updated 
*             ImageParam or GeneratorParam
* 
*/

#include "Halide.h"

using namespace Halide;

namespace {

class SobelConv: public Halide::Generator<SobelConv> {

public:
	Input<Buffer<float>>  input     {"input", 3};
	Output<Buffer<float>> output    {"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-1, 3, -1, 3);

        // RGB Conversion
        clamped(x, y, c) = BoundaryConditions::repeat_edge(input)(x, y, c);
        R_const = 0.299f;
        G_const = 0.587f;
        B_const = 0.114f;
        zero_f  = 0.0f;
        one_f   = 1.0f;
        gray(x, y) = ((R_const * clamped(x, y, 0)) + (G_const * clamped(x, y, 1)) + (B_const * clamped(x, y, 2)));
        
        conv_x(x, y) = ( (-1 * gray(x-1, y-1)) + (1 * gray(x+1, y-1))  + 
        				 (-2 * gray(x-1, y))   + (2 * gray(x+1, y)) + 
        				 (-1 * gray(x-1, y+1)) + (1 * gray(x+1, y+1)) ); 
        conv_y(x, y) = ( (-1 * gray(x-1, y-1)) + (-2 * gray(x, y-1)) + (-1 * gray(x+1, y-1))  + 
        				 (1  * gray(x-1, y+1)) + (2 * gray(x, y+1)) + (1 * gray(x+1, y+1)) ); 

        val = abs(conv_x(x, y)) + abs(conv_y(x, y));
        hw_output(x, y) = clamp(val, zero_f, one_f);
        output(x, y) = cast<float>(hw_output(x, y));

        /*
        // Set the arguments
        args.push_back(input);
        args.push_back(weight);
        args.push_back(bias);
		*/
	}

	void schedule() {
		input.dim(2).set_bounds(0, 3);

        output.dim(0).set_stride(1);

		if (get_target().has_hls_feature()) {

			std::cout << "\ncompiling HLS code..." << std::endl;

			//gray.compute_root();
			clamped.compute_root(); // prepare the input for the whole image

			// HLS schedule: make a hw pipeline producing 'hw_output', taking
	        // inputs of 'clamped', buffering intermediates at (output, xo) loop
	        // level
	        hw_output.compute_root();
	        hw_output.tile(x, y, xo, yo, xi, yi, 1920, 1080).reorder(xi, yi, xo, yo);
	        // hw_output.tile(x, y, xo, yo, xi, yi, 640, 480).reorder(xi, yi, xo, yo);
	        //hw_output.unroll(xi, 2);
	        hw_output.accelerate({clamped}, xi, xo);  // define the inputs and the output
	        conv_x.linebuffer();
	        conv_x.unroll(x).unroll(y);
	        conv_y.linebuffer();
	        conv_y.unroll(x).unroll(y);

	        // Linebuffering gray
	        gray.linebuffer().compute_at(hw_output, xi);

		} else {

			std::cout << "\ncompiling CPU code..." << std::endl;

			gray.compute_root();
			
			output.tile(x, y, xo, yo, xi, yi, 256, 256);
	        output.fuse(xo, yo, xo).parallel(xo);

	        output.vectorize(xi, 8);
	        conv_x.compute_at(output, xo).vectorize(x, 8);
	        conv_y.compute_at(output, xo).vectorize(x, 8);
	       
		}
	}

	void type_schedule() {

		if (get_target().has_hls_feature()) {
			clamped.cast_to(UFixedPoint(12,1));
			gray.cast_to(UFixedPoint(12, 1), {{R_const, UFixedPoint(12, 0)}, {G_const, UFixedPoint(12, 0)}, {B_const, UFixedPoint(12,0)}});
			conv_y.cast_to(FixedPoint(15,3));
			conv_x.cast_to(FixedPoint(15,3));
			hw_output.cast_to(UFixedPoint(12,1), {{zero_f, UFixedPoint(1,1)}, {one_f, UFixedPoint(1,1)}});
		}
	}

	Expr R_const, G_const, B_const, zero_f, one_f;
	Func gray{"gray"};
	Func clamped{"clamped"}, conv_x{"conv_x"}, conv_y{"conv_y"};
    Func hw_output{"hw_output"};
	Var x{"x"}, y{"y"}, c{"c"};
	Var xo{"xo"}, xi{"xi"}, yi{"yi"}, yo{"yo"};
	Expr val{"val"};

	std::vector<Argument> args;
};

HALIDE_REGISTER_GENERATOR(SobelConv, "sobel_conv");

} // namespace