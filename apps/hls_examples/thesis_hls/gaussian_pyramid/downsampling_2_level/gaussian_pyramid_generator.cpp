#include "Halide.h"

namespace {

constexpr int maxJ = 20;

class GaussianPyramid : public Halide::Generator<GaussianPyramid> {
public:

    GeneratorParam<int>   pyramid_levels{"pyramid_levels", 3, 1, maxJ};

    ImageParam            input{UInt(16), 3, "input"};
    Param<int>            levels{"levels"};
    Param<float>          alpha{"alpha"};
    Param<float>          beta{"beta"};

    Func build() {
        /* THE ALGORITHM */
        const int J = pyramid_levels;

        // Make the remapping function as a lookup table.
        Func remap;
        Expr fx = cast<float>(x) / 256.0f;
        remap(x) = alpha*fx*exp(-fx*fx/2.0f);

        // Set a boundary condition
        Func clamped = Halide::BoundaryConditions::repeat_edge(input);

        // Convert to floating point
        Func floating;
        floating(x, y, c) = clamped(x, y, c) / 65535.0f;

        // Get the luminance channel
        //Func gray;
        //gray(x, y) = 0.299f * floating(x, y, 0) + 0.587f * floating(x, y, 1) + 0.114f * floating(x, y, 2);

        // Make the processed Gaussian pyramid.
        //Func gPyramid[maxJ];
        // Do a lookup into a lut with 256 entires per intensity level
        //Expr level = k * (1.0f / (levels - 1));
        //Expr idx = gray(x, y)*cast<float>(levels-1)*256.0f;
        //idx = clamp(cast<int>(idx), 0, (levels-1)*256);
        //gPyramid[0](x, y, k) = beta*(gray(x, y) - level) + level + remap(idx - 256*k);
        //for (int j = 1; j < J; j++) {
        //    gPyramid[j](x, y, k) = downsample(gPyramid[j-1])(x, y, k);
        //}

        // Make the processed Gaussian Pyramid
        
        Func gPyramid[maxJ];
        // Perform subsampling
        gPyramid[0](x, y, c) = floating(x, y, c);
        for (int j = 1; j < J; j++) {
            gPyramid[j](x, y, c) = downsample(gPyramid[j-1])(x, y, c);
        }
        
        // Perform fake filtering



        // Perform upsampling
        /*
        Func outGPyramid[maxJ];
        //outGPyramid[J-1](x, y, c) = gPyramid[J-1](x, y, c);
        outGPyramid[J-1](x, y, c) = floating(x, y, c);
        for (int j = J-2; j >= 0; j--) {
            outGPyramid[j](x, y, c) = upsample(outGPyramid[j+1])(x, y, c);//+ gPyramid[j](x, y, c);
        }
        */

        /*
        // Get its laplacian pyramid
        Func lPyramid[maxJ];
        lPyramid[J-1](x, y, k) = gPyramid[J-1](x, y, k);
        for (int j = J-2; j >= 0; j--) {
            lPyramid[j](x, y, k) = gPyramid[j](x, y, k) - upsample(gPyramid[j+1])(x, y, k);
        }

        // Make the Gaussian pyramid of the input
        Func inGPyramid[maxJ];
        inGPyramid[0](x, y) = gray(x, y);
        for (int j = 1; j < J; j++) {
            inGPyramid[j](x, y) = downsample(inGPyramid[j-1])(x, y);
        }

        // Make the laplacian pyramid of the output
        Func outLPyramid[maxJ];
        for (int j = 0; j < J; j++) {
            // Split input pyramid value into integer and floating parts
            Expr level = inGPyramid[j](x, y) * cast<float>(levels-1);
            Expr li = clamp(cast<int>(level), 0, levels-2);
            Expr lf = level - cast<float>(li);
            // Linearly interpolate between the nearest processed pyramid levels
            outLPyramid[j](x, y) = (1.0f - lf) * lPyramid[j](x, y, li) + lf * lPyramid[j](x, y, li+1);
        }

        // Make the Gaussian pyramid of the output
        Func outGPyramid[maxJ];
        outGPyramid[J-1](x, y) = outLPyramid[J-1](x, y);
        for (int j = J-2; j >= 0; j--) {
            outGPyramid[j](x, y) = upsample(outGPyramid[j+1])(x, y) + outLPyramid[j](x, y);
        }

        // Reintroduce color (Connelly: use eps to avoid scaling up noise w/ apollo3.png input)
        Func color;
        float eps = 0.01f;
        color(x, y, c) = outGPyramid[0](x, y) * (floating(x, y, c)+eps) / (gray(x, y)+eps);

        */
        Func output("output");
        // Convert back to 16-bit
        output(x, y, c) = cast<uint16_t>(clamp(gPyramid[2](x, y, c), 0.0f, 1.0f) * 65535.0f);

        /* THE SCHEDULE */
        //remap.compute_root();
        if (get_target().has_hls_feature()) {
            Var xi, yi, xo, yo, x_grid, y_grid;

            floating.compute_root();

            //output.tile(x, y, xo, yo, xi, yi, 768, 1280);

            gPyramid[0].linebuffer().compute_at(gPyramid[1], xi);
            gPyramid[1].linebuffer().compute_at(gPyramid[2], xi);
            //gPyramid[2].linebuffer().compute_at(gPyramid[3], xi);
            //gPyramid[1].linebuffer().compute_at(outGPyramid[1], xi);
            gPyramid[2].compute_root();

            //outGPyramid[3].linebuffer().compute_at(outGPyramid[2], xi);
            //outGPyramid[2].linebuffer().compute_at(outGPyramid[1], xi);
                //outGPyramid[1].linebuffer().compute_at(outGPyramid[0], x_grid);

            //gPyramid[0].tile(x, y, xo, yo, xi, yi, 1536, 2560);
            gPyramid[0].unroll(x).unroll(y);
            gPyramid[1].unroll(x).unroll(y);
            //gPyramid[1].tile(x, y, xo, yo, xi, yi, 768, 1280);
            gPyramid[2].tile(x, y, xo, yo, xi, yi, 384, 640);
            //gPyramid[3].tile(x, y, xo, yo, xi, yi, 192, 320);

                //outGPyramid[0].tile(x, y, xo, yo, xi, yi, 3072, 5120);
                //outGPyramid[0].tile(xi, yi, x_grid, y_grid, xi, yi, 2, 2);
            //outGPyramid[0].split(xi, x_grid, xi, 2);
                //outGPyramid[0].unroll(xi).unroll(yi);

            //outGPyramid[0].tile(xi, yi, x_grid, y_grid, xi, yi, 2, 2);
            //outGPyramid[1].tile(x, y, xo, yo, xi, yi, 1536, 2560);
            //outGPyramid[2].tile(x, y, xo, yo, xi, yi, 384, 640);
            //outGPyramid[3].tile(x, y, xo, yo, xi, yi, 192, 320);

                //outGPyramid[0].accelerate({floating}, x_grid, xo);
            gPyramid[2].accelerate({floating}, xi, xo);
                //outGPyramid[0].compute_root();
/*
        if (get_target().has_gpu_feature()) {
            // gpu schedule
            Var xi, yi;
            output.compute_root().gpu_tile(x, y, xi, yi, 16, 8);
            for (int j = 0; j < J; j++) {
                int blockw = 16, blockh = 8;
                if (j > 3) {
                    blockw = 2;
                    blockh = 2;
                }
                if (j > 0) {
                    inGPyramid[j].compute_root().gpu_tile(x, y, xi, yi, blockw, blockh);
                    gPyramid[j].compute_root().reorder(k, x, y).gpu_tile(x, y, xi, yi, blockw, blockh);
                }
                outGPyramid[j].compute_root().gpu_tile(x, y, xi, yi, blockw, blockh);
            }
*/
        } else {
            /*
            // cpu schedule
            Var yo;
            output.reorder(c, x, y).split(y, yo, y, 64).parallel(yo).vectorize(x, 8);
            //gray.compute_root().parallel(y, 32).vectorize(x, 8);
            for (int j = 1; j < 5; j++) {
                //inGPyramid[j]
                //    .compute_root().parallel(y, 32).vectorize(x, 8);
                gPyramid[j]
                    .compute_root().reorder_storage(x, c, y)
                    .reorder(c, y).parallel(y, 8).vectorize(x, 8);
                //outGPyramid[j]
                //    .store_at(output, yo).compute_at(output, y)
                 //   .vectorize(x, 8);
            }
            //outGPyramid[0]
            //    .compute_at(output, y).vectorize(x, 8);
            for (int j = 5; j < J; j++) {
                //inGPyramid[j].compute_root();
                gPyramid[j].compute_root().parallel(c);
                //outGPyramid[j].compute_root();
            }
        */
        }


        return output;
    }
private:
    Var x, y, c, k;

