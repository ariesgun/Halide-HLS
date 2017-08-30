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
template<int BIT_W, bool SIGNED>
struct arb_int_t {
    static const int bit_width = BIT_W;
    static const bool is_signed = SIGNED;

    typedef bool arb_int;
};

template <typename T, typename = int>
struct isArbInt : std::false_type { };

template <typename T>
struct isArbInt<T, decltype((void) T::arb_int, 0)> : std::true_type { };

template<typename T>
struct has_arb_int {
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
    static yes& test(typename C::arb_int*);

    template <typename>
    static no& test(...);

    static const bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
};

} // namespace Halide'

template<bool SIGNED, int BIT_W, int INT_W >
struct halide_type_of_helper {
    static HALIDE_ALWAYS_INLINE halide_type_t halide_type_of_custom() {
        if (SIGNED) {
            return halide_type_t(halide_type_fixed_point, BIT_W, INT_W, 1);
        } else {
            return halide_type_t(halide_type_ufixed_point, BIT_W, INT_W, 1);
        }
    }
};

template<bool SIGNED, int BIT_W>
struct halide_type_of_helper<SIGNED, BIT_W, BIT_W> {
    static HALIDE_ALWAYS_INLINE halide_type_t halide_type_of_custom() {
        if (SIGNED) {
            return halide_type_t(halide_type_fixed_point, BIT_W, BIT_W, 1);
        } else {
            return halide_type_t(halide_type_ufixed_point, BIT_W, BIT_W, 1);
        }
    }
};

#endif