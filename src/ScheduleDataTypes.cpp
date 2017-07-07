#include "IRMutator.h"
#include "ScheduleDataTypes.h"
#include "Debug.h"

namespace Halide {
namespace Internal {

void cast_data_type(Function &f, Type type, const std::vector<std::pair<Expr&, Type>> exprs, const std::vector<std::pair<Expr&, Type>> args_exprs) {

	f.set_is_type_casted(true);
	f.set_casted_output_type(type);

	for (size_t i = 0; i < exprs.size(); i++) {
		std::pair<Expr&, Type> item = exprs[i];
		debug(3) << "Address check " << &(exprs[i].first) << " with " << &(item.first) << "\n";
		debug(3) << "# Inserted " << item.first << " with type " << item.second << "\n";
		f.insert_expr_cast_ref(item);
	}

	for (size_t i = 0; i < args_exprs.size(); i++) {
		std::pair<Expr&, Type> item = args_exprs[i];
		debug(3) << "Address check " << &(args_exprs[i].first) << " with " << &(item.first) << "\n";
		debug(3) << "# Inserted " << item.first << " with type " << item.second << "\n";
		f.insert_args_expr_cast_ref(item);
	}

	debug(3) << "Casted " << f.name() << " with type " << f.get_casted_output_type() << "\n";
	// Cast the Expressions

}

} // namespace Internal
} // namespace Halide
