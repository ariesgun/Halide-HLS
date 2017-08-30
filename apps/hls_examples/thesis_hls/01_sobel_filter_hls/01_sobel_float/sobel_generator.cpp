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
	Input<Buffer<float>>  input    {"input", 3};
	Output<Buffer<float>> output   {"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-1, 3, -1, 3);

        // RGB Conversion
        clamped = BoundaryConditions::repeat_edge(input);
        gray(x, y) = ((0.299f * clamped(x, y, 0)) + (0.587f * clamped(x, y, 1)) + (0.114f * clamped(x, y, 2)));

        conv_x(x, y) = ( (-1 * gray(x-1, y-1)) + (1 * gray(x+1, y-1))  + 
        				 (-2 * gray(x-1, y))   + (2 * gray(x+1, y)) + 
        				 (-1 * gray(x-1, y+1)) + (1 * gray(x+1, y+1)) ); 
        conv_y(x, y) = ( (-1 * gray(x-1, y-1)) + (-2 * gray(x, y-1)) + (-1 * gray(x+1, y-1))  + 
        				 (1  * gray(x-1, y+1)) + (2 * gray(x, y+1)) + (1 * gray(x+1, y+1)) ); 

        val = abs(conv_x(x, y)) + abs(conv_y(x, y));
        hw_output(x, y) = clamp(val, 0.0f, 1.0f);
        output(x, y) = hw_output(x, y);

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

		// if (get_target().has_hls_feature()) {
		// 	//gray.cast_to(UInt(8));
		// 	//conv_x.cast_to(Int(16));
		// 	//conv_y.cast_to(Int(16));
		// 	//hw_output.cast_to(UInt(8));
		// 	//gray.cast_to(Uint(8), {R, Int8}, {G, Int8}, {B, Int8});
			
		// 	// Data promotion rule: Floating point is higher than fixed point.
		// 	// Hence, clamped will be casted to float. And then at the end of the computation, the result will be casted back to fixedpoint again?
		// 	// So, the operation will be done in floating point.
		// 	// Hence, To do the operation in fixed-point, we need to specify that th floating point constants should be converted back to fixed-point.

		// 	// Alternative way is to allow all constants which are in floating point to be casted to fixed-point. but is it a good idea?
		// 		// If the output is in fixed-point then, all of the operations should be in fixed-point? Automatic conversion. But no flexibility?

		// 	gray.cast_to(FixedPoint(20, 8), {{Expr(0.299f), FixedPoint(12, 0)}, {Expr(0.587f), FixedPoint(12, 0)}, {Expr(0.114f), FixedPoint(12,0)}});
		// 	//hw_output.cast_to(FixedPoint(UInt(8), {{val, FixedPoint()}}), );
		// 	hw_output.cast_to(UInt(8));
		// 	//gray.cast_to(FixedPoint(8,8));
		// 	kernel_x.cast_to(FixedPoint(3,3));
		// 	kernel_y.cast_to(FixedPoint(3,3));
		// 	conv_y.cast_to(FixedPoint(12,12));
		// 	conv_x.cast_to(FixedPoint(12,12));
		// }	
	}

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