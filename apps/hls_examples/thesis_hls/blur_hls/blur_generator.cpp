/*
* Change History:
* 27/04/2017: Implement Blur application with the new enhanced generator class
* 
*/

#include "Halide.h"

using namespace Halide;

namespace {

class Blur: public Halide::Generator<Blur> {

public:
	Input<Buffer<uint8_t>>   input {"input", 3};
	
	Output<Buffer<uint8_t>>  output{"output", 3};

	RDom win;

	// Algorithm Description
	void generate() {
		win = RDom(0, 2, 0, 2);

		clamped = BoundaryConditions::repeat_edge(input);

		
		blur_y(x, y, c) = (cast<uint16_t>(clamped(x, y, c)) + 
							cast<uint16_t>(clamped(x+1, y, c)) + 
							 cast<uint16_t>(clamped(x+2, y, c))) / 3;
   		/*
   		blur_y(x, y, c) = (blur_x(x, y, c)  + blur_x(x, y+1, c) + blur_x(x, y+2, c)) / 3;
		*/

   		output(x, y, c) = cast<uint8_t> (blur_y(x, y, c));

        /*
        // Set the arguments
        args.push_back(input);
        args.push_back(weight);
        args.push_back(bias);
		*/
       
	}

	void schedule() {
		output.bound(c, 0, 3);

		if (get_target().has_hls_feature()) {
			// HLS Schedule
			std::cout << "\ncompiling HLS code..." << std::endl;

			clamped.compute_root();

			blur_y.compute_root();
	        blur_y.tile(x, y, xo, yo, xi, yi, 1920, 1080).reorder(c, xi, yi, xo, yo);
	        blur_y.accelerate({clamped}, xi, xo);  // define the inputs and the output

	        //output.reorder(c, x, y);

			//blur_x.linebuffer().unroll(c);
			blur_y.unroll(c);

		} else {
			// CPU Schedule
			//blur_y.split(y, y, yi, 8);
			// Tell the blur_x to be computed as needed per yi coordinate of blur_y
			// Tell the blur_x result to be stored per strip -> might use circular buffer (2 scanlines)
   			//blur_x.store_at(blur_y, y).compute_at(blur_y, yi);
			//clamped.compute_root();

   			//blur_x.trace_stores();
   			//blur_x.compute_root();
   			output.reorder(c, x, y).unroll(c);

		}
	}

	Func blur_x{"blur_x"}; 
	Func blur_y{"blur_y"};
	Func clamped{"clamped"};

	Var x{"x"}, y{"y"}, c{"c"};
	Var xi{"xi"}, xo{"xo"}, yi{"yi"}, yo{"yo"};

	std::vector<Argument> args;
};

HALIDE_REGISTER_GENERATOR(Blur, "blur");

} // namespace
