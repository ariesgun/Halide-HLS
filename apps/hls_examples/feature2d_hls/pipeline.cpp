#include "Halide.h"
#include <stdio.h>

using namespace Halide;

Var x("x"), y("y"), c("c");
Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), tile_index("ti");
Var xio("xio"), yio("yio"), xv("xv"), yp("yp");

int blockSize = 3;
int Ksize = 3;


float k = 0.04;
//float threshold = 20000000;
float threshold = 1000;
 

class MyPipeline {

public:
    ImageParam input;
    ImageParam intg;
    //Param<float> k,  threshold;
    std::vector<Argument> args;
    
    Func original;
    Func padded;
    Func grad_x, grad_y;
    Func grad_xx, grad_yy, grad_xy;
    Func grad_gx, grad_gy, grad_gxy;
    Func cim;
    Func corners;
    Func kpt;
    Func brief;
    Func output, hw_output;
    RDom box, maxWin;

  MyPipeline()
    : input(UInt(8), 2), intg(Int(32), 2), padded("padded"),
      grad_x("grad_x"), grad_y("grad_y"),
      grad_xx("grad_xx"), grad_yy("grad_yy"), grad_xy("grad_xy"),
      grad_gx("grad_gx"), grad_gy("grad_gy"), grad_gxy("grad_gxy"),
      corners("corners"),
      output("output"), hw_output("hw_output"),
      box(-blockSize/2, blockSize, -blockSize/2, blockSize),
      maxWin(-1, 3, -1, 3) {

    //original(x, y) = (input(x, y));
    padded = BoundaryConditions::repeat_edge(input);

    // sobel filter
    Func padded16;
    padded16(x,y) = cast<int16_t>(padded(x,y));
    grad_x(x, y) = cast<int16_t>(-padded16(x-1,y-1) + padded16(x+1,y-1)
                                 -2*padded16(x-1,y) + 2*padded16(x+1,y)
                                 -padded16(x-1,y+1) + padded16(x+1,y+1));
    grad_y(x, y) = cast<int16_t>(padded16(x-1,y+1) - padded16(x-1,y-1) +
                                 2*padded16(x,y+1) - 2*padded16(x,y-1) +
                                 padded16(x+1,y+1) - padded16(x+1,y-1));

    grad_xx(x, y) = cast<int32_t>(grad_x(x,y)) * cast<int32_t>(grad_x(x,y));
    grad_yy(x, y) = cast<int32_t>(grad_y(x,y)) * cast<int32_t>(grad_y(x,y));
    grad_xy(x, y) = cast<int32_t>(grad_x(x,y)) * cast<int32_t>(grad_y(x,y));

    // box filter (i.e. windowed sum)
    grad_gx(x, y) += grad_xx(x+box.x, y+box.y);
    grad_gy(x, y) += grad_yy(x+box.x, y+box.y);
    grad_gxy(x, y) += grad_xy(x+box.x, y+box.y);

    // calculate Cim
    int scale = (1 << (Ksize-1)) * blockSize;
    Expr lgx = cast<float>(grad_gx(x, y) / scale / scale);
    Expr lgy = cast<float>(grad_gy(x, y) / scale / scale);
    Expr lgxy = cast<float>(grad_gxy(x, y) / scale / scale);
    Expr det = lgx*lgy - lgxy*lgxy;
    Expr trace = lgx + lgy;
    cim(x, y) = det - k*trace*trace;

    // Perform non-maximal suppression
    Expr is_max = cim(x, y) > cim(x-1, y-1) && cim(x, y) > cim(x, y-1) &&
        cim(x, y) > cim(x+1, y-1) && cim(x, y) > cim(x-1, y) &&
        cim(x, y) > cim(x+1, y) && cim(x, y) > cim(x-1, y+1) &&
        cim(x, y) > cim(x, y+1) && cim(x, y) > cim(x+1, y+1);
    corners(x, y) = select( is_max , cim(x, y), 0);
    //corners(x, y) = cim(x,y);

    output.define_extern("brief", {corners, intg, input.width(), input.height(), threshold}, UInt(8), 2);
    /*brief.define_extern("brief", {corners, intg, input.width(), input.height(), threshold}, UInt(8), 2);

    hw_output(x, y) = brief(x, y);

    output(x, y) = hw_output(x, y);
     */
    // Arguments
    //args = {input, k, threshold};
    args = {input, intg};
  }

