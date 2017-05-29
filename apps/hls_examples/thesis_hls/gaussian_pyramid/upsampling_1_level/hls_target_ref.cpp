#include "hls_target.h"

#include "Linebuffer.h"
#include "halide_math.h"
void hls_target(
hls::stream<AxiPackedStencil<float, 1, 1, 1> > &arg_0,
hls::stream<AxiPackedStencil<float, 2, 2, 1> > &arg_1)
{
#pragma HLS DATAFLOW
#pragma HLS INLINE region
#pragma HLS INTERFACE s_axilite port=return bundle=config
#pragma HLS INTERFACE axis register port=arg_0
#pragma HLS INTERFACE axis register port=arg_1

 // alias the arguments
 hls::stream<AxiPackedStencil<float, 1, 1, 1> > &_floating_stencil_update_stream = arg_0;
 hls::stream<AxiPackedStencil<float, 2, 2, 1> > &_outGPyramid_0__stencil_stream = arg_1;

 hls::stream<PackedStencil<float, 2, 2, 1> > _floating_stencil_stream;
#pragma HLS STREAM variable=_floating_stencil_stream depth=1
#pragma HLS RESOURCE variable=_floating_stencil_stream core=FIFO_SRL

 linebuffer<1543, 2567, 1, 1, 2, 2>(_floating_stencil_update_stream, _floating_stencil_stream);
 (void)0;
 // dispatch_stream(_floating_stencil_stream, 3, 2, 2, 1543, 2, 2, 2567, 1, 1, 1, 1, "gPyramid[0]", 0, 0, 1543, 0, 2567, 0, 1);
 hls::stream<PackedStencil<float, 2, 2, 1> > &_floating_stencil_stream_to_gPyramid_0_ = _floating_stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 2, 2, 1> > _gPyramid_0__stencil_update_stream;
#pragma HLS STREAM variable=_gPyramid_0__stencil_update_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_0__stencil_update_stream core=FIFO_SRL

 // produce gPyramid[0].stencil_update.stream
 for (int _gPyramid_0__y___scan_dim_1 = 0; _gPyramid_0__y___scan_dim_1 < 0 + 1284; _gPyramid_0__y___scan_dim_1++)
 {
  for (int _gPyramid_0__x___scan_dim_0 = 0; _gPyramid_0__x___scan_dim_0 < 0 + 772; _gPyramid_0__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 2, 2, 1> _floating_stencil;
#pragma HLS ARRAY_PARTITION variable=_floating_stencil.value complete dim=0

   _floating_stencil = _floating_stencil_stream_to_gPyramid_0_.read();
   (void)0;
   Stencil<float, 2, 2, 1> _gPyramid_0__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_0__stencil.value complete dim=0

   float _802 = _floating_stencil(0, 0, 0);
   _gPyramid_0__stencil(0, 0, 0) = _802;
   float _803 = _floating_stencil(1, 0, 0);
   _gPyramid_0__stencil(1, 0, 0) = _803;
   float _804 = _floating_stencil(0, 1, 0);
   _gPyramid_0__stencil(0, 1, 0) = _804;
   float _805 = _floating_stencil(1, 1, 0);
   _gPyramid_0__stencil(1, 1, 0) = _805;
   _gPyramid_0__stencil_update_stream.write(_gPyramid_0__stencil);
   (void)0;
  } // for _gPyramid_0__x___scan_dim_0
 } // for _gPyramid_0__y___scan_dim_1
 // consume gPyramid[0].stencil_update.stream
 hls::stream<PackedStencil<float, 4, 2, 1> > _gPyramid_0__stencil_stream;
#pragma HLS STREAM variable=_gPyramid_0__stencil_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_0__stencil_stream core=FIFO_SRL

 printf("OK1\n");
 linebuffer<1543, 2567, 1>(_gPyramid_0__stencil_update_stream, _gPyramid_0__stencil_stream);
 (void)0;
 // dispatch_stream(_gPyramid_0__stencil_stream, 3, 4, 2, 1543, 2, 2, 2567, 1, 1, 1, 1, "downx", 0, 0, 1543, 0, 2567, 0, 1);
 hls::stream<PackedStencil<float, 4, 2, 1> > &_gPyramid_0__stencil_stream_to_downx = _gPyramid_0__stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 1, 2, 1> > _downx_stencil_update_stream;
#pragma HLS STREAM variable=_downx_stencil_update_stream depth=1
#pragma HLS RESOURCE variable=_downx_stencil_update_stream core=FIFO_SRL

 // produce downx.stencil_update.stream
 for (int _downx_y___scan_dim_1 = 0; _downx_y___scan_dim_1 < 0 + 1284; _downx_y___scan_dim_1++)
 {
  for (int _downx_x___scan_dim_0 = 0; _downx_x___scan_dim_0 < 0 + 771; _downx_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 4, 2, 1> _gPyramid_0__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_0__stencil.value complete dim=0

   _gPyramid_0__stencil = _gPyramid_0__stencil_stream_to_downx.read();
   (void)0;
   Stencil<float, 1, 2, 1> _downx_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_stencil.value complete dim=0

   float _806 = _gPyramid_0__stencil(0, 0, 0);
   float _807 = _gPyramid_0__stencil(1, 0, 0);
   float _808 = _gPyramid_0__stencil(2, 0, 0);
   float _809 = _807 + _808;
   float _810 = _809 * float_from_bits(1077936128 /* 3 */);
   float _811 = _806 + _810;
   float _812 = _gPyramid_0__stencil(3, 0, 0);
   float _813 = _811 + _812;
   float _814 = _813 * float_from_bits(1040187392 /* 0.125 */);
   _downx_stencil(0, 0, 0) = _814;
   float _815 = _gPyramid_0__stencil(0, 1, 0);
   float _816 = _gPyramid_0__stencil(1, 1, 0);
   float _817 = _gPyramid_0__stencil(2, 1, 0);
   float _818 = _816 + _817;
   float _819 = _818 * float_from_bits(1077936128 /* 3 */);
   float _820 = _815 + _819;
   float _821 = _gPyramid_0__stencil(3, 1, 0);
   float _822 = _820 + _821;
   float _823 = _822 * float_from_bits(1040187392 /* 0.125 */);
   _downx_stencil(0, 1, 0) = _823;
   _downx_stencil_update_stream.write(_downx_stencil);
   (void)0;
  } // for _downx_x___scan_dim_0
 } // for _downx_y___scan_dim_1
 // consume downx.stencil_update.stream
 hls::stream<PackedStencil<float, 1, 4, 1> > _downx_stencil_stream;
#pragma HLS STREAM variable=_downx_stencil_stream depth=1
#pragma HLS RESOURCE variable=_downx_stencil_stream core=FIFO_SRL

 printf("OK2\n");
 linebuffer<771, 2567, 1>(_downx_stencil_update_stream, _downx_stencil_stream);
 (void)0;
 // dispatch_stream(_downx_stencil_stream, 3, 1, 1, 770, 4, 2, 2567, 1, 1, 1, 1, "gPyramid[1]", 0, 0, 770, 0, 2567, 0, 1);
 hls::stream<PackedStencil<float, 1, 4, 1> > &_downx_stencil_stream_to_gPyramid_1_ = _downx_stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 1, 1, 1> > _gPyramid_1__stencil_stream;
#pragma HLS STREAM variable=_gPyramid_1__stencil_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_1__stencil_stream core=FIFO_SRL

 // produce gPyramid[1].stencil.stream
 for (int _gPyramid_1__y___scan_dim_1 = 0; _gPyramid_1__y___scan_dim_1 < 0 + 1283; _gPyramid_1__y___scan_dim_1++)
 {
  for (int _gPyramid_1__x___scan_dim_0 = 0; _gPyramid_1__x___scan_dim_0 < 0 + 771; _gPyramid_1__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 1, 4, 1> _downx_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_stencil.value complete dim=0

   _downx_stencil = _downx_stencil_stream_to_gPyramid_1_.read();
   (void)0;
   Stencil<float, 1, 1, 1> _gPyramid_1__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_1__stencil.value complete dim=0

   float _824 = _downx_stencil(0, 0, 0);
   float _825 = _downx_stencil(0, 1, 0);
   float _826 = _downx_stencil(0, 2, 0);
   float _827 = _825 + _826;
   float _828 = _827 * float_from_bits(1077936128 /* 3 */);
   float _829 = _824 + _828;
   float _830 = _downx_stencil(0, 3, 0);
   float _831 = _829 + _830;
   float _832 = _831 * float_from_bits(1040187392 /* 0.125 */);
   _gPyramid_1__stencil(0, 0, 0) = _832;
   _gPyramid_1__stencil_stream.write(_gPyramid_1__stencil);
   (void)0;
  } // for _gPyramid_1__x___scan_dim_0
 } // for _gPyramid_1__y___scan_dim_1
 // consume gPyramid[1].stencil.stream
 // dispatch_stream(_gPyramid_1__stencil_stream, 3, 1, 1, 770, 1, 1, 1282, 1, 1, 1, 1, "outGPyramid[1]", 0, 0, 770, 0, 1282, 0, 1);
 hls::stream<PackedStencil<float, 1, 1, 1> > &_gPyramid_1__stencil_stream_to_outGPyramid_1_ = _gPyramid_1__stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 1, 1, 1> > _outGPyramid_1__stencil_update_stream;
#pragma HLS STREAM variable=_outGPyramid_1__stencil_update_stream depth=1
#pragma HLS RESOURCE variable=_outGPyramid_1__stencil_update_stream core=FIFO_SRL

 // produce outGPyramid[1].stencil_update.stream
 for (int _outGPyramid_1__y___scan_dim_1 = 0; _outGPyramid_1__y___scan_dim_1 < 0 + 1282; _outGPyramid_1__y___scan_dim_1++)
 {
  for (int _outGPyramid_1__x___scan_dim_0 = 0; _outGPyramid_1__x___scan_dim_0 < 0 + 770; _outGPyramid_1__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 1, 1, 1> _gPyramid_1__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_1__stencil.value complete dim=0

   _gPyramid_1__stencil = _gPyramid_1__stencil_stream_to_outGPyramid_1_.read();
   (void)0;
   Stencil<float, 1, 1, 1> _outGPyramid_1__stencil;
#pragma HLS ARRAY_PARTITION variable=_outGPyramid_1__stencil.value complete dim=0

   float _833 = _gPyramid_1__stencil(0, 0, 0);
   _outGPyramid_1__stencil(0, 0, 0) = _833;
   _outGPyramid_1__stencil_update_stream.write(_outGPyramid_1__stencil);

   if ((_outGPyramid_1__y___scan_dim_1 == 0 && (_outGPyramid_1__x___scan_dim_0 == 0 || _outGPyramid_1__x___scan_dim_0 == 1 || _outGPyramid_1__x___scan_dim_0 >= 768)) || 
     ((_outGPyramid_1__y___scan_dim_1 == 1 && (_outGPyramid_1__x___scan_dim_0 == 0 || _outGPyramid_1__x___scan_dim_0 == 1 || _outGPyramid_1__x___scan_dim_0 >= 768))) || 
     ((_outGPyramid_1__y___scan_dim_1 == 2 && (_outGPyramid_1__x___scan_dim_0 == 0 || _outGPyramid_1__x___scan_dim_0 == 1 || _outGPyramid_1__x___scan_dim_0 >= 768))) || 
     ((_outGPyramid_1__y___scan_dim_1 == 0 && (_outGPyramid_1__x___scan_dim_0 == 0 || _outGPyramid_1__x___scan_dim_0 >= 768)))) {
      printf("Write %d %d: \n", _outGPyramid_1__y___scan_dim_1, _outGPyramid_1__x___scan_dim_0 );
      printf("%f ", _833);
   }

   (void)0;
  } // for _outGPyramid_1__x___scan_dim_0
  printf("\n");
 } // for _outGPyramid_1__y___scan_dim_1
 // consume outGPyramid[1].stencil_update.stream
 hls::stream<PackedStencil<float, 3, 1, 1> > _outGPyramid_1__stencil_stream;
#pragma HLS STREAM variable=_outGPyramid_1__stencil_stream depth=1
#pragma HLS RESOURCE variable=_outGPyramid_1__stencil_stream core=FIFO_SRL

 printf("OK4\n");
 linebuffer<770, 1282, 1>(_outGPyramid_1__stencil_update_stream, _outGPyramid_1__stencil_stream);
 (void)0;
 // dispatch_stream(_outGPyramid_1__stencil_stream, 3, 3, 1, 770, 1, 1, 1282, 1, 1, 1, 1, "upx", 0, 0, 770, 0, 1282, 0, 1);
 hls::stream<PackedStencil<float, 3, 1, 1> > &_outGPyramid_1__stencil_stream_to_upx = _outGPyramid_1__stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 2, 1, 1> > _upx_stencil_update_stream;
#pragma HLS STREAM variable=_upx_stencil_update_stream depth=1
#pragma HLS RESOURCE variable=_upx_stencil_update_stream core=FIFO_SRL

 // produce upx.stencil_update.stream
 for (int _upx_y___scan_dim_1 = 0; _upx_y___scan_dim_1 < 0 + 1282; _upx_y___scan_dim_1++)
 {
  for (int _upx_x___scan_dim_0 = 0; _upx_x___scan_dim_0 < 0 + 768; _upx_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 3, 1, 1> _outGPyramid_1__stencil;
#pragma HLS ARRAY_PARTITION variable=_outGPyramid_1__stencil.value complete dim=0

   _outGPyramid_1__stencil = _outGPyramid_1__stencil_stream_to_upx.read();
   (void)0;
   Stencil<float, 2, 1, 1> _upx_stencil;
#pragma HLS ARRAY_PARTITION variable=_upx_stencil.value complete dim=0

   float _834 = _outGPyramid_1__stencil(0, 0, 0);
   float _835 = _outGPyramid_1__stencil(1, 0, 0);
   float _836 = _835 * float_from_bits(1077936128 /* 3 */);
   float _837 = _834 + _836;
   float _838 = _837 * float_from_bits(1048576000 /* 0.25 */);
   _upx_stencil(0, 0, 0) = _838;
   float _839 = _outGPyramid_1__stencil(2, 0, 0);
   float _840 = _outGPyramid_1__stencil(1, 0, 0);
   float _841 = _840 * float_from_bits(1077936128 /* 3 */);
   float _842 = _839 + _841;
   float _843 = _842 * float_from_bits(1048576000 /* 0.25 */);
   _upx_stencil(1, 0, 0) = _843;
   _upx_stencil_update_stream.write(_upx_stencil);
   (void)0;
  } // for _upx_x___scan_dim_0
 } // for _upx_y___scan_dim_1
 // consume upx.stencil_update.stream
 hls::stream<PackedStencil<float, 2, 3, 1> > _upx_stencil_stream;
#pragma HLS STREAM variable=_upx_stencil_stream depth=1
#pragma HLS RESOURCE variable=_upx_stencil_stream core=FIFO_SRL

 printf("OK3\n");
 linebuffer<1536, 1282, 1>(_upx_stencil_update_stream, _upx_stencil_stream);
 (void)0;
 // dispatch_stream(_upx_stencil_stream, 3, 2, 2, 1536, 3, 1, 1282, 1, 1, 1, 1, "outGPyramid[0]", 0, 0, 1536, 0, 1282, 0, 1);
 hls::stream<PackedStencil<float, 2, 3, 1> > &_upx_stencil_stream_to_outGPyramid_0_ = _upx_stencil_stream;
 (void)0;
 // produce outGPyramid[0].stencil.stream
 for (int _outGPyramid_0__y___scan_dim_1 = 0; _outGPyramid_0__y___scan_dim_1 < 0 + 1280; _outGPyramid_0__y___scan_dim_1++)
 {
  for (int _outGPyramid_0__x___scan_dim_0 = 0; _outGPyramid_0__x___scan_dim_0 < 0 + 768; _outGPyramid_0__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 2, 3, 1> _upx_stencil;
#pragma HLS ARRAY_PARTITION variable=_upx_stencil.value complete dim=0

   _upx_stencil = _upx_stencil_stream_to_outGPyramid_0_.read();
   (void)0;
   Stencil<float, 2, 2, 1> _outGPyramid_0__stencil;
#pragma HLS ARRAY_PARTITION variable=_outGPyramid_0__stencil.value complete dim=0

   float _844 = _upx_stencil(0, 0, 0);
   float _845 = _upx_stencil(0, 1, 0);
   float _846 = _845 * float_from_bits(1077936128 /* 3 */);
   float _847 = _844 + _846;
   float _848 = _847 * float_from_bits(1048576000 /* 0.25 */);
   _outGPyramid_0__stencil(0, 0, 0) = _848;
   float _849 = _upx_stencil(1, 0, 0);
   float _850 = _upx_stencil(1, 1, 0);
   float _851 = _850 * float_from_bits(1077936128 /* 3 */);
   float _852 = _849 + _851;
   float _853 = _852 * float_from_bits(1048576000 /* 0.25 */);
   _outGPyramid_0__stencil(1, 0, 0) = _853;
   float _854 = _upx_stencil(0, 2, 0);
   float _855 = _upx_stencil(0, 1, 0);
   float _856 = _855 * float_from_bits(1077936128 /* 3 */);
   float _857 = _854 + _856;
   float _858 = _857 * float_from_bits(1048576000 /* 0.25 */);
   _outGPyramid_0__stencil(0, 1, 0) = _858;
   float _859 = _upx_stencil(1, 2, 0);
   float _860 = _upx_stencil(1, 1, 0);
   float _861 = _860 * float_from_bits(1077936128 /* 3 */);
   float _862 = _859 + _861;
   float _863 = _862 * float_from_bits(1048576000 /* 0.25 */);
   _outGPyramid_0__stencil(1, 1, 0) = _863;
   AxiPackedStencil<float, 2, 2, 1> _outGPyramid_0__stencil_packed = _outGPyramid_0__stencil;
   if (_outGPyramid_0__x___scan_dim_0 == 767 && _outGPyramid_0__y___scan_dim_1 == 1279) {
    _outGPyramid_0__stencil_packed.last = 1;
   } else {
    _outGPyramid_0__stencil_packed.last = 0;
   }
   _outGPyramid_0__stencil_stream.write(_outGPyramid_0__stencil_packed);
   (void)0;
  } // for _outGPyramid_0__x___scan_dim_0
 } // for _outGPyramid_0__y___scan_dim_1
} // kernel hls_target_hls_target


