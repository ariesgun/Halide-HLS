#include "hls_target.h"

#include "Linebuffer.h"
#include "halide_math.h"
void hls_target(
//hls::stream<AxiPackedStencil<uint16_t, 1, 1, 3> > &arg_0,
//hls::stream<AxiPackedStencil<uint8_t, 1, 1, 1> > &arg_1)
hls::stream<uint16_t> &arg_0,
hls::stream<uint8_t>  &arg_1)
{
#pragma HLS DATAFLOW
#pragma HLS INLINE region
#pragma HLS INTERFACE s_axilite port=return bundle=config
#pragma HLS INTERFACE axis register port=arg_0
#pragma HLS INTERFACE axis register port=arg_1

 // alias the arguments
 hls::stream<uint16_t> &_blur_y_stencil_stream = arg_0;
 hls::stream<PackedStencil<uint8_t, 3, 1, 1> > _repeat_edge_stencil_stream;

 Stencil<uint8_t, 1, 1, 1> _repeat_edge_stencil_update;
 hls::stream<PackedStencil<uint8_t, 1, 1, 1> > _repeat_edge_stencil_update_stream;
#pragma HLS STREAM variable=_repeat_edge_stencil_stream depth=1
#pragma HLS RESOURCE variable=_repeat_edge_stencil_stream core=FIFO_SRL

 for (int _dim_2 = 0; _dim_2 <= 2; _dim_2 += 3)
  for (int _dim_1 = 0; _dim_1 <= 1079; _dim_1 += 1)
  for (int _dim_0 = 0; _dim_0 <= 1919; _dim_0 += 1)
  {
 #pragma HLS PIPELINE
   //PackedStencil<uint8_t, 1, 1, 1> _tmp_stencil = _repeat_edge_stencil_stream.read();
   _repeat_edge_stencil_update(0, 0, 0) = arg_1.read();
   //printf("Read at %d %d: %d\n", _dim_0, _dim_1, _repeat_edge_stencil_update(0, 0, 0));
   //PackedStencil<uint8_t, 1, 1, 1> _tmp_stencil =  _repeat_edge_stencil_update(0, 0, 0);
   if (_dim_0 >= 0 && _dim_0 <= 1919 && _dim_1 >= 0 && _dim_1 <= 1079 && _dim_2 >= 0 && _dim_2 <= 2)
   {
    _repeat_edge_stencil_update_stream.write(_repeat_edge_stencil_update);
   }
  }

 // Linebuffer in the first step to convert uint8 to Stencil or Packed Stencil.
 linebuffer<1920, 1080, 1, 1, 0, 1>(_repeat_edge_stencil_update_stream, _repeat_edge_stencil_stream);
 (void)0;
 // dispatch_stream(_repeat_edge_stencil_stream, 3, 3, 1, 1922, 1, 1, 1080, 3, 3, 2, 1, "blur_y", 0, 0, 1922, 0, 1080, 0, 2);
 hls::stream<PackedStencil<uint8_t, 3, 1, 1> > &_repeat_edge_stencil_stream_to_blur_y = _repeat_edge_stencil_stream;
 (void)0;
 // produce blur_y.stencil.stream
 for (int _blur_y_y___scan_dim_1 = 0; _blur_y_y___scan_dim_1 < 0 + 1080; _blur_y_y___scan_dim_1++)
 {
  for (int _blur_y_x___scan_dim_0 = 0; _blur_y_x___scan_dim_0 < 0 + 1920; _blur_y_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<uint8_t, 3, 1, 1> _repeat_edge_stencil;
#pragma HLS ARRAY_PARTITION variable=_repeat_edge_stencil.value complete dim=0

   _repeat_edge_stencil = _repeat_edge_stencil_stream_to_blur_y.read();
   (void)0;
   Stencil<uint16_t, 1, 1, 1> _blur_y_stencil;
#pragma HLS ARRAY_PARTITION variable=_blur_y_stencil.value complete dim=0

   uint8_t _627 = _repeat_edge_stencil(0, 0, 0);
   uint16_t _628 = (uint16_t)(_627);
   uint8_t _629 = _repeat_edge_stencil(1, 0, 0);
   uint16_t _630 = (uint16_t)(_629);
   uint16_t _631 = _628 + _630;
   uint8_t _632 = _repeat_edge_stencil(2, 0, 0);
   uint16_t _633 = (uint16_t)(_632);
   uint16_t _634 = _631 + _633;
   uint16_t _635 = (uint16_t)(3);
   uint16_t _636 = _634 / _635;
   _blur_y_stencil(0, 0, 0) = _636;
   // uint8_t _637 = _repeat_edge_stencil(0, 0, 1);
   // uint16_t _638 = (uint16_t)(_637);
   // uint8_t _639 = _repeat_edge_stencil(1, 0, 1);
   // uint16_t _640 = (uint16_t)(_639);
   // uint16_t _641 = _638 + _640;
   // uint8_t _642 = _repeat_edge_stencil(2, 0, 1);
   // uint16_t _643 = (uint16_t)(_642);
   // uint16_t _644 = _641 + _643;
   // uint16_t _645 = (uint16_t)(3);
   // uint16_t _646 = _644 / _645;
   // _blur_y_stencil(0, 0, 1) = _646;
   // uint8_t _647 = _repeat_edge_stencil(0, 0, 2);
   // uint16_t _648 = (uint16_t)(_647);
   // uint8_t _649 = _repeat_edge_stencil(1, 0, 2);
   // uint16_t _650 = (uint16_t)(_649);
   // uint16_t _651 = _648 + _650;
   // uint8_t _652 = _repeat_edge_stencil(2, 0, 2);
   // uint16_t _653 = (uint16_t)(_652);
   // uint16_t _654 = _651 + _653;
   // uint16_t _655 = (uint16_t)(3);
   // uint16_t _656 = _654 / _655;
   // _blur_y_stencil(0, 0, 2) = _656;
   
   uint16_t _blur_y_output = (uint16_t) _blur_y_stencil(0, 0, 0);
    //printf("Write at %d %d: %d\n", _blur_y_x___scan_dim_0, _blur_y_y___scan_dim_1, _blur_y_output);
    _blur_y_stencil_stream.write(_blur_y_output);

    //if (_blur_y_x___scan_dim_0 == 1919 && _blur_y_y___scan_dim_1 == 1079) {
    // _blur_y_stencil_packed.last = 1;
    //} else {
    // _blur_y_stencil_packed.last = 0;
    //}
    //_blur_y_stencil_stream.write(_blur_y_stencil_packed);
    (void)0;
  } // for _blur_y_x___scan_dim_0
 } // for _blur_y_y___scan_dim_1
} // kernel hls_target_hls_target