  void compile_cpu() {
    std::cout << "\ncompiling cpu code..." << std::endl;

    corners.tile(x, y, xo, yo, xi, yi, 240, 320);
    grad_x.compute_at(corners, xo).vectorize(x, 8);
    grad_y.compute_at(corners, xo).vectorize(x, 8);
    grad_xx.compute_at(corners, xo).vectorize(x, 4);
    grad_yy.compute_at(corners, xo).vectorize(x, 4);
    grad_xy.compute_at(corners, xo).vectorize(x, 4);
    grad_gx.compute_at(corners, xo).vectorize(x, 4);
    grad_gy.compute_at(corners, xo).vectorize(x, 4);
    grad_gxy.compute_at(corners, xo).vectorize(x, 4);
    cim.compute_at(corners, xo).vectorize(x, 4);

    grad_gx.update(0).unroll(box.x).unroll(box.y);
    grad_gy.update(0).unroll(box.x).unroll(box.y);
    grad_gxy.update(0).unroll(box.x).unroll(box.y);

    corners.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4); 
    //output.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4); 
    
    corners.compute_root();
    //brief.compute_root();

    Target target = get_target_from_environment();
    target.set_feature(Target::Profile);
    //output.print_loop_nest();
    //corners.compile_to_lowered_stmt("pipeline_corners.ir.html", args, HTML);
    output.compile_to_lowered_stmt("pipeline_native.ir.html", args, HTML, target);
   
  
    output.compile_to_header("pipeline_native.h", args, "pipeline_native", target);
    output.compile_to_object("pipeline_native.o", args, "pipeline_native", target);
    corners.compile_to_header("pipeline_corners.h", args, "pipeline_corners", target);
    corners.compile_to_object("pipeline_corners.o", args, "pipeline_corners", target);
  }

  void compile_gpu() {
    std::cout << "\ncompiling gpu code..." << std::endl;

    grad_gx.update(0).unroll(box.x).unroll(box.y);
    grad_gy.update(0).unroll(box.x).unroll(box.y);
    grad_gxy.update(0).unroll(box.x).unroll(box.y);
    //output.compute_root(); //.gpu_tile(x, y, 32, 16);
    cim.compute_root().gpu_tile(x, y, 32, 16);
    corners.compute_root();
    //output.compute_root();
    //conv1.compute_at(output, Var::gpu_blocks()).gpu_threads(x, y, c);

    //output.print_loop_nest();

    Target target = get_target_from_environment();
    target.set_feature(Target::CUDA);
    output.compile_to_lowered_stmt("pipeline_cuda.ir.html", args, HTML, target);
    output.compile_to_header("pipeline_cuda.h", args, "pipeline_cuda", target);
    output.compile_to_object("pipeline_cuda.o", args, "pipeline_cuda", target);
  }

  void compile_hls() {
    std::cout << "\ncompiling HLS code..." << std::endl;

    hw_output.compute_root();
    //hw_output.tile(x, y, xo, yo, xi, yi, 1920, 1080);
    hw_output.tile(x, y, xo, yo, xi, yi, 512, 512);
    //hw_output.unroll(xi, 2);
    std::vector<Func> hw_bounds = hw_output.accelerate({padded}, xi, xo);
    hw_bounds[0].unroll(x);

    grad_x.linebuffer().unroll(x);
    grad_y.linebuffer().unroll(x);
    grad_xx.linebuffer().unroll(x);
    grad_yy.linebuffer().unroll(x);
    grad_xy.linebuffer().unroll(x);
    grad_gx.linebuffer().unroll(x);
    grad_gx.update(0).unroll(x);
    grad_gy.linebuffer().unroll(x);
    grad_gy.update(0).unroll(x);
    grad_gxy.linebuffer().unroll(x);
    grad_gxy.update(0).unroll(x);
    cim.linebuffer().unroll(x);

    grad_gx.update(0).unroll(box.x).unroll(box.y);
    grad_gy.update(0).unroll(box.x).unroll(box.y);
    grad_gxy.update(0).unroll(box.x).unroll(box.y);

    //output.print_loop_nest();
    output.compile_to_lowered_stmt("pipeline_hls.ir.html", args, HTML);
    output.compile_to_hls("pipeline_hls.cpp", args, "pipeline_hls");
    output.compile_to_header("pipeline_hls.h", args, "pipeline_hls");
  }
};

int main(int argc, char **argv) {
  MyPipeline p1;
  p1.compile_cpu();

  /*MyPipeline p2;
  p2.compile_hls();
   */
  MyPipeline p3;
  p3.compile_gpu();
  return 0;
}
