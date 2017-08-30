/*
* Change History:
* 14/04/2017: Update the generator class with the new enhanced generator and with updated 
*             ImageParam or GeneratorParam
* 
*/

#include "Halide.h"

using namespace Halide;

namespace {

class BilateralGrid : public Halide::Generator<BilateralGrid> {
public:
    GeneratorParam<int>   s_sigma{"s_sigma", 8};

    Input<Buffer<float>>  input{"input", 2};
    Input<float>          r_sigma{"r_sigma"};

    Output<Buffer<float>> output{"output", 2};

    // Algorithm Description
    void generate() {
        //int s_sigma = 8;
        r = RDom(0, s_sigma, 0, s_sigma);
        
        // Add a boundary condition
        clamped(x,y) = BoundaryConditions::repeat_edge(input)(x,y);

        // Construct the bilateral grid
        Expr val = clamped(x * s_sigma + r.x - s_sigma/2, y * s_sigma + r.y - s_sigma/2);
        val = clamp(val, 0.0f, 1.0f);

        Expr zi = cast<int>(val * (1.0f/0.1f) + 0.5f);

        // Histogram
        histogram(x, y, z, c) = 0.0f;
        histogram(x, y, zi, c) += select(c == 0, val, 1.0f);

        // Blur the grid using a five-tap filter
        blurz(x, y, z, c) = (histogram(x, y, z-2, c) +
                             histogram(x, y, z-1, c)*4 +
                             histogram(x, y, z  , c)*6 +
                             histogram(x, y, z+1, c)*4 +
                             histogram(x, y, z+2, c));
        blurx(x, y, z, c) = (blurz(x-2, y, z, c) +
                             blurz(x-1, y, z, c)*4 +
                             blurz(x  , y, z, c)*6 +
                             blurz(x+1, y, z, c)*4 +
                             blurz(x+2, y, z, c));
        blury(x, y, z, c) = (blurx(x, y-2, z, c) +
                             blurx(x, y-1, z, c)*4 +
                             blurx(x, y  , z, c)*6 +
                             blurx(x, y+1, z, c)*4 +
                             blurx(x, y+2, z, c));

        // Take trilinear samples to compute the output
        input2(x, y) = input(x, y);
        val     = clamp(input2(x, y), 0.0f, 1.0f);
        Expr zv = val * (1.0f/0.1f);
        zi      = cast<int>(zv);
        Expr zf = zv - zi;
        Expr xf = cast<float>(x % s_sigma) / s_sigma;
        Expr yf = cast<float>(y % s_sigma) / s_sigma;
        Expr xi = x/s_sigma;
        Expr yi = y/s_sigma;
        
        interpolated(x, y, c) =
            lerp(lerp(lerp(blury(xi, yi, zi, c), blury(xi+1, yi, zi, c), xf),
                      lerp(blury(xi, yi+1, zi, c), blury(xi+1, yi+1, zi, c), xf), yf),
                 lerp(lerp(blury(xi, yi, zi+1, c), blury(xi+1, yi, zi+1, c), xf),
                      lerp(blury(xi, yi+1, zi+1, c), blury(xi+1, yi+1, zi+1, c), xf), yf), zf);

        // Normalize and return the output.
        bilateral_grid(x, y) = interpolated(x, y, 0)/interpolated(x, y, 1);
        output(x,y)          = bilateral_grid(x,y);

    }

    // Scheduling
    void schedule() { 
        // int s_sigma = 8;
        if (get_target().has_gpu_feature()) {
            // The GPU schedule
            Var xi{"xi"}, yi{"yi"}, zi{"zi"};

            // Schedule blurz in 8x8 tiles. This is a tile in
            // grid-space, which means it represents something like
            // 64x64 pixels in the input (if s_sigma is 8).
            blurz.compute_root().reorder(c, z, x, y).gpu_tile(x, y, xi, yi, 8, 8);

            // Schedule histogram to happen per-tile of blurz, with
            // intermediate results in shared memory. This means histogram
            // and blurz makes a three-stage kernel:
            // 1) Zero out the 8x8 set of histograms
            // 2) Compute those histogram by iterating over lots of the input image
            // 3) Blur the set of histograms in z
            histogram.reorder(c, z, x, y).compute_at(blurz, x).gpu_threads(x, y);
            histogram.update().reorder(c, r.x, r.y, x, y).gpu_threads(x, y).unroll(c);

            // An alternative schedule for histogram that doesn't use shared memory:
            // histogram.compute_root().reorder(c, z, x, y).gpu_tile(x, y, xi, yi, 8, 8);
            // histogram.update().reorder(c, r.x, r.y, x, y).gpu_tile(x, y, xi, yi, 8, 8).unroll(c);

            // Schedule the remaining blurs and the sampling at the end similarly.
            blurx.compute_root().gpu_tile(x, y, z, xi, yi, zi, 8, 8, 1);
            blury.compute_root().gpu_tile(x, y, z, xi, yi, zi, 8, 8, 1);
            bilateral_grid.compute_root().gpu_tile(x, y, xi, yi, s_sigma, s_sigma);
        } else if (get_target().has_hls_feature()) {
            blury.linebuffer().compute_at(bilateral_grid, x_in);
            blurx.linebuffer().compute_at(bilateral_grid, x_in);
            blurz.linebuffer().compute_at(bilateral_grid, x_in);

            histogram.linebuffer().compute_at(blurz, x_in).reorder(c, z, x, y).unroll(c).unroll(z);
            histogram.update().reorder(c, r.x, r.y, x, y).unroll(c);

            clamped.compute_root();
            input2.compute_root();

            // bilateral_grid.tile(x, y, xo, yo, x_in, y_in, 1536, 2560);
            bilateral_grid.tile(x, y, xo, yo, x_in, y_in, 480, 640);
            bilateral_grid.tile(x_in, y_in, x_grid, y_grid, x_in, y_in, 8, 8);
            bilateral_grid.compute_root();
            bilateral_grid.accelerate({clamped, input2}, x_grid, xo);

        } else {
            // The CPU schedule.
            blurz.compute_root().reorder(c, z, x, y).parallel(y).vectorize(x, 8).unroll(c);
            histogram.compute_at(blurz, y);
            histogram.update().reorder(c, r.x, r.y, x, y).unroll(c);
            blurx.compute_root().reorder(c, x, y, z).parallel(z).vectorize(x, 8).unroll(c);
            blury.compute_root().reorder(c, x, y, z).parallel(z).vectorize(x, 8).unroll(c);
            bilateral_grid.compute_root().parallel(y).vectorize(x, 8);
        }
    }

    Func clamped{"clamped"}, histogram{"histogram"};
    Func input2{"input2"};
    Func bilateral_grid{"bilateral_grid"};
    Func blurx{"blurx"}, blury{"blury"}, blurz{"blurz"}, interpolated{"interpolated"};
    Var x{"x"}, y{"y"}, z{"z"}, c{"c"};
    Var x_in{"x_in"}, y_in{"y_in"}, xo{"xo"}, yo{"yo"}, x_grid{"x_grid"}, y_grid{"y_grid"};
    RDom r;

};

//Halide::RegisterGenerator<BilateralGrid> register_me{"bilateral_grid"};
HALIDE_REGISTER_GENERATOR(BilateralGrid, "bilateral_grid");

}  // namespace
