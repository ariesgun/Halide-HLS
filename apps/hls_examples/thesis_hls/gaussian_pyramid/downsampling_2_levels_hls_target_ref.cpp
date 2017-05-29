#include "hls_target.h"

#include "Linebuffer.h"
#include "halide_math.h"
void hls_target(
hls::stream<AxiPackedStencil<float, 1, 1, 1> > &arg_0,
hls::stream<AxiPackedStencil<float, 1, 1, 1> > &arg_1)
{
#pragma HLS DATAFLOW
#pragma HLS INLINE region
#pragma HLS INTERFACE s_axilite port=return bundle=config
#pragma HLS INTERFACE axis register port=arg_0
#pragma HLS INTERFACE axis register port=arg_1

 // alias the arguments
 hls::stream<AxiPackedStencil<float, 1, 1, 1> > &_floating_stencil_update_stream = arg_0;
 hls::stream<AxiPackedStencil<float, 1, 1, 1> > &_gPyramid_3__stencil_stream = arg_1;

 hls::stream<PackedStencil<float, 8, 1, 1> > _floating_stencil_stream;
#pragma HLS STREAM variable=_floating_stencil_stream depth=1
#pragma HLS RESOURCE variable=_floating_stencil_stream core=FIFO_SRL

 linebuffer<1550, 2560, 1, 1, 8, 1>(_floating_stencil_update_stream, _floating_stencil_stream);
 (void)0;
 // dispatch_stream(_floating_stencil_stream, 3, 8, 8, 1550, 1, 1, 2560, 1, 1, 1, 1, "gPyramid[0]", 0, 0, 1550, 0, 2560, 0, 1);
 hls::stream<PackedStencil<float, 8, 1, 1> > &_floating_stencil_stream_to_gPyramid_0_ = _floating_stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 8, 1, 1> > _gPyramid_0__stencil_update_stream;
#pragma HLS STREAM variable=_gPyramid_0__stencil_update_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_0__stencil_update_stream core=FIFO_SRL

 // produce gPyramid[0].stencil_update.stream
 for (int _gPyramid_0__y___scan_dim_1 = 0; _gPyramid_0__y___scan_dim_1 < 0 + 2560; _gPyramid_0__y___scan_dim_1++)
 {
  for (int _gPyramid_0__x___scan_dim_0 = 0; _gPyramid_0__x___scan_dim_0 < 0 + 194; _gPyramid_0__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 8, 1, 1> _floating_stencil;
#pragma HLS ARRAY_PARTITION variable=_floating_stencil.value complete dim=0

   _floating_stencil = _floating_stencil_stream_to_gPyramid_0_.read();
   (void)0;
   Stencil<float, 8, 1, 1> _gPyramid_0__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_0__stencil.value complete dim=0

   float _678 = _floating_stencil(0, 0, 0);
   _gPyramid_0__stencil(0, 0, 0) = _678;
   float _679 = _floating_stencil(1, 0, 0);
   _gPyramid_0__stencil(1, 0, 0) = _679;
   float _680 = _floating_stencil(2, 0, 0);
   _gPyramid_0__stencil(2, 0, 0) = _680;
   float _681 = _floating_stencil(3, 0, 0);
   _gPyramid_0__stencil(3, 0, 0) = _681;
   float _682 = _floating_stencil(4, 0, 0);
   _gPyramid_0__stencil(4, 0, 0) = _682;
   float _683 = _floating_stencil(5, 0, 0);
   _gPyramid_0__stencil(5, 0, 0) = _683;
   float _684 = _floating_stencil(6, 0, 0);
   _gPyramid_0__stencil(6, 0, 0) = _684;
   float _685 = _floating_stencil(7, 0, 0);
   _gPyramid_0__stencil(7, 0, 0) = _685;

   if ((_gPyramid_0__y___scan_dim_1 == 0 && (_gPyramid_0__x___scan_dim_0 == 0 || _gPyramid_0__x___scan_dim_0 == 1 || _gPyramid_0__x___scan_dim_0 >= 190)) || 
     ((_gPyramid_0__y___scan_dim_1 == 1 && (_gPyramid_0__x___scan_dim_0 == 0 || _gPyramid_0__x___scan_dim_0 == 1 || _gPyramid_0__x___scan_dim_0 >= 190))) || 
     ((_gPyramid_0__y___scan_dim_1 == 2 && (_gPyramid_0__x___scan_dim_0 == 0 || _gPyramid_0__x___scan_dim_0 == 1 || _gPyramid_0__x___scan_dim_0 >= 190))) || 
     ((_gPyramid_0__y___scan_dim_1 == 3 && (_gPyramid_0__x___scan_dim_0 == 0 || _gPyramid_0__x___scan_dim_0 >= 190)))) {
      printf("Write %d %d: \n", _gPyramid_0__y___scan_dim_1, _gPyramid_0__x___scan_dim_0 );
      printf("%f %f %f %f %f %f %f %f\n", _678, _679, _680, _681, _682, _683, _684, _685);
   }

   _gPyramid_0__stencil_update_stream.write(_gPyramid_0__stencil);
   (void)0;
  } // for _gPyramid_0__x___scan_dim_0
 } // for _gPyramid_0__y___scan_dim_1
 // consume gPyramid[0].stencil_update.stream
 hls::stream<PackedStencil<float, 10, 1, 1> > _gPyramid_0__stencil_stream;
#pragma HLS STREAM variable=_gPyramid_0__stencil_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_0__stencil_stream core=FIFO_SRL

 linebuffer<1550, 2560, 1>(_gPyramid_0__stencil_update_stream, _gPyramid_0__stencil_stream);
 printf("OK\n");
 (void)0;
 // dispatch_stream(_gPyramid_0__stencil_stream, 3, 10, 8, 1550, 1, 1, 2560, 1, 1, 1, 1, "downx", 0, 0, 1550, 0, 2560, 0, 1);
 hls::stream<PackedStencil<float, 10, 1, 1> > &_gPyramid_0__stencil_stream_to_downx = _gPyramid_0__stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 4, 1, 1> > _downx_stencil_stream;
#pragma HLS STREAM variable=_downx_stencil_stream depth=1
#pragma HLS RESOURCE variable=_downx_stencil_stream core=FIFO_SRL

 // produce downx.stencil.stream
 for (int _downx_y___scan_dim_1 = 0; _downx_y___scan_dim_1 < 0 + 2560; _downx_y___scan_dim_1++)
 {
  for (int _downx_x___scan_dim_0 = 0; _downx_x___scan_dim_0 < 0 + 194; _downx_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 10, 1, 1> _gPyramid_0__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_0__stencil.value complete dim=0

   _gPyramid_0__stencil = _gPyramid_0__stencil_stream_to_downx.read();
   (void)0;
   Stencil<float, 4, 1, 1> _downx_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_stencil.value complete dim=0

   float _686 = _gPyramid_0__stencil(0, 0, 0);
   float _687 = _gPyramid_0__stencil(1, 0, 0);
   float _688 = _gPyramid_0__stencil(2, 0, 0);
   float _689 = _687 + _688;
   float _690 = _689 * float_from_bits(1077936128 /* 3 */);
   float _691 = _686 + _690;
   float _692 = _gPyramid_0__stencil(3, 0, 0);
   float _693 = _691 + _692;
   float _694 = _693 * float_from_bits(1040187392 /* 0.125 */);
   _downx_stencil(0, 0, 0) = _694;
   float _695 = _gPyramid_0__stencil(2, 0, 0);
   float _696 = _gPyramid_0__stencil(3, 0, 0);
   float _697 = _gPyramid_0__stencil(4, 0, 0);
   float _698 = _696 + _697;
   float _699 = _698 * float_from_bits(1077936128 /* 3 */);
   float _700 = _695 + _699;
   float _701 = _gPyramid_0__stencil(5, 0, 0);
   float _702 = _700 + _701;
   float _703 = _702 * float_from_bits(1040187392 /* 0.125 */);
   _downx_stencil(1, 0, 0) = _703;
   float _704 = _gPyramid_0__stencil(4, 0, 0);
   float _705 = _gPyramid_0__stencil(5, 0, 0);
   float _706 = _gPyramid_0__stencil(6, 0, 0);
   float _707 = _705 + _706;
   float _708 = _707 * float_from_bits(1077936128 /* 3 */);
   float _709 = _704 + _708;
   float _710 = _gPyramid_0__stencil(7, 0, 0);
   float _711 = _709 + _710;
   float _712 = _711 * float_from_bits(1040187392 /* 0.125 */);
   _downx_stencil(2, 0, 0) = _712;
   float _713 = _gPyramid_0__stencil(6, 0, 0);
   float _714 = _gPyramid_0__stencil(7, 0, 0);
   float _715 = _gPyramid_0__stencil(8, 0, 0);
   float _716 = _714 + _715;
   float _717 = _716 * float_from_bits(1077936128 /* 3 */);
   float _718 = _713 + _717;
   float _719 = _gPyramid_0__stencil(9, 0, 0);
   float _720 = _718 + _719;
   float _721 = _720 * float_from_bits(1040187392 /* 0.125 */);
   _downx_stencil(3, 0, 0) = _721;

   if ((_downx_y___scan_dim_1 == 0 && (_downx_x___scan_dim_0 == 0 || _downx_x___scan_dim_0 == 1 || _downx_x___scan_dim_0 >= 190)) || 
     ((_downx_y___scan_dim_1 == 1 && (_downx_x___scan_dim_0 == 0 || _downx_x___scan_dim_0 == 1 || _downx_x___scan_dim_0 >= 190))) || 
     ((_downx_y___scan_dim_1 == 2 && (_downx_x___scan_dim_0 == 0 || _downx_x___scan_dim_0 == 1 || _downx_x___scan_dim_0 >= 190))) || 
     ((_downx_y___scan_dim_1 == 3 && (_downx_x___scan_dim_0 == 0 || _downx_x___scan_dim_0 >= 190)))) {
      printf("Write %d %d: \n", _downx_y___scan_dim_1, _downx_x___scan_dim_0 );
      printf("%f %f %f %f %f %f %f %f %f %f\n", _686, _687, _688, _696, _697, _701, _706, _710, _715, _719);
   }

   _downx_stencil_stream.write(_downx_stencil);
   (void)0;
  } // for _downx_x___scan_dim_0
 } // for _downx_y___scan_dim_1
 // consume downx.stencil.stream
 // dispatch_stream(_downx_stencil_stream, 3, 4, 4, 774, 1, 1, 2560, 1, 1, 1, 1, "gPyramid[1]", 0, 0, 774, 0, 2560, 0, 1);
 hls::stream<PackedStencil<float, 4, 1, 1> > &_downx_stencil_stream_to_gPyramid_1_ = _downx_stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 4, 1, 1> > _gPyramid_1__stencil_update_stream;
#pragma HLS STREAM variable=_gPyramid_1__stencil_update_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_1__stencil_update_stream core=FIFO_SRL

printf("-------\n");

 // produce gPyramid[1].stencil_update.stream
 for (int _gPyramid_1__y___scan_dim_1 = 0; _gPyramid_1__y___scan_dim_1 < 0 + 2560; _gPyramid_1__y___scan_dim_1++)
 {
  for (int _gPyramid_1__x___scan_dim_0 = 0; _gPyramid_1__x___scan_dim_0 < 0 + 194; _gPyramid_1__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 4, 1, 1> _downx_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_stencil.value complete dim=0

   _downx_stencil = _downx_stencil_stream_to_gPyramid_1_.read();
   (void)0;
   Stencil<float, 4, 1, 1> _gPyramid_1__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_1__stencil.value complete dim=0

   float _722 = _downx_stencil(0, 0, 0);
   _gPyramid_1__stencil(0, 0, 0) = _722;
   float _723 = _downx_stencil(1, 0, 0);
   _gPyramid_1__stencil(1, 0, 0) = _723;
   float _724 = _downx_stencil(2, 0, 0);
   _gPyramid_1__stencil(2, 0, 0) = _724;
   float _725 = _downx_stencil(3, 0, 0);
   _gPyramid_1__stencil(3, 0, 0) = _725;
   _gPyramid_1__stencil_update_stream.write(_gPyramid_1__stencil);

   if ((_gPyramid_1__y___scan_dim_1 == 0 && (_gPyramid_1__x___scan_dim_0 == 0 || _gPyramid_1__x___scan_dim_0 == 1 || _gPyramid_1__x___scan_dim_0 >= 190)) || 
     ((_gPyramid_1__y___scan_dim_1 == 1 && (_gPyramid_1__x___scan_dim_0 == 0 || _gPyramid_1__x___scan_dim_0 == 1 || _gPyramid_1__x___scan_dim_0 >= 190))) || 
     ((_gPyramid_1__y___scan_dim_1 == 2 && (_gPyramid_1__x___scan_dim_0 == 0 || _gPyramid_1__x___scan_dim_0 == 1 || _gPyramid_1__x___scan_dim_0 >= 190))) || 
     ((_gPyramid_1__y___scan_dim_1 == 3 && (_gPyramid_1__x___scan_dim_0 == 0 || _gPyramid_1__x___scan_dim_0 >= 190)))) {
      printf("Write %d %d: \n", _gPyramid_1__y___scan_dim_1, _gPyramid_1__x___scan_dim_0 );
      printf("%f %f %f %f \n", _722, _723, _724, _725);
   }
   (void)0;
  } // for _gPyramid_1__x___scan_dim_0
 } // for _gPyramid_1__y___scan_dim_1
 // consume gPyramid[1].stencil_update.stream
 hls::stream<PackedStencil<float, 6, 1, 1> > _gPyramid_1__stencil_stream;
#pragma HLS STREAM variable=_gPyramid_1__stencil_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_1__stencil_stream core=FIFO_SRL

 printf("OK2\n");
 linebuffer<774, 2560, 1>(_gPyramid_1__stencil_update_stream, _gPyramid_1__stencil_stream);
 (void)0;
 // dispatch_stream(_gPyramid_1__stencil_stream, 3, 6, 4, 774, 1, 1, 2560, 1, 1, 1, 1, "downx$1", 0, 0, 774, 0, 2560, 0, 1);
 hls::stream<PackedStencil<float, 6, 1, 1> > &_gPyramid_1__stencil_stream_to_downx_1 = _gPyramid_1__stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 2, 1, 1> > _downx_1_stencil_stream;
#pragma HLS STREAM variable=_downx_1_stencil_stream depth=1
#pragma HLS RESOURCE variable=_downx_1_stencil_stream core=FIFO_SRL

 // produce downx$1.stencil.stream
 for (int _downx_1_y___scan_dim_1 = 0; _downx_1_y___scan_dim_1 < 0 + 2560; _downx_1_y___scan_dim_1++)
 {
  for (int _downx_1_x___scan_dim_0 = 0; _downx_1_x___scan_dim_0 < 0 + 193; _downx_1_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 6, 1, 1> _gPyramid_1__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_1__stencil.value complete dim=0


   _gPyramid_1__stencil = _gPyramid_1__stencil_stream_to_downx_1.read();
   (void)0;
   Stencil<float, 2, 1, 1> _downx_1_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_1_stencil.value complete dim=0

   float _726 = _gPyramid_1__stencil(0, 0, 0);
   float _727 = _gPyramid_1__stencil(1, 0, 0);
   float _728 = _gPyramid_1__stencil(2, 0, 0);
   float _729 = _727 + _728;
   float _730 = _729 * float_from_bits(1077936128 /* 3 */);
   float _731 = _726 + _730;
   float _732 = _gPyramid_1__stencil(3, 0, 0);
   float _733 = _731 + _732;
   float _734 = _733 * float_from_bits(1040187392 /* 0.125 */);
   _downx_1_stencil(0, 0, 0) = _734;
   float _735 = _gPyramid_1__stencil(2, 0, 0);
   float _736 = _gPyramid_1__stencil(3, 0, 0);
   float _737 = _gPyramid_1__stencil(4, 0, 0);
   float _738 = _736 + _737;
   float _739 = _738 * float_from_bits(1077936128 /* 3 */);
   float _740 = _735 + _739;
   float _741 = _gPyramid_1__stencil(5, 0, 0);
   float _742 = _740 + _741;
   float _743 = _742 * float_from_bits(1040187392 /* 0.125 */);
   _downx_1_stencil(1, 0, 0) = _743;
   _downx_1_stencil_stream.write(_downx_1_stencil);
      if ((_downx_1_y___scan_dim_1 == 0 && (_downx_1_x___scan_dim_0 == 0 || _downx_1_x___scan_dim_0 == 1 || _downx_1_x___scan_dim_0 >= 190)) || 
     ((_downx_1_y___scan_dim_1 == 1 && (_downx_1_x___scan_dim_0 == 0 || _downx_1_x___scan_dim_0 == 1 || _downx_1_x___scan_dim_0 >= 190))) || 
     ((_downx_1_y___scan_dim_1 == 2 && (_downx_1_x___scan_dim_0 == 0 || _downx_1_x___scan_dim_0 == 1 || _downx_1_x___scan_dim_0 >= 190))) || 
     ((_downx_1_y___scan_dim_1 == 3 && (_downx_1_x___scan_dim_0 == 0 || _downx_1_x___scan_dim_0 >= 190)))) {
      printf("Write %d %d: \n", _downx_1_y___scan_dim_1, _downx_1_x___scan_dim_0 );
      printf("%f %f %f %f %f %f \n", _726, _727, _728, _732, _737, _741);
   }
   (void)0;
  } // for _downx_1_x___scan_dim_0
 } // for _downx_1_y___scan_dim_1
 // consume downx$1.stencil.stream
 // dispatch_stream(_downx_1_stencil_stream, 3, 2, 2, 386, 1, 1, 2560, 1, 1, 1, 1, "gPyramid[2]", 0, 0, 386, 0, 2560, 0, 1);
 hls::stream<PackedStencil<float, 2, 1, 1> > &_downx_1_stencil_stream_to_gPyramid_2_ = _downx_1_stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 2, 1, 1> > _gPyramid_2__stencil_update_stream;
#pragma HLS STREAM variable=_gPyramid_2__stencil_update_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_2__stencil_update_stream core=FIFO_SRL

 // produce gPyramid[2].stencil_update.stream
 for (int _gPyramid_2__y___scan_dim_1 = 0; _gPyramid_2__y___scan_dim_1 < 0 + 2560; _gPyramid_2__y___scan_dim_1++)
 {
  for (int _gPyramid_2__x___scan_dim_0 = 0; _gPyramid_2__x___scan_dim_0 < 0 + 193; _gPyramid_2__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 2, 1, 1> _downx_1_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_1_stencil.value complete dim=0

   _downx_1_stencil = _downx_1_stencil_stream_to_gPyramid_2_.read();
   (void)0;
   Stencil<float, 2, 1, 1> _gPyramid_2__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_2__stencil.value complete dim=0

   float _744 = _downx_1_stencil(0, 0, 0);
   _gPyramid_2__stencil(0, 0, 0) = _744;
   float _745 = _downx_1_stencil(1, 0, 0);
   _gPyramid_2__stencil(1, 0, 0) = _745;
   _gPyramid_2__stencil_update_stream.write(_gPyramid_2__stencil);
   if ((_gPyramid_2__y___scan_dim_1 == 0 && (_gPyramid_2__x___scan_dim_0 == 0 || _gPyramid_2__x___scan_dim_0 == 1 || _gPyramid_2__x___scan_dim_0 >= 190)) || 
     ((_gPyramid_2__y___scan_dim_1 == 1 && (_gPyramid_2__x___scan_dim_0 == 0 || _gPyramid_2__x___scan_dim_0 == 1 || _gPyramid_2__x___scan_dim_0 >= 190))) || 
     ((_gPyramid_2__y___scan_dim_1 == 2 && (_gPyramid_2__x___scan_dim_0 == 0 || _gPyramid_2__x___scan_dim_0 == 1 || _gPyramid_2__x___scan_dim_0 >= 190))) || 
     ((_gPyramid_2__y___scan_dim_1 == 3 && (_gPyramid_2__x___scan_dim_0 == 0 || _gPyramid_2__x___scan_dim_0 >= 190)))) {
      printf("Write %d %d: \n", _gPyramid_2__y___scan_dim_1, _gPyramid_2__x___scan_dim_0 );
      printf("%f %f \n", _744, _745);
   }
   (void)0;
  } // for _gPyramid_2__x___scan_dim_0
 } // for _gPyramid_2__y___scan_dim_1
 // consume gPyramid[2].stencil_update.stream
 hls::stream<PackedStencil<float, 4, 1, 1> > _gPyramid_2__stencil_stream;
#pragma HLS STREAM variable=_gPyramid_2__stencil_stream depth=1
#pragma HLS RESOURCE variable=_gPyramid_2__stencil_stream core=FIFO_SRL

 printf("ok-----\n");

 linebuffer<386, 2560, 1>(_gPyramid_2__stencil_update_stream, _gPyramid_2__stencil_stream);
 (void)0;
 // dispatch_stream(_gPyramid_2__stencil_stream, 3, 4, 2, 386, 1, 1, 2560, 1, 1, 1, 1, "downx$2", 0, 0, 386, 0, 2560, 0, 1);
 hls::stream<PackedStencil<float, 4, 1, 1> > &_gPyramid_2__stencil_stream_to_downx_2 = _gPyramid_2__stencil_stream;
 (void)0;
 hls::stream<PackedStencil<float, 1, 1, 1> > _downx_2_stencil_stream;
#pragma HLS STREAM variable=_downx_2_stencil_stream depth=1
#pragma HLS RESOURCE variable=_downx_2_stencil_stream core=FIFO_SRL

 // produce downx$2.stencil.stream
 for (int _downx_2_y___scan_dim_1 = 0; _downx_2_y___scan_dim_1 < 0 + 2560; _downx_2_y___scan_dim_1++)
 {
  for (int _downx_2_x___scan_dim_0 = 0; _downx_2_x___scan_dim_0 < 0 + 192; _downx_2_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 4, 1, 1> _gPyramid_2__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_2__stencil.value complete dim=0

   _gPyramid_2__stencil = _gPyramid_2__stencil_stream_to_downx_2.read();
   (void)0;
   Stencil<float, 1, 1, 1> _downx_2_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_2_stencil.value complete dim=0

   float _746 = _gPyramid_2__stencil(0, 0, 0);
   float _747 = _gPyramid_2__stencil(1, 0, 0);
   float _748 = _gPyramid_2__stencil(2, 0, 0);
   float _749 = _747 + _748;
   float _750 = _749 * float_from_bits(1077936128 /* 3 */);
   float _751 = _746 + _750;
   float _752 = _gPyramid_2__stencil(3, 0, 0);
   float _753 = _751 + _752;
   float _754 = _753 * float_from_bits(1040187392 /* 0.125 */);
   _downx_2_stencil(0, 0, 0) = _754;
   _downx_2_stencil_stream.write(_downx_2_stencil);
   if ((_downx_2_y___scan_dim_1 == 0 && (_downx_2_x___scan_dim_0 == 0 || _downx_2_x___scan_dim_0 == 1 || _downx_2_x___scan_dim_0 >= 190)) || 
     ((_downx_2_y___scan_dim_1 == 1 && (_downx_2_x___scan_dim_0 == 0 || _downx_2_x___scan_dim_0 == 1 || _downx_2_x___scan_dim_0 >= 190))) || 
     ((_downx_2_y___scan_dim_1 == 2 && (_downx_2_x___scan_dim_0 == 0 || _downx_2_x___scan_dim_0 == 1 || _downx_2_x___scan_dim_0 >= 190))) || 
     ((_downx_2_y___scan_dim_1 == 3 && (_downx_2_x___scan_dim_0 == 0 || _downx_2_x___scan_dim_0 >= 190)))) {
      printf("Write %d %d: \n", _downx_2_y___scan_dim_1, _downx_2_x___scan_dim_0 );
      printf("%f %f %f %f\n", _746, _747, _748, _752);
   }
   (void)0;
  } // for _downx_2_x___scan_dim_0
 } // for _downx_2_y___scan_dim_1
 // consume downx$2.stencil.stream
 // dispatch_stream(_downx_2_stencil_stream, 3, 1, 1, 192, 1, 1, 2560, 1, 1, 1, 1, "gPyramid[3]", 0, 0, 192, 0, 2560, 0, 1);
 hls::stream<PackedStencil<float, 1, 1, 1> > &_downx_2_stencil_stream_to_gPyramid_3_ = _downx_2_stencil_stream;
 (void)0;
 // produce gPyramid[3].stencil.stream
 for (int _gPyramid_3__y___scan_dim_1 = 0; _gPyramid_3__y___scan_dim_1 < 0 + 2560; _gPyramid_3__y___scan_dim_1++)
 {
  for (int _gPyramid_3__x___scan_dim_0 = 0; _gPyramid_3__x___scan_dim_0 < 0 + 192; _gPyramid_3__x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<float, 1, 1, 1> _downx_2_stencil;
#pragma HLS ARRAY_PARTITION variable=_downx_2_stencil.value complete dim=0

   _downx_2_stencil = _downx_2_stencil_stream_to_gPyramid_3_.read();
   (void)0;
   Stencil<float, 1, 1, 1> _gPyramid_3__stencil;
#pragma HLS ARRAY_PARTITION variable=_gPyramid_3__stencil.value complete dim=0

   float _755 = _downx_2_stencil(0, 0, 0);
   _gPyramid_3__stencil(0, 0, 0) = _755;
   AxiPackedStencil<float, 1, 1, 1> _gPyramid_3__stencil_packed = _gPyramid_3__stencil;
   if (_gPyramid_3__x___scan_dim_0 == 191 && _gPyramid_3__y___scan_dim_1 == 2559) {
    _gPyramid_3__stencil_packed.last = 1;
   } else {
    _gPyramid_3__stencil_packed.last = 0;
   }
   _gPyramid_3__stencil_stream.write(_gPyramid_3__stencil_packed);
   (void)0;
  } // for _gPyramid_3__x___scan_dim_0
 } // for _gPyramid_3__y___scan_dim_1
} // kernel hls_target_hls_target


