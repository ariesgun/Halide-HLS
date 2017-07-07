#ifndef CUSTOM_DATA_TYPES_H
#define CUSTOM_DATA_TYPES_H

#include "runtime/HalideRuntime.h"
#include <stdint.h>
#include <string>
#include "Util.h"

namespace Halide {

// Fixed-Point Data Type
struct fixed_point_t {

	enum QuantizationMode { Round = 0, RoundToZero, RoundToMinInf, 
		RoundToInf, RoundToConv, TruncToMinInf, TruncToZero};

	enum OverflowMode { Sat = 0, SatToZero, SatToSym, Wrap, SignWrap};

	int32_t total_width;
	int32_t int_width;
	bool is_signed;
	QuantizationMode quantz_mode;
	OverflowMode overflow_mode;

	/// \name Constructors
    /// @{

    /** Construct a fixed_point from a float using a particular overflow mode and quantization mode.
     *  A warning will be emitted if the result cannot be represented exactly
     *  and error will be raised if the conversion results in overflow.
     *
     *  \param value the input float
     *  \param quantizationMode The rounding mode to use
     *  \param overflowMode The overflow mode to use
     *
     */
    EXPORT explicit fixed_point_t(float value, QuantizationMode roundingMode=TruncToZero, 
    	OverflowMode overflowMode=Wrap);

    /** Construct from a integer value using a particular rounding mode.
     *  A warning will be emitted if the result cannot be represented exactly
     *  and error will be raised if the conversion results in overflow.
     *
     *  \param value the input double
     *  \param roundingMode The rounding mode to use
     *
     */
    EXPORT explicit fixed_point_t(int value, QuantizationMode roundingMode=TruncToZero, 
    	OverflowMode overflowMode=Wrap);

    /** Construct a fixed_point_t with the bits initialised to 0. This represents
     * positive zero.*/
    EXPORT fixed_point_t();

    // Use explicit to avoid accidently raising the precision
    /** Cast to float */
    EXPORT explicit operator float() const;
    /** Cast to double */
    EXPORT explicit operator double() const;
    /** Cast to uint64_t */
    explicit operator uint64_t() const {
        return data;
    }

    /// @}
private:
    uint64_t data;

};

// Arbitrary Data Type
struct arb_int_t {
	int32_t bit_width;
	bool is_signed;

	/// \name Constructors
	/// @{

	/** Construct an arbitrary precision data type from an signed integer value.
	 *
	 *	\param value of the input integer
	 *  
	 */
	EXPORT explicit arb_int_t(int value);

	/** Construct an arbitrary precision data type from an unsigned integer value.
	 *
	 *	\param value of the input integer
	 *  
	 */
	EXPORT explicit arb_int_t(uint value);

};

} // namespace Halide'

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<Halide::arb_int_t>(uint16_t bits) {
    return halide_type_t(halide_type_fixed_point, bits);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<Halide::fixed_point_t>(uint16_t bits, uint16_t int_bits) {
    return halide_type_t(halide_type_fixed_point, bits, int_bits, 1);
}

#endif