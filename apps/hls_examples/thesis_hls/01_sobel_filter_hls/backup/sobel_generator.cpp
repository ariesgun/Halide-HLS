/*
* Change History:
* 24/04/2017: Update the generator class with the new enhanced generator and with updated 
*             ImageParam or GeneratorParam
* 
*/

// gray.cast_to(fixed_point_t).set_bit_width(total, fraction);
// conv_x.cast_to(fixed_point_t).set_bit_width(total, fraction);
// conv_y.cast_to(fixed_point_t).set_bit_width(total, fraction);
// hw_output.cast_to(fixed_point_t).set_bit_width(total, fraction);

// output(x, y) = x*y + p*q

// Input and output are still using standard data C types.
// Conversion from fixed-point to floating-point is handled by Xilinx fixed-point library.

// The type for Expr val will be determined automatically from the input Funcs' types.
// Mathematical operations that do not support fixed-point will be converted back to floating-point and than back to fixed point if it is used.

// Data Type IR Transformation.

#include "Halide.h"

using namespace Halide;

namespace {

class SobelConv: public Halide::Generator<SobelConv> {

public:
	Input<Buffer<uint8_t>>  input   {"input", 3};
	Input<Buffer<int8_t>>   kernel_x{"kernel_x", 2};
	Input<Buffer<int8_t>>   kernel_y{"kernel_y", 2};
	
	Output<Buffer<uint8_t>> output  {"output", 2};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(-1, 3, -1, 3);

        // RGB Conversion
        clamped = BoundaryConditions::repeat_edge(input);
        gray(x, y) = ((0.299f * clamped(x, y, 0)) + (0.587f * clamped(x, y, 1)) + (0.114f * clamped(x, y, 2)));

        //conv1 = clamped;
        conv_x(x, y) += gray(x+win.x, y+win.y) * kernel_x(win.x + 1, win.y + 1);
        conv_y(x, y) += gray(x+win.x, y+win.y) * kernel_y(win.x + 1, win.y + 1);

        val = sqrt(( conv_x(x, y) * conv_x(x, y) + conv_y(x, y) * conv_y(x, y) ));
        //val = abs(conv_x(x, y)) + abs(conv_y(x, y));
        hw_output(x, y) = (clamp(val, 0, 255));
        //output(x, y) = cast<uint8_t> (clamp(hw_output(x, y), 0.0f, 1.0f) * 255);3
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

		kernel_x.dim(0).set_bounds(0, 3);
		kernel_x.dim(1).set_bounds(0, 3);
		kernel_y.dim(0).set_bounds(0, 3);
		kernel_y.dim(1).set_bounds(0, 3);		

        kernel_x.dim(0).set_stride(1);
        kernel_x.dim(1).set_stride(3);
        kernel_y.dim(0).set_stride(1);
        kernel_y.dim(1).set_stride(3);

        output.dim(0).set_stride(1);

		if (get_target().has_hls_feature()) {

			std::cout << "\ncompiling HLS code..." << std::endl;

			//gray.compute_root();
			clamped.compute_root(); // prepare the input for the whole image

			// HLS schedule: make a hw pipeline producing 'hw_output', taking
	        // inputs of 'clamped', buffering intermediates at (output, xo) loop
	        // level
	        hw_output.compute_root();
	        //hw_output.tile(x, y, xo, yo, xi, yi, 1920, 1080).reorder(c, xi, yi, xo, yo);
	        hw_output.tile(x, y, xo, yo, xi, yi, 640, 480).reorder(xi, yi, xo, yo);
	        //hw_output.unroll(xi, 2);
	        hw_output.accelerate({clamped}, xi, xo);  // define the inputs and the output
	        conv_x.linebuffer();
	        conv_x.unroll(x).unroll(y);
	        conv_y.linebuffer();
	        conv_y.unroll(x).unroll(y);

	        // Linebuffering gray
	        gray.linebuffer().compute_at(hw_output, xi);

        	// unroll the reduction
        	conv_x.update(0).unroll(win.x).unroll(win.y);
        	conv_y.update(0).unroll(win.x).unroll(win.y);

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