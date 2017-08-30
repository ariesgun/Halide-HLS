#include <hls_stream.h>

template < typename T, typename T_out, size_t IMG_EXTENT_0, size_t IMG_EXTENT_1,
		  size_t EXTENT_0, size_t EXTENT_1, size_t EXTENT_2, size_t EXTENT_3,
		  size_t OFFSET_L, size_t OFFSET_R, size_t OFFSET_T, size_t OFFSET_B >
class InputBufferStage {

public:
	// e.g. <3x3> stencil requires 2*WIDTH + 2 extra pixels;
	T buffer[EXTENT_1 - 1][IMG_EXTENT_0 + OFFSET_R + OFFSET_L];
	//static T next_row_buffer[EXTENT_0 - 1];
	T window[EXTENT_1][EXTENT_0];

	//size_t col_row_tracker;
	inline void linebuffer_1d(T temp_buffer[EXTENT_1], size_t row, size_t col, size_t write_idx,
		hls::stream<PackedStencil <T_out, EXTENT_0, EXTENT_1, EXTENT_2, EXTENT_3> > &out_stream) {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable=window complete dim=0
#pragma HLS ARRAY_PARTITION variable=temp_buffer complete dim=0

		// Shift left the window first.
		for (size_t j = 0; j < EXTENT_1; j++) {
			for (size_t i = 0; i < EXTENT_0-1; i++) {
				window[j][i] = window[j][i+1]; // Shift left
			}
		}
		// Get the next data
		for (size_t j = 0; j < EXTENT_1; j++) {
			window[j][EXTENT_0-1] = temp_buffer[j];
		}

		if (col >= EXTENT_0 - 1) {
			PackedStencil <T_out, EXTENT_0, EXTENT_1, EXTENT_2, EXTENT_3 > out_stencil;

			for (size_t idx_3 = 0; idx_3 < EXTENT_3; idx_3++) {
	        	for (size_t idx_2 = 0; idx_2 < EXTENT_2; idx_2++) {
	       			for (size_t idx_1 = 0; idx_1 < EXTENT_1; idx_1++) {
	       				for (size_t idx_0 = 0; idx_0 < EXTENT_0; idx_0++) {
#pragma HLS PIPELINE II=1
							//out_stencil(idx_0, idx_1, idx_2, idx_3) = window[idx_1][idx_0];

							 out_stencil(idx_0, idx_1, idx_2, idx_3) = (window[idx_1][idx_0] >> idx_2 * 8) & 0xFF;
						}
					}
				}
			}
			//Stencil<T, EXTENT_0,EXTENT_1, EXTENT_2, EXTENT_3> temp_stencil = out_stencil;
			// printf(" 		Out stencil %d %d %d %d %d %d %d %d %d\n", temp_stencil(0,0), temp_stencil(1,0), temp_stencil(2,0),
   //  			temp_stencil(0,1), temp_stencil(1,1), temp_stencil(2,1),
   //  			temp_stencil(0,2), temp_stencil(1,2), temp_stencil(2,2));
			out_stream.write(out_stencil);
		}

	}

