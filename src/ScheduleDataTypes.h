#ifndef SCHEDULE_DATA_TYPES_H
#define SCHEDULE_DATA_TYPES_H

/** \file
 *
 * Cast the type of Functions into a defined data type.
 */
#include "IR.h"

namespace Halide {
namespace Internal {

/** Mark the functions transitively called by Function f as
 * HW kernels. We traverse all the function directly called
 * by Function f, or indirectly in those function, recursively.
 * The recursion only terminates at functions with names included
 * by set inputs.
 */
/** Traverse all the function directly called by Function f.
 *
 */
void cast_data_type(Function &f, Type type, const std::vector<std::pair<Expr&, Type>> exprs, const std::vector<std::pair<Expr&, Type>> args_exprs);


} // namespace Internal
} // namespace Halide


#endif