    // Downsample with a 1 3 3 1 filter
    Func downsample(Func f) {
        using Halide::_;
        Func downx("downx"), downy("downy");
        downx(x, y, c) = (f(2*x-1, y, c)     + 3.0f * (f(2*x, y, c)     + f(2*x+1, y, c))     + f(2*x+2, y, c)) / 8.0f;
        downy(x, y, c) = (downx(x, 2*y-1, c) + 3.0f * (downx(x, 2*y, c) + downx(x, 2*y+1, c)) + downx(x, 2*y+2, c)) / 8.0f;

        if (get_target().has_hls_feature()) {
            downx.linebuffer().compute_at(downy, x);
            downx.unroll(y).unroll(x);
        }

        return downy;
    }

    // Upsample using a low pass filter 1 3 3 1 filter
    Func upsample(Func f) {
        using Halide::_;
        Func upx("upx"), upy("upy");
        upx(x, y, _) = (1.0f * f(2*(x % 2) + (x/2) -1, y, _)   + 3.0f * f((x/2), y, _)) / 4.0f;
        upy(x, y, _) = (1.0f * upx(x, 2*(y % 2) + (y/2) -1, _) + 3.0f * upx(x, (y/2), _)) / 4.0f;

        if (get_target().has_hls_feature()) {
            upx.linebuffer().compute_at(upy, x);
            upx.unroll(x);
        }

        return upy;
    }
    // Upsample using bilinear interpolation
    /*
    Func upsample(Func f) {
        using Halide::_;
        Func upx, upy;
        upx(x, y, _) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2, y, _);
        upy(x, y, _) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x, y/2, _);
        return upy;
    }
    */


};

Halide::RegisterGenerator<GaussianPyramid> register_me{"gaussian_pyramid"};

}  // namespace