	inline void fill_buffer_and_send_data (const T in_data, const size_t row, const size_t col,
			hls::stream<PackedStencil<T_out, EXTENT_0, EXTENT_1, EXTENT_2, EXTENT_3> > &out_stream, ap_uint<EXTENT_1 - 1> &write_idx) {
#pragma HLS INLINE
#pragma HLS LOOP_FLATTEN off
#pragma HLS ARRAY_PARTITION variable=buffer complete dim=1

		T temp_buf[EXTENT_1];
#pragma HLS ARRAY_PARTITION variable=temp_buf complete dim=1

		const size_t BUFFER_EXTENT = EXTENT_1 - 1;
		//const size_t EXTRA_BUFFER_EXTENT = EXTENT_0 - 1;
	
		//const size_t BUFFER_WIDTH = IMG_EXTENT_0 + OFFSET_R + OFFSET_L;
		//const size_t TOTAL_BUFFER_SIZE = BUFFER_EXTENT * BUFFER_WIDTH +	EXTRA_BUFFER_EXTENT;

		//if (col_row_tracker >= TOTAL_BUFFER_SIZE && col >= EXTRA_BUFFER_EXTENT) {
		if (row >= IMG_EXTENT_1 + OFFSET_T) {
			// For Offset Bottom
			// Fill and send the stencil
			for (size_t idx_1 = 0; idx_1 < BUFFER_EXTENT; idx_1++) {
				size_t idx_line = idx_1 + write_idx;
				if (idx_line >= BUFFER_EXTENT) {
					idx_line -= BUFFER_EXTENT;
				}
				temp_buf[idx_1] = buffer[idx_line][col];
			}
			size_t idx_line = BUFFER_EXTENT - 1 + write_idx;
			if (idx_line >= BUFFER_EXTENT) {
				idx_line -= BUFFER_EXTENT;
			}
			temp_buf[BUFFER_EXTENT] = buffer[idx_line][col];
			buffer[write_idx][col] = buffer[idx_line][col];

			linebuffer_1d(temp_buf, row, col, write_idx, out_stream);

		} else if (row >= BUFFER_EXTENT) {
			// Fill and send the stencil

			for (size_t idx_1 = 0; idx_1 <= BUFFER_EXTENT; idx_1++) {
				if (idx_1 == BUFFER_EXTENT) {
					temp_buf[BUFFER_EXTENT] = in_data;
					buffer[write_idx][col] = in_data;
				} else {
					size_t idx_line = idx_1 + write_idx;
					if (idx_line >= BUFFER_EXTENT) {
						idx_line -= BUFFER_EXTENT;
					}
					temp_buf[idx_1] = buffer[idx_line][col];
				}
			}
			//printf("	Temp buffer: %d %d %d\n", temp_buf[0], temp_buf[1], temp_buf[2]);
			linebuffer_1d(temp_buf, row, col, write_idx, out_stream);

	       	//printf("Writing data into main buffer %ld %ld %d\n", write_idx, col-BUFFER_EXTENT, next_row_buffer[0]);
// 	        if (col == IMG_EXTENT_0 + OFFSET_L + OFFSET_R - 1) {
// 	       		for (size_t idx = 0; idx < EXTRA_BUFFER_EXTENT; idx++) {
// #pragma HLS UNROLL
// 	       			//printf("Last action %ld %ld", write_idx, col - BUFFER_EXTENT + idx);
// 		       		buffer[write_idx][col - BUFFER_EXTENT + idx] = next_row_buffer[idx];
// 		       	}
// 		       	buffer[write_idx][col] = in_data;
// 	       	} else {
// 	       		buffer[write_idx][col - BUFFER_EXTENT] = next_row_buffer[0];
// 	       	    for (size_t j = 0; j < EXTRA_BUFFER_EXTENT - 1; j++) {
// 	       			next_row_buffer[j] = next_row_buffer[j+1]; // left shift
// 	       	    }
// 	       	    next_row_buffer[EXTRA_BUFFER_EXTENT - 1] = in_data;
// 	       	}

// 	       	out_stream.write(out_stencil);

		//} else if (col_row_tracker < BUFFER_WIDTH * BUFFER_EXTENT) {
		} else if (row < BUFFER_EXTENT) {
			// Filling the main line buffers
			buffer[row][col] = in_data;
			//col_row_tracker++;
		}
//		else {
//			// Fill the extra pixel buffers.
//			for (size_t j = 0; j < BUFFER_EXTENT - 1; j++) {
//	            next_row_buffer[j] = next_row_buffer[j+1]; // left shift
//	        }
//			next_row_buffer[EXTRA_BUFFER_EXTENT - 1] = in_data;
//			//col_row_tracker++;
//		}

	}

