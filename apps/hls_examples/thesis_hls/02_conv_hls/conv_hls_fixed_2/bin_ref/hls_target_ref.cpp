#include "hls_target.h"

#include "Linebuffer_default.h"
#include "halide_math.h"
void hls_target(
hls::stream<AxiPackedStencil<ap_fixed<20,1> , 1, 1> > &arg_0,
//hls::stream<AxiPackedStencilFixed<20, 1, 1, 1> > &arg_0,
hls::stream<AxiPackedStencil<ap_fixed<29,1> , 1, 1> > &arg_1)
{
#pragma HLS DATAFLOW
#pragma HLS INLINE region
#pragma HLS INTERFACE s_axilite port=return bundle=config
#pragma HLS INTERFACE axis register port=arg_0
#pragma HLS INTERFACE axis register port=arg_1

 // alias the arguments
 hls::stream<AxiPackedStencil<ap_fixed<20,1> , 1, 1> > &_clamped_stencil_update_stream = arg_0;
 hls::stream<AxiPackedStencil<ap_fixed<29,1> , 1, 1> > &_hw_output_stencil_stream = arg_1;

 hls::stream<PackedStencil<ap_fixed<20,1> , 5, 5> > _clamped_stencil_stream;
#pragma HLS STREAM variable=_clamped_stencil_stream depth=1
#pragma HLS RESOURCE variable=_clamped_stencil_stream core=FIFO_SRL

 linebuffer<644, 484>(_clamped_stencil_update_stream, _clamped_stencil_stream);
 (void)0;
 // dispatch_stream(_clamped_stencil_stream, 2, 5, 1, 644, 5, 1, 484, 1, "conv1", 0, 0, 644, 0, 484);
 hls::stream<PackedStencil<ap_fixed<20,1> , 5, 5> > &_clamped_stencil_stream_to_conv1 = _clamped_stencil_stream;
 (void)0;
 hls::stream<PackedStencil<ap_fixed<29,1> , 1, 1> > _conv1_stencil_stream;
#pragma HLS STREAM variable=_conv1_stencil_stream depth=1
#pragma HLS RESOURCE variable=_conv1_stencil_stream core=FIFO_SRL

 // produce conv1.stencil.stream
 for (int _conv1_y___scan_dim_1 = 0; _conv1_y___scan_dim_1 < 0 + 480; _conv1_y___scan_dim_1++)
 {
  for (int _conv1_x___scan_dim_0 = 0; _conv1_x___scan_dim_0 < 0 + 640; _conv1_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   StencilFixed_<ap_fixed<20,1>, 20, 1, 5, 5> _clamped_stencil;
#pragma HLS ARRAY_PARTITION variable=_clamped_stencil.value complete dim=0

   _clamped_stencil = _clamped_stencil_stream_to_conv1.read();
   (void)0;
   //Stencil<ap_fixed<29,1> , 1, 1> _conv1_stencil;
   StencilFixed_<ap_fixed<29,1>, 29, 1, 1, 1> _conv1_stencil;
#pragma HLS ARRAY_PARTITION variable=_conv1_stencil.value complete dim=0

   ap_fixed<29,1> _466 = (ap_fixed<29,1> )(0);
   _conv1_stencil(0, 0) = _466;
   ap_fixed<29,1> _467 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _468 = _clamped_stencil(0, 0);
   //printf("Value %f\n", (float) _468);
   ap_fixed<21,1> _469 = (ap_fixed<21,1> )(float_from_bits(1011081199 /* 0.0119552 */));
   ap_fixed<41,2> _470 = _468 * _469;
   ap_fixed<42,3> _471 = _467 + _470;
   _conv1_stencil(0, 0) = _471;
   ap_fixed<29,1> _472 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _473 = _clamped_stencil(1, 0);
   ap_fixed<21,1> _474 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _475 = _473 * _474;
   ap_fixed<42,3> _476 = _472 + _475;
   _conv1_stencil(0, 0) = _476;
   ap_fixed<29,1> _477 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _478 = _clamped_stencil(2, 0);
   ap_fixed<21,1> _479 = (ap_fixed<21,1> )(float_from_bits(1022245297 /* 0.0290802 */));
   ap_fixed<41,2> _480 = _478 * _479;
   ap_fixed<42,3> _481 = _477 + _480;
   _conv1_stencil(0, 0) = _481;
   ap_fixed<29,1> _482 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _483 = _clamped_stencil(3, 0);
   ap_fixed<21,1> _484 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _485 = _483 * _484;
   ap_fixed<42,3> _486 = _482 + _485;
   _conv1_stencil(0, 0) = _486;
   ap_fixed<29,1> _487 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _488 = _clamped_stencil(4, 0);
   ap_fixed<21,1> _489 = (ap_fixed<21,1> )(float_from_bits(1011081199 /* 0.0119552 */));
   ap_fixed<41,2> _490 = _488 * _489;
   ap_fixed<42,3> _491 = _487 + _490;
   _conv1_stencil(0, 0) = _491;
   ap_fixed<29,1> _492 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _493 = _clamped_stencil(0, 1);
   ap_fixed<21,1> _494 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _495 = _493 * _494;
   ap_fixed<42,3> _496 = _492 + _495;
   _conv1_stencil(0, 0) = _496;
   ap_fixed<29,1> _497 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _498 = _clamped_stencil(1, 1);
   ap_fixed<21,1> _499 = (ap_fixed<21,1> )(float_from_bits(1027196253 /* 0.0453542 */));
   ap_fixed<41,2> _500 = _498 * _499;
   ap_fixed<42,3> _501 = _497 + _500;
   _conv1_stencil(0, 0) = _501;
   ap_fixed<29,1> _502 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _503 = _clamped_stencil(2, 1);
   ap_fixed<21,1> _504 = (ap_fixed<21,1> )(float_from_bits(1030225909 /* 0.0566406 */));
   ap_fixed<41,2> _505 = _503 * _504;
   ap_fixed<42,3> _506 = _502 + _505;
   _conv1_stencil(0, 0) = _506;
   ap_fixed<29,1> _507 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _508 = _clamped_stencil(3, 1);
   ap_fixed<21,1> _509 = (ap_fixed<21,1> )(float_from_bits(1027196253 /* 0.0453542 */));
   ap_fixed<41,2> _510 = _508 * _509;
   ap_fixed<42,3> _511 = _507 + _510;
   _conv1_stencil(0, 0) = _511;
   ap_fixed<29,1> _512 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _513 = _clamped_stencil(4, 1);
   ap_fixed<21,1> _514 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _515 = _513 * _514;
   ap_fixed<42,3> _516 = _512 + _515;
   _conv1_stencil(0, 0) = _516;
   ap_fixed<29,1> _517 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _518 = _clamped_stencil(0, 2);
   ap_fixed<21,1> _519 = (ap_fixed<21,1> )(float_from_bits(1022245297 /* 0.0290802 */));
   ap_fixed<41,2> _520 = _518 * _519;
   ap_fixed<42,3> _521 = _517 + _520;
   _conv1_stencil(0, 0) = _521;
   ap_fixed<29,1> _522 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _523 = _clamped_stencil(1, 2);
   ap_fixed<21,1> _524 = (ap_fixed<21,1> )(float_from_bits(1030225909 /* 0.0566406 */));
   ap_fixed<41,2> _525 = _523 * _524;
   ap_fixed<42,3> _526 = _522 + _525;
   _conv1_stencil(0, 0) = _526;
   ap_fixed<29,1> _527 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _528 = _clamped_stencil(2, 2);
   ap_fixed<21,1> _529 = (ap_fixed<21,1> )(float_from_bits(1032904138 /* 0.0707355 */));
   ap_fixed<41,2> _530 = _528 * _529;
   ap_fixed<42,3> _531 = _527 + _530;
   _conv1_stencil(0, 0) = _531;
   ap_fixed<29,1> _532 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _533 = _clamped_stencil(3, 2);
   ap_fixed<21,1> _534 = (ap_fixed<21,1> )(float_from_bits(1030225909 /* 0.0566406 */));
   ap_fixed<41,2> _535 = _533 * _534;
   ap_fixed<42,3> _536 = _532 + _535;
   _conv1_stencil(0, 0) = _536;
   ap_fixed<29,1> _537 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _538 = _clamped_stencil(4, 2);
   ap_fixed<21,1> _539 = (ap_fixed<21,1> )(float_from_bits(1022245297 /* 0.0290802 */));
   ap_fixed<41,2> _540 = _538 * _539;
   ap_fixed<42,3> _541 = _537 + _540;
   _conv1_stencil(0, 0) = _541;
   ap_fixed<29,1> _542 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _543 = _clamped_stencil(0, 3);
   ap_fixed<21,1> _544 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _545 = _543 * _544;
   ap_fixed<42,3> _546 = _542 + _545;
   _conv1_stencil(0, 0) = _546;
   ap_fixed<29,1> _547 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _548 = _clamped_stencil(1, 3);
   ap_fixed<21,1> _549 = (ap_fixed<21,1> )(float_from_bits(1027196253 /* 0.0453542 */));
   ap_fixed<41,2> _550 = _548 * _549;
   ap_fixed<42,3> _551 = _547 + _550;
   _conv1_stencil(0, 0) = _551;
   ap_fixed<29,1> _552 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _553 = _clamped_stencil(2, 3);
   ap_fixed<21,1> _554 = (ap_fixed<21,1> )(float_from_bits(1030225909 /* 0.0566406 */));
   ap_fixed<41,2> _555 = _553 * _554;
   ap_fixed<42,3> _556 = _552 + _555;
   _conv1_stencil(0, 0) = _556;
   ap_fixed<29,1> _557 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _558 = _clamped_stencil(3, 3);
   ap_fixed<21,1> _559 = (ap_fixed<21,1> )(float_from_bits(1027196253 /* 0.0453542 */));
   ap_fixed<41,2> _560 = _558 * _559;
   ap_fixed<42,3> _561 = _557 + _560;
   _conv1_stencil(0, 0) = _561;
   ap_fixed<29,1> _562 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _563 = _clamped_stencil(4, 3);
   ap_fixed<21,1> _564 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _565 = _563 * _564;
   ap_fixed<42,3> _566 = _562 + _565;
   _conv1_stencil(0, 0) = _566;
   ap_fixed<29,1> _567 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _568 = _clamped_stencil(0, 4);
   ap_fixed<21,1> _569 = (ap_fixed<21,1> )(float_from_bits(1011081199 /* 0.0119552 */));
   ap_fixed<41,2> _570 = _568 * _569;
   ap_fixed<42,3> _571 = _567 + _570;
   _conv1_stencil(0, 0) = _571;
   ap_fixed<29,1> _572 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _573 = _clamped_stencil(1, 4);
   ap_fixed<21,1> _574 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _575 = _573 * _574;
   ap_fixed<42,3> _576 = _572 + _575;
   _conv1_stencil(0, 0) = _576;
   ap_fixed<29,1> _577 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _578 = _clamped_stencil(2, 4);
   ap_fixed<21,1> _579 = (ap_fixed<21,1> )(float_from_bits(1022245297 /* 0.0290802 */));
   ap_fixed<41,2> _580 = _578 * _579;
   ap_fixed<42,3> _581 = _577 + _580;
   _conv1_stencil(0, 0) = _581;
   ap_fixed<29,1> _582 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _583 = _clamped_stencil(3, 4);
   ap_fixed<21,1> _584 = (ap_fixed<21,1> )(float_from_bits(1019134342 /* 0.0232856 */));
   ap_fixed<41,2> _585 = _583 * _584;
   ap_fixed<42,3> _586 = _582 + _585;
   _conv1_stencil(0, 0) = _586;
   ap_fixed<29,1> _587 = _conv1_stencil(0, 0);
   ap_fixed<20,1> _588 = _clamped_stencil(4, 4);
   ap_fixed<21,1> _589 = (ap_fixed<21,1> )(float_from_bits(1011081199 /* 0.0119552 */));
   ap_fixed<41,2> _590 = _588 * _589;
   ap_fixed<42,3> _591 = _587 + _590;
   _conv1_stencil(0, 0) = _591;
   _conv1_stencil_stream.write(_conv1_stencil);
   (void)0;
  } // for _conv1_x___scan_dim_0
 } // for _conv1_y___scan_dim_1
 // consume conv1.stencil.stream
 // dispatch_stream(_conv1_stencil_stream, 2, 1, 1, 640, 1, 1, 480, 1, "hw_output", 0, 0, 640, 0, 480);
 hls::stream<PackedStencil<ap_fixed<29,1> , 1, 1> > &_conv1_stencil_stream_to_hw_output = _conv1_stencil_stream;
 (void)0;
 // produce hw_output.stencil.stream
 for (int _hw_output_y___scan_dim_1 = 0; _hw_output_y___scan_dim_1 < 0 + 480; _hw_output_y___scan_dim_1++)
 {
  for (int _hw_output_x___scan_dim_0 = 0; _hw_output_x___scan_dim_0 < 0 + 640; _hw_output_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   //Stencil<ap_fixed<29,1> , 1, 1> _conv1_stencil
   StencilFixed_<ap_fixed<29,1>, 29, 1, 1, 1> _conv1_stencil;
#pragma HLS ARRAY_PARTITION variable=_conv1_stencil.value complete dim=0

   _conv1_stencil = _conv1_stencil_stream_to_hw_output.read();
   (void)0;
   //Stencil<ap_fixed<29,1> , 1, 1> _hw_output_stencil;
   StencilFixed_<ap_fixed<29,1>, 29, 1, 1, 1> _hw_output_stencil;
#pragma HLS ARRAY_PARTITION variable=_hw_output_stencil.value complete dim=0

   ap_fixed<29,1> _592 = _conv1_stencil(0, 0);
   _hw_output_stencil(0, 0) = _592;
   AxiPackedStencil<ap_fixed<29,1> , 1, 1> _hw_output_stencil_packed = _hw_output_stencil;
   if (_hw_output_x___scan_dim_0 == 639 && _hw_output_y___scan_dim_1 == 479) {
    _hw_output_stencil_packed.last = 1;
   } else {
    _hw_output_stencil_packed.last = 0;
   }
   _hw_output_stencil_stream.write(_hw_output_stencil_packed);
   (void)0;
  } // for _hw_output_x___scan_dim_0
 } // for _hw_output_y___scan_dim_1
} // kernel hls_target_hls_target