	void call(hls::stream<T> &in_stream,
		      hls::stream<PackedStencil<T_out, EXTENT_0, EXTENT_1, EXTENT_2, EXTENT_3> > &out_stream) {

		const size_t BUFFER_EXTENT = EXTENT_1 - 1;

		// First row.
		//	First pixel
		T in_data;
		ap_uint<BUFFER_EXTENT> write_idx = 0;

	FirstRowLoop:
		in_data = in_stream.read();
		for (size_t row_offset = 0; row_offset <= OFFSET_T; row_offset++) {
#pragma HLS PIPELINE II=1
			for (size_t col_offset = 0; col_offset <= OFFSET_L; col_offset++) {
				//printf("Fill %d to %ld %ld\n", in_data, row_offset, col_offset);
				fill_buffer_and_send_data(in_data, row_offset, col_offset, out_stream, write_idx);
			}
		}

		// 	Middle columns
		for (size_t col = 1; col < IMG_EXTENT_0 - 1; col++) {
#pragma HLS DEPENDENCE array inter false
#pragma HLS PIPELINE II=1
			for (size_t row_offset = 0; row_offset <= OFFSET_T; row_offset++) {
				if (row_offset == 0) {
					in_data = in_stream.read();
				}
				//printf("Fill %d to %ld %ld\n", in_data, row_offset, col + OFFSET_L);
				fill_buffer_and_send_data(in_data, row_offset, col + OFFSET_L, out_stream, write_idx);
			}
		}

		//	Last pixel
		in_data = in_stream.read();
		for (size_t row_offset = 0; row_offset <= OFFSET_T; row_offset++) {
#pragma HLS PIPELINE II=1
			for (size_t col_offset = IMG_EXTENT_0 - 1 + OFFSET_L; col_offset <= IMG_EXTENT_0 - 1 + OFFSET_L + OFFSET_R; col_offset++) {
				//printf("Fill %d to %ld %ld\n", in_data, row_offset, col_offset);
				fill_buffer_and_send_data(in_data, row_offset, col_offset, out_stream, write_idx);
			}
		}

		// Middle Rows until the (last row - 1)
 	MiddleInputStageLoop:
 		for (size_t row = 1 + OFFSET_T; row < IMG_EXTENT_1 + OFFSET_T; row++) {

 			// First pixel
 			for (size_t col_offset = 0; col_offset <= OFFSET_L; col_offset++) {
 #pragma HLS PIPELINE II=1
 				if (col_offset == 0) {
 					in_data = in_stream.read();
 					write_idx++;
 					if (write_idx >= BUFFER_EXTENT) {
 						write_idx -= (BUFFER_EXTENT);
 					}
 				}
 				//printf("Fill %d to %ld %ld\n", in_data, row, col_offset);
 				fill_buffer_and_send_data(in_data, row, col_offset, out_stream, write_idx);
 			}

 			// Middle pixel
 			//for (size_t col = 1 + OFFSET_L; col < IMG_EXTENT_0 - 1 + OFFSET_L; col++) {
 			for (size_t col = 0; col < IMG_EXTENT_0 - 1 - OFFSET_R; col++) {
 #pragma HLS PIPELINE II=1
 				in_data = in_stream.read();
 				//printf("Fill %d to %ld %ld\n", in_data, row, col + 1 + OFFSET_L);
 				fill_buffer_and_send_data(in_data, row, col + 1 + OFFSET_L, out_stream, write_idx);
 			}

 			// Last pixel
 			//for (size_t col_offset = IMG_EXTENT_0 - 1 + OFFSET_L; col_offset <= IMG_EXTENT_0 - 1 + OFFSET_R + OFFSET_L; col_offset++) {
 			for (size_t col_offset = 0; col_offset <= OFFSET_R; col_offset++) {
 #pragma HLS PIPELINE II=1
 				if (col_offset == 0) {
 					in_data = in_stream.read();
 				}
 				//printf("Fill %d to %ld %ld\n", in_data, row, col_offset + IMG_EXTENT_0 -1 + OFFSET_L);
 				fill_buffer_and_send_data(in_data, row, col_offset + IMG_EXTENT_0 -1 + OFFSET_L, out_stream, write_idx);
 			}
 		}

 		// Last Row
 	LastRowLoop:
		for(size_t row_offset = 0; row_offset < OFFSET_B; row_offset++) {
			write_idx++;
			if (write_idx >= BUFFER_EXTENT) {
				write_idx -= (BUFFER_EXTENT);
			}
			for(size_t col = 0; col < IMG_EXTENT_0 + OFFSET_R + OFFSET_L; col++) {
#pragma HLS PIPELINE II=1
				//printf("Fill 2 %d to %ld %ld\n", in_data, IMG_EXTENT_1 + row_offset + OFFSET_T, col);
				fill_buffer_and_send_data(0, IMG_EXTENT_1 + row_offset + OFFSET_T, col, out_stream, write_idx);
			}
		}
	}

};


template <typename T, typename T_out, size_t IMG_EXTENT_0, size_t IMG_EXTENT_1, size_t EXTENT_2, size_t EXTENT_3,
			size_t OFFSET_L, size_t OFFSET_R, size_t OFFSET_T, size_t OFFSET_B>
class InputBufferStage<T, T_out, IMG_EXTENT_0, IMG_EXTENT_1, 1, 1, EXTENT_2, EXTENT_3, OFFSET_L, OFFSET_R, OFFSET_T, OFFSET_B> {
public:

	// Linebuffer for Top and bottom Offset
	T buffer[IMG_EXTENT_0 + OFFSET_L + OFFSET_R];

	inline void send_output(T in_data,
							hls::stream< PackedStencil <T_out, 1, 1, EXTENT_2, EXTENT_3> > &out_stream) {

		PackedStencil< T_out, 1, 1, EXTENT_2, EXTENT_3> out_stencil;
		for (size_t idx_3 = 0; idx_3 < EXTENT_3; idx_3++) {
		    for (size_t idx_2 = 0; idx_2 < EXTENT_2; idx_2++) {
#pragma HLS PIPELINE II=1
				out_stencil(0, 0, idx_2, idx_3) = (T_out) (in_data >> (idx_2 * 8)) & 0xFF;
			}
		}
		out_stream.write(out_stencil);
	}

	void call(hls::stream<T> &in_stream,
			  hls::stream<PackedStencil<T_out, 1, 1, EXTENT_2, EXTENT_3> > &out_stream) {

#pragma HLS INLINE OFF
#pragma HLS DATAFLOW

		T in_data;

		for (size_t row_offset = 0; row_offset <= OFFSET_T; row_offset++) {

			for (size_t col = 0; col < IMG_EXTENT_0 + OFFSET_R + OFFSET_L; col++) {
#pragma HLS PIPELINE II=1
				if (OFFSET_T > 0) {
					if (!(col > 0 && col <= OFFSET_L) && !(col >= IMG_EXTENT_0 + OFFSET_L)) {
						if (row_offset == 0) {
							in_data = in_stream.read();
							buffer[col] = in_data;
						} else {
							in_data = buffer[col];
						}
					} else {
						buffer[col] = in_data;
					}
				} else if (OFFSET_T == 0) {
					if (!(col > 0 && col <= OFFSET_L) && !(col >= IMG_EXTENT_0 + OFFSET_L)) {
						in_data = in_stream.read();
					}
				}
				send_output(in_data, out_stream);
			}
	}

		// Middle Rows until the (last row - 1)
 	MiddleInputStageLoop:
 		for (size_t row = 1 + OFFSET_T; row < IMG_EXTENT_1 + OFFSET_T - 1; row++) {

 			for (size_t col = 0; col < IMG_EXTENT_0 + OFFSET_L + OFFSET_R; col++) {
 #pragma HLS PIPELINE II=1
 				if (!(col > 0 && col <= OFFSET_L) && !(col >= IMG_EXTENT_0 + OFFSET_L)) {
 					in_data = in_stream.read();
 				}
 				send_output(in_data, out_stream);
 			}
 		}

 		// Last Row
 	LastRowLoop:
		for(size_t row_offset = 0; row_offset <= OFFSET_B; row_offset++) {

			for (size_t col = 0; col < IMG_EXTENT_0 + OFFSET_R + OFFSET_L; col++) {
#pragma HLS PIPELINE II=1
				if (OFFSET_B > 0) {
					if (!(col > 0 && col <= OFFSET_L) && !(col >= IMG_EXTENT_0 + OFFSET_L)) {
						if (row_offset == 0) {
							in_data = in_stream.read();
							buffer[col] = in_data;
						} else {
							in_data = buffer[col];
						}
					} else {
						buffer[col] = in_data;
					}
				} else if (OFFSET_B == 0) {
					if (!(col > 0 && col <= OFFSET_L) && !(col >= IMG_EXTENT_0 + OFFSET_L)) {
						in_data = in_stream.read();
					}
				}
				send_output(in_data, out_stream);
			}
		}
	}
};

template < typename T, typename T_out, size_t IMG_EXTENT_0, size_t IMG_EXTENT_1,
		  size_t EXTENT_0, size_t EXTENT_1, size_t EXTENT_2, size_t EXTENT_3,
		  size_t OFFSET_L, size_t OFFSET_R, size_t OFFSET_T, size_t OFFSET_B >
void input_buffer(hls::stream<T> &in_stream,
				  hls::stream<PackedStencil<T_out, EXTENT_0, EXTENT_1, EXTENT_2, EXTENT_3> > &out_stream) {

#pragma HLS INLINE OFF
#pragma HLS DATAFLOW

	static InputBufferStage<T, T_out, IMG_EXTENT_0, IMG_EXTENT_1, EXTENT_0, EXTENT_1, EXTENT_2, EXTENT_3,
		OFFSET_L, OFFSET_R, OFFSET_T, OFFSET_B> in;
	in.call(in_stream, out_stream);

}
