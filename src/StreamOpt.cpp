#include "StreamOpt.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "IREquality.h"
#include "Scope.h"
#include "Debug.h"
#include "Substitute.h"
#include "IRPrinter.h"
#include "Simplify.h"
#include "Bounds.h"

#include <iostream>
#include <algorithm>
using std::ostream;

namespace Halide {
namespace Internal {

using std::string;
using std::map;
using std::set;
using std::pair;
using std::vector;
using std::list;

namespace {

class UpStepDifference : public IRMutator {
    bool is_downsampling;
    using IRMutator::visit;

    void visit(const Add *op) {

        const Mul *tmp_a = op->a.as<Mul>();

        if (is_downsampling) {
            const IntImm *tmp_int = op->b.as<IntImm>();
            internal_assert(tmp_int);

            expr = Add::make(op->a, IntImm::make(Int(32), tmp_int->value + 1));   
        } else if (tmp_a) {
            // recursively process
            expr = mutate(op->a) + op->b;
            if (!is_downsampling) {
                expr = expr + 1;
            }
        } else {
            expr = op->a + op->b + 1;
        }
    }

    void visit(const Div *op) {
        //expr = brute_force(op);
        is_downsampling = true;
        Expr temp = mutate(op->a);
        expr = Div::make(temp, op->b);

    }

    void visit(const Mul *op) {
        //const IntImm *tmp_b = op->b.as<IntImm>();
        expr = mutate(op->a) * mutate(op->b);
        debug(3) << "Test " << expr << "\n";
    }

public:
    UpStepDifference() : is_downsampling(false) {}
};

Expr up_step_difference(Expr expr) {
    return UpStepDifference().mutate(expr);
}

class ExpandExpr : public IRMutator {
    using IRMutator::visit;
    const Scope<Expr> &scope;

    void visit(const Variable *var) {
        if (scope.contains(var->name)) {
            expr = scope.get(var->name);
            debug(4) << "Fully expanded " << var->name << " -> " << expr << "\n";
        } else {
            expr = var;
        }
    }

public:
    ExpandExpr(const Scope<Expr> &s) : scope(s) {}

};

// Perform all the substitutions in a scope
Expr expand_expr(Expr e, const Scope<Expr> &scope) {
    ExpandExpr ee(scope);
    Expr result = ee.mutate(e);
    debug(4) << "Expanded " << e << " into " << result << "\n";
    return result;
}

}

class ExpandMulExpr : public IRMutator {
    using IRMutator::visit;
    int mul_factor;
    bool expand_mul;

    void visit(const Mul *var) {
        if (!var->a.as<Mod>() && !var->a.as<Variable>() && !var->a.as<Min>() && var->b.as<IntImm>() && (var->b.as<IntImm>()->value != 0)) {
            expand_mul = true;
            mul_factor = var->b.as<IntImm>()->value;
            expr = mutate(var->a);
            mul_factor = 0;
            expand_mul = false;
        } else {
            expr = var;
        }

        debug(3) << "Fully expanded into " << expr << "\n";
    }

    void visit(const Add *var) {
        Expr temp_expr_a, temp_expr_b;

        if (expand_mul) {
            if (var->a.as<Add>() || var->a.as<Div>()) {
                temp_expr_a = mutate(var->a);
            } else {
                temp_expr_a = Mul::make(var->a, IntImm::make(Int(32), mul_factor));
            }

            if (var->b.as<Add>() || var->b.as<Div>()) {
                temp_expr_b = mutate(var->b);
            } else {
                temp_expr_b = Mul::make(var->b, IntImm::make(Int(32), mul_factor));
            }

            expr = Add::make(temp_expr_a, temp_expr_b);
        } else {
            expr = mutate(var->a) + mutate(var->b);
        }

    }

    void visit(const Div *mul) {

        if (expand_mul) {
            if (mul->b.as<IntImm>()) {
                expr = (expand_mul && mul->b.as<IntImm>()->value == mul_factor) ? mul->a : Mul::make(mul, IntImm::make(Int(32), mul_factor));
            } else {
                expr = mul;
            }
        } else {
            expr = mul;
        }
    }

public:
    ExpandMulExpr() : mul_factor(0), expand_mul(false) {}

};

// Perform all the substitutions in a scope
Expr expand_mul_expr(Expr e) {
    Expr result = ExpandMulExpr().mutate(e);
    debug(4) << "Expanded " << e << " into " << result << "\n";
    return result;
}

class ChangeOffset : public IRMutator {

    Expr offset;
    bool offset_found;
    Expr moved_expr;
    int count;
    bool insert_offset;
    bool remove_offset;
    //std::set<Expr> set_offsets;
    Expr op_a; 
    Expr temp_b;
    bool is_equal;

    using IRMutator::visit;

    void visit(const Variable *op) {
        if (insert_offset) {
            insert_offset  = false;
            remove_offset  = true;
            temp_b = op;
            op_a = mutate(op_a);
            if (is_equal) { 
                expr = make_zero(Int(32));
                is_equal = false;
            } else {
                expr = op;
            }
            debug(3) << "State of op_a " << op_a << "\n\n";
            insert_offset  = true;
            remove_offset  = false;
        } else if (remove_offset)  {
            debug(3) << "CHANGEOFFSET ARIES: " << op->name << " and " << temp_b << "\n";
            if (equal(temp_b, op)) {
                debug(3) << " Equal\n";
                is_equal = true;
                expr = make_zero(Int(32));
            } else {
                expr = op;
            }
        } else {
            expr = op;
        }
    }

    void visit(const Mul *op) {
        if (insert_offset) {
            insert_offset  = false;
            remove_offset  = true;
            temp_b = op;
            op_a = mutate(op_a);
            if (is_equal) { 
                expr = make_zero(Int(32));
                is_equal = false;
            } else {
                expr = op;
            }
            debug(3) << "State of op_a " << op_a << "\n\n";
            insert_offset  = true;
            remove_offset  = false;
        } else if (remove_offset)  {
            debug(3) << "CHANGEOFFSET ARIES: " << op->a << " * " << op->b << " and " << temp_b << "\n";
            if (equal(temp_b, op)) {
                debug(3) << " Equal\n";
                is_equal = true;
                expr = make_zero(Int(32));
            } else {
                expr = op;
            }
        } else {
            expr = op;
        }
    }

    void visit(const Div *op) {
        if (insert_offset) {
            insert_offset  = false;
            remove_offset  = true;
            temp_b = op;
            op_a = mutate(op_a);
            if (is_equal) { 
                expr = make_zero(Int(32));
                is_equal = false;
            } else {
                expr = op;
            }
            debug(3) << "State of op_a " << op_a << "\n\n";
            insert_offset  = true;
            remove_offset  = false;
        } else if (remove_offset)  {
            debug(3) << "CHANGEOFFSET ARIES: " << op->a << " / " << op->b << " and " << temp_b << "\n";
            if (equal(temp_b, op)) {
                debug(3) << " Equal\n";
                is_equal = true;
                expr = make_zero(Int(32));
            } else {
                expr = op;
            }
        } else {
            expr = op;
        }
    }

    void visit(const Min *op) {
        if (insert_offset) {
            insert_offset  = false;
            remove_offset  = true;
            temp_b = op;
            op_a = mutate(op_a);
            if (is_equal) { 
                expr = make_zero(Int(32));
                is_equal = false;
            } else {
                expr = op;
            }
            debug(3) << "State of op_a " << op_a << "\n\n";
            insert_offset  = true;
            remove_offset  = false;
        } else if (remove_offset)  {
            debug(3) << "CHANGEOFFSET ARIES: " << op->a << " min " << op->b << " and " << temp_b << "\n";
            if (equal(temp_b, op)) {
                debug(3) << " Equal\n";
                is_equal = true;
                expr = make_zero(Int(32));
            } else {
                expr = op;
            }
        } else {
            expr = op;
        }
    }

    void visit(const Sub *op) {
        // Assume offset is in op->b
        debug(3) << " Entering with op_a pre " << op->a << "\n";
        op_a = simplify(expand_mul_expr(op->a));
        debug(3) << " Entering with op_a " << op_a << "\n";

        insert_offset = true;
        Expr temp = mutate(op->b);
        insert_offset = false;

        debug(3) << " Remove Offset: " << op_a << " - " << temp << "\n";

        expr = simplify(op_a - temp);

        debug(3) << " Remove Offset post: " << expr << "\n";

        /*
        if (op->a.as<Add>()) {
            offset = op->b;
            count++;
            expr = mutate(op->a) - op->b;
            count--;
        } else if (op->b.as<Add>()) {
            offset = op->a;
            count++;
            expr = op->a - mutate(op->b);
            count--;
        } else {
            expr = op;
        }
        */

    }

    /*
    void visit(const Add *op) {

        if (equal(op->a, offset)) {
            offset_found = true;
            moved_expr = op->b;
        } else if (equal(op->b, offset)) {
            debug(3) << "Found at this location : " << count << " with expr " << op->b << "\n";
            offset_found = true;
            moved_expr = op->a;
        }

        if (offset_found && count > 1) {
            expr = offset;
        } else if (offset_found && count == 1) {
            expr = op;
        } else {
            expr = op;
            if (op->a.as<Add>()) {
                count++;
                expr = mutate(op->a);
                count--;

                if (offset_found) {
                    expr = expr + Add::make(op->b, moved_expr);
                }
            }

            if (!offset_found && op->b.as<Add>()) {
                count++;
                expr = mutate(op->b);
                count--;   

                if (offset_found) {
                    expr = Add::make(op->a, moved_expr) + expr;
                }

            }
        }

    }
    */

public:
    ChangeOffset(Expr offset) : offset(offset), offset_found(false), count(0), insert_offset(false), remove_offset(false), is_equal(false) {}
};

Expr change_offset(Expr e, Expr offset) {
    Expr result = ChangeOffset(offset).mutate(e);
    return result;
}

class ExpandDivExpr : public IRMutator {
    using IRMutator::visit;
    int div_factor;
    bool expand_div;

    void visit(const Div *var) {
        if (!var->a.as<Variable>() && !var->a.as<Min>() && var->b.as<IntImm>() && (var->b.as<IntImm>()->value != 0)) {
            expand_div = true;
            div_factor = var->b.as<IntImm>()->value;
            expr = mutate(var->a);
            div_factor = 0;
            expand_div = false;
        } else {
            expr = var;
        }

        debug(3) << "Fully expanded into " << expr << "\n";
    }

    void visit(const Add *var) {
        Expr temp_expr_a, temp_expr_b;

        if (expand_div) {
            if (var->a.as<Add>() || var->a.as<Mul>()) {
                temp_expr_a = mutate(var->a);
            } else {
                temp_expr_a = Div::make(var->a, IntImm::make(Int(32), div_factor));
            }

            if (var->b.as<Add>() || var->b.as<Mul>()) {
                temp_expr_b = mutate(var->b);
            } else {
                temp_expr_b = Div::make(var->b, IntImm::make(Int(32), div_factor));
            }

            expr = Add::make(temp_expr_a, temp_expr_b);
        } else {
            expr = mutate(var->a) + mutate(var->b);
        }

    }

    void visit(const Mul *mul) {

        if (mul->a.as<Variable>() && mul->b.as<IntImm>()) {
            expr = (expand_div && mul->b.as<IntImm>()->value == div_factor) ? mul->a : mul;
        } else if (mul->b.as<Variable>() && mul->a.as<IntImm>()) {
            expr = (expand_div && mul->a.as<IntImm>()->value == div_factor) ? mul->b : mul;
        } else {
            expr = mul;
        }
    }

public:
    ExpandDivExpr() : div_factor(0), expand_div(false) {}

};

// Perform all the substitutions in a scope
Expr expand_div_expr(Expr e) {
    Expr result = ExpandDivExpr().mutate(e);
    debug(4) << "Expanded " << e << " into " << result << "\n";
    return result;
}

class SimplifyMod : public IRMutator {
    int up_step;
    string kernel_name;
    const Scope<Expr> &scope;
    const Expr offset;
    bool mod;

    using IRMutator::visit;

    void visit(const Mod *op) {
        const IntImm *temp = op->b.as<IntImm>();
        if (temp && up_step == temp->value) {
            //Expr tmp_a = op->a.as<Add>()->a;
            mod = true;

            Expr tmp = expand_mul_expr(Mul::make(offset, IntImm::make(Int(32), 2)));
            debug(3) << "SimplifyMod tmp : " << tmp << "\n";
            Expr tmp_a = simplify(Sub::make(op->a, tmp));
            debug(3) << "SimplifyMod tmp_a : " << tmp_a << "\n";

            //Expr tmp_a = mutate(op->a.as<Add>()->a);
            mod = false;
            expr = Mod::make(tmp_a, op->b);
        } else {
            expr = op;
        }
    }

    void visit(const Variable *op) {
        debug(3) << " Variable : " << op->name << "\n";
        if (mod) {
           expr = op;

        } else {
            expr = op;
        }
    }

public:
    SimplifyMod(int up_step, string kernel_name, const Scope<Expr> &scope, Expr offset) : up_step(up_step), kernel_name(kernel_name), scope(scope), offset(offset), mod(false) {}
};

Expr simplify_mod(Expr e, int up_step, string kernel_name, const Scope<Expr> &scope, Expr offset) {
    Expr result = SimplifyMod(up_step, kernel_name, scope, offset).mutate(e);
    return result;

}

class ReplaceReferencesWithStencil : public IRMutator {
    const HWKernel &kernel;
    const HWKernelDAG &dag;  // FIXME not needed
    Scope<Expr> scope;

    using IRMutator::visit;

    void visit(const For *op) {
        debug(3) << "ReplaceReferencesWithStencil op:" << op->name << " & kernel: " << kernel.name << "\n";
        if (!starts_with(op->name, kernel.name)) {
            debug(3) << "Not starts_with\n";
            // try to simplify trivial reduction loops
            // TODO add assertions to check loop type
            Expr loop_extent = simplify(expand_expr(op->extent, scope));
            if (is_one(loop_extent)) {
                scope.push(op->name, simplify(expand_expr(op->min, scope)));
                Stmt body = mutate(op->body);
                scope.pop(op->name);
                stmt = LetStmt::make(op->name, op->min, body);
            } else {
                Stmt body = mutate(op->body);
                stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, body);
            }
        } else {
            // replace the loop var over the dimensions of the original function
            // realization with the loop var over the stencil dimension.
            // e.g. funcA.s0.x -> funcA.stencil.s0.x
            //      funcA.s1.x -> funcA.stencil.s1.x
            string old_var_name = op->name;
            string stage_dim_name = op->name.substr(kernel.name.size()+1, old_var_name.size() - kernel.name.size());
            string new_var_name = kernel.name + ".stencil." + stage_dim_name;
            Expr new_var = Variable::make(Int(32), new_var_name);

            // find the stencil dimension given dim_name
            int dim_idx = -1;
            for(size_t i = 0; i < kernel.func.args().size(); i++)
                if(ends_with(stage_dim_name, "." + kernel.func.args()[i])) {
                    dim_idx = i;
                    break;
                }
            if (dim_idx == -1) {
                // it is a loop over reduction domain, and we keep it
                // TODO add an assertion
                IRMutator::visit(op);
                return;
            }
            Expr new_min = 0;
            Expr new_extent = kernel.dims[dim_idx].step;

            // create a let statement for the old_loop_var
            Expr old_min = op->min;
            Expr old_var_value = new_var + old_min;

            debug(3) << " PIN_ARIES_1 " << op->name << "with new_var : " << old_var_value << "\n";

            // traversal down into the body
            scope.push(old_var_name, simplify(expand_expr(old_var_value, scope)));
            Stmt new_body = mutate(op->body);
            scope.pop(old_var_name);

            new_body = LetStmt::make(old_var_name, old_var_value, new_body);
            stmt = For::make(new_var_name, new_min, new_extent, op->for_type, op->device_api, new_body);
        }
    }

    void visit(const Provide *op) {
        if(op->name != kernel.name) {
            IRMutator::visit(op);
        } else {
            // Replace the provide node of func with provide node of func.stencil
            string stencil_name = kernel.name + ".stencil";
            vector<Expr> new_args(op->args.size());

            // Replace the arguments. e.g.
            //   func.s0.x -> func.stencil.x
            for (size_t i = 0; i < kernel.func.args().size(); i++) {
                debug(3) << "PIN_ARIES\n";
                //debug(3) << "ReplaceReferences : " << i << " with op->args: " << op->args[i] << "\n";
                //debug(3) << " ReplaceReferences : " << i << " with op->args: " << mutate(op->args[i]) << "\n";
                //debug(3) << " ReplaceReferences kernel: " << kernel.name << " with min_pos: " << kernel.dims[i].min_pos << "\n";
                //debug(3) << " ReplaceReferences kernel: " << kernel.name << " with min_pos: " << expand_expr(mutate(op->args[i]) - kernel.dims[i].min_pos, scope) << "\n";
                new_args[i] = simplify(expand_mul_expr(expand_expr(mutate(op->args[i]) - kernel.dims[i].min_pos, scope)));
                debug(3) << " ReplaceReferences kernel: " << kernel.name << " with new_args: " << new_args[i] << "\n";
            }

            vector<Expr> new_values(op->values.size());
            for (size_t i = 0; i < op->values.size(); i++) {
                debug(3) << "PIN_ARIES pre" << op->values[i] << "\n";
                new_values[i] = mutate(op->values[i]);
                debug(3) << "PIN_ARIES " << new_values[i] << "\n";
            }

            stmt = Provide::make(stencil_name, new_values, new_args);
        }
    }

    void visit(const Call *op) {
        if(op->name == kernel.name || // call to this kernel itself (in update definition)
           std::find(kernel.input_streams.begin(), kernel.input_streams.end(),
                     op->name) != kernel.input_streams.end() // call to a input stencil
           ) {
            // check assumptions
            internal_assert(op->call_type == Call::Halide);

            const auto it = dag.kernels.find(op->name);
            internal_assert(it != dag.kernels.end());
            const HWKernel &stencil_kernel = it->second;
            internal_assert(op->args.size() == stencil_kernel.func.args().size());

            // Replace the call node of func with call node of func.stencil
            string stencil_name = stencil_kernel.name + ".stencil";
            vector<Expr> new_args(op->args.size());

            debug(3) << " PIN_ARIES_6 " << op->name << " and kernel name " << kernel.name << " and stencil_name " << stencil_kernel.name << "\n";
            
            // Mutate the arguments.
            // The value of the new argment is the old_value - stencil.min_pos.
            // The new value shouldn't refer to old loop vars any more
            for (size_t i = 0; i < op->args.size(); i++) {
                Expr old_arg = mutate(op->args[i]);
                Expr offset;
                if (stencil_kernel.name == kernel.name) {
                    // The call is in an update definition of the kernel itself
                    offset = stencil_kernel.dims[i].min_pos;
                } else {
                    // This is call to input stencil
                    // we use the min_pos stored in in_kernel.consumer_stencils
                    const auto it = stencil_kernel.consumer_stencils.find(kernel.name);
                    internal_assert(it != kernel.consumer_stencils.end());
                    offset = it->second[i].min_pos;
                }

                Expr new_arg = old_arg - offset;
                debug(3) << " SCope " << scope << "\n\n";
                debug(3) << "   new_arg " << new_arg << "\n";
                debug(3) << "   old_arg " << old_arg << "\n";
                debug(3) << "   offset " << offset << "\n";
                debug(3) << "   expand_expr " << expand_expr(new_arg, scope) << "\n";
                debug(3) << "   expand_div " << expand_mul_expr(expand_div_expr(expand_expr(new_arg, scope))) << "\n";
                debug(3) << "   simplify_expand_div " << simplify(simplify_mod(expand_div_expr(expand_expr(new_arg, scope)),2, kernel.name, scope, expand_expr(offset, scope))) << "\n";
                debug(3) << "   change_offset " << change_offset(simplify(simplify_mod(expand_div_expr(expand_expr(new_arg, scope)),2, kernel.name, scope, expand_expr(offset, scope))), expand_expr(offset, scope)) << "\n";
                debug(3) << "   simplify change_offset " << simplify(change_offset(simplify(simplify_mod(expand_div_expr(expand_expr(new_arg, scope)),2, kernel.name, scope, expand_expr(offset, scope))), expand_expr(offset, scope))) << "\n";
                new_arg = simplify(change_offset(simplify(simplify_mod(expand_mul_expr(expand_div_expr(expand_expr(new_arg, scope))),2, kernel.name, scope, expand_expr(offset, scope))), expand_expr(offset, scope)));
                //new_arg = simplify(expand_div_expr(expand_expr(new_arg, scope)));
                //debug(3) << "   simplify_mod new_arg " << simplify_mod(new_arg, kernel.dims[i].step) << " with step: " << kernel.dims[i].step << "\n\n";
                debug(3) << "   simplified new_arg " << new_arg << "\n\n";
                // TODO check if the new_arg only depends on the loop vars
                // inside the producer
                new_args[i] = new_arg;
            }
            expr = Call::make(op->type, stencil_name, new_args, Call::Intrinsic);
            debug(4) << "replacing call " << Expr(op) << " with\n"
                     << "\t" << expr << "\n";
        } else {
            IRMutator::visit(op);
        }
    }

    void visit(const Realize *op) {
        debug(3) << " PIN_ARIES_3 " << op->name << "\n";
        // this must be a realize node of a inlined function
        internal_assert(dag.kernels.count(op->name));
        internal_assert(dag.kernels.find(op->name)->second.is_inlined);
        // expand and simplify bound expressions
        // TODO this may not be needed if the letstmt in the scope is well preserved

        Region new_bounds(op->bounds.size());
        bool bounds_changed = false;

        // Mutate the bounds
        for (size_t i = 0; i < op->bounds.size(); i++) {
            Expr old_min    = op->bounds[i].min;
            Expr old_extent = op->bounds[i].extent;
            Expr new_min    = simplify(expand_expr(mutate(op->bounds[i].min), scope));
            Expr new_extent = simplify(expand_expr(mutate(op->bounds[i].extent), scope));
            if (!new_min.same_as(old_min))       bounds_changed = true;
            if (!new_extent.same_as(old_extent)) bounds_changed = true;
            new_bounds[i] = Range(new_min, new_extent);
        }

        Stmt body = mutate(op->body);
        Expr condition = mutate(op->condition);
        if (!bounds_changed &&
            body.same_as(op->body) &&
            condition.same_as(op->condition)) {
            stmt = op;
        } else {
            stmt = Realize::make(op->name, op->types, new_bounds,
                                 condition, body);
        }
    }

    void visit(const Let *op) {
        debug(3) << " PIN_ARIES_4 " << op->name << "\n";
        Expr new_value = simplify(expand_expr(mutate(op->value), scope));
        scope.push(op->name, new_value);
        Expr new_body = mutate(op->body);
        if (new_value.same_as(op->value) &&
            new_body.same_as(op->body)) {
            expr = op;
        } else {
            expr = Let::make(op->name, new_value, new_body);
        }
        scope.pop(op->name);
    }

    void visit(const LetStmt *op) {
        debug(3) << " PIN_ARIES_5 " << op->name << "\n";
        Expr new_value = simplify(expand_expr(op->value, scope));
        scope.push(op->name, new_value);
        Stmt new_body = mutate(op->body);
        if (new_value.same_as(op->value) &&
            new_body.same_as(op->body)) {
            stmt = op;
        } else {
            stmt = LetStmt::make(op->name, new_value, new_body);
        }
        scope.pop(op->name);
    }

public:
    ReplaceReferencesWithStencil(const HWKernel &k, const HWKernelDAG &d,
                                 const Scope<Expr> *s = NULL)
        : kernel(k), dag(d) {
        scope.set_containing_scope(s);
    }
};


Stmt create_dispatch_call(const HWKernel& kernel, int min_fifo_depth = 0) {
    // dispatch the stream into seperate streams for each of its consumers
    // syntax:
    //   dispatch_stream(stream_name, num_of_dimensions,
    //                   stencil_size_dim_0, stencil_step_dim_0, store_extent_dim_0,
    //                   [stencil_size_dim_1, stencil_step_dim_1, store_extent_dim_1, ...]
    //                   num_of_consumers,
    //                   consumer_0_name, fifo_0_depth,
    //                   consumer_0_offset_dim_0, consumer_0_extent_dim_0,
    //                   [consumer_0_offset_dim_1, consumer_0_extent_dim_1, ...]
    //                   [consumer_1_name, ...])
    Expr stream_var = Variable::make(Handle(), kernel.name + ".stencil.stream");
    vector<Expr> dispatch_args({stream_var, (int)kernel.dims.size()});
    for (size_t i = 0; i < kernel.dims.size(); i++) {
        dispatch_args.push_back(kernel.dims[i].size);
        dispatch_args.push_back(kernel.dims[i].step);
        //Expr store_extent = simplify(kernel.dims[i].store_bound.max -
        //                             kernel.dims[i].store_bound.min + 1);

        Expr store_extent;
        //if (kernel.dims[i].up_step != 0) {
        if (is_const(kernel.dims[i].store_bound.max)) {
            store_extent = simplify(kernel.dims[i].store_bound.max -
                                     kernel.dims[i].store_bound.min + 1);
        } else {
            Expr temp = up_step_difference(kernel.dims[i].store_bound.max); 
            debug(3) << "Up step of " << kernel.dims[i].store_bound.max << " with " << up_step_difference(kernel.dims[i].store_bound.max) << "\n";
            store_extent = simplify(expand_mul_expr(temp) -
                                     expand_mul_expr(kernel.dims[i].store_bound.min));
            if (is_zero(store_extent)) {
                store_extent = make_one(Int(32));
            }
        }
        //} else {
        //    store_extent = simplify(kernel.dims[i].store_bound.max -
        //                             kernel.dims[i].store_bound.min + 1);

        //}

        debug(3) << "max | min : " << kernel.dims[i].store_bound.max << " | " << kernel.dims[i].store_bound.min << "\n";
        debug(3) << "Extent: " << store_extent << "\n";
        internal_assert(is_const(store_extent));
        dispatch_args.push_back((int)*as_const_int(store_extent));
    }
    dispatch_args.push_back((int)kernel.consumer_stencils.size());
    for (const auto& p : kernel.consumer_stencils) {
        dispatch_args.push_back(p.first);
        internal_assert(kernel.consumer_fifo_depths.count(p.first));
        dispatch_args.push_back(std::max(min_fifo_depth, kernel.consumer_fifo_depths.find(p.first)->second));
        internal_assert(p.second.size() == kernel.dims.size());
        for (size_t i = 0; i < kernel.dims.size(); i++) {

            debug(3) << "Consumer stencils\n";
            debug(3) << p.second[i].store_bound.min << " | " << kernel.dims[i].store_bound.min << "\n";
            debug(3) << p.second[i].store_bound.max << " | " << p.second[i].store_bound.min << "\n";

            Expr store_offset = simplify(expand_mul_expr(p.second[i].store_bound.min) -
                                         expand_mul_expr(kernel.dims[i].store_bound.min));
            debug(3) << store_offset << "\n";

            //Expr store_extent = simplify(p.second[i].store_bound.max -
            //                             p.second[i].store_bound.min + 1);

            Expr store_extent;
            //if (kernel.dims[i].up_step != 0) {
            if (is_const(p.second[i].store_bound.max)) {
                store_extent = simplify(p.second[i].store_bound.max -
                                         p.second[i].store_bound.min + 1);
            } else {
                Expr temp = up_step_difference(p.second[i].store_bound.max); 
                debug(3) << "Temp " << up_step_difference(p.second[i].store_bound.max) << "\n";
                store_extent = simplify(expand_mul_expr(temp) -
                                         expand_mul_expr(p.second[i].store_bound.min));
                if (is_zero(store_extent)) {
                    store_extent = make_one(Int(32));
                }
            }
            //} else {
            //    store_extent = simplify(p.second[i].store_bound.max -
            //                             p.second[i].store_bound.min + 1);

            //}

            debug(3) << "Extent: " << store_extent << "\n";

            internal_assert(is_const(store_offset));
            internal_assert(is_const(store_extent));
            dispatch_args.push_back((int)*as_const_int(store_offset));
            dispatch_args.push_back((int)*as_const_int(store_extent));
        }
    }
    return Evaluate::make(Call::make(Handle(), "dispatch_stream", dispatch_args, Call::Intrinsic));
}


// Add realize and read_stream calls arround IR s
Stmt add_input_stencil(Stmt s, const HWKernel &kernel, const HWKernel &input) {
    string stencil_name = input.name + ".stencil";
    string stream_name = stencil_name + ".stream";
    Expr stream_var = Variable::make(Handle(), stream_name);
    Expr stencil_var = Variable::make(Handle(), stencil_name);

    // syntax for read_stream()
    // read_stream(src_stream, des_stencil, [consumer_name])
    vector<Expr> args({stream_var, stencil_var});
    if (input.name != kernel.name) {
        // for non-output kernel, we add an addition argument
        args.push_back(kernel.name);
    }
    Stmt read_call = Evaluate::make(Call::make(Handle(), "read_stream", args, Call::Intrinsic));
    Stmt pc = Block::make(ProducerConsumer::make(stencil_name, true, read_call),
                          ProducerConsumer::make(stencil_name, false, s));

    // create a realizeation of the stencil image
    Region bounds;
    for (StencilDimSpecs dim: input.dims) {
        bounds.push_back(Range(0, dim.size));
    }
    s = Realize::make(stencil_name, input.func.output_types(), bounds, const_true(), pc);
    return s;
}

bool need_linebuffer(const HWKernelDAG &dag, const HWKernel &kernel) {
    // check if we need a line buffer
    bool ret = false;
    
    if (dag.input_kernels.count(kernel.name) > 0) {
        if (!kernel.is_output && kernel.dims[0].size > 1 && kernel.dims[0].step == 1) {
            debug(0) << " Need_linebuffer: Kernel " << kernel.name  << kernel.dims[0].size << " " << kernel.dims[0].step << " is line-buffered\n";
            ret =true;
        }
    } else {
        for (size_t i = 0; i < kernel.dims.size(); i++) {
            if (kernel.dims[i].size != kernel.dims[i].step) {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

// IR for line buffers
// A line buffer is instantiated to buffer the stencil_update.stream and
// to generate the stencil.stream
// The former is smaller, which only consist of the new pixels
// sided in each shift of the stencil window.
Stmt add_linebuffer(Stmt s, const HWKernelDAG &dag, const HWKernel &kernel) {
    Stmt ret;
    debug(3) << "Segmentation fault? " << kernel << "\n";
    if (need_linebuffer(dag, kernel)) {
        // Before mutation:
        //       stmt...
        //
        // After mutation:
        //       realize func.stencil.stream {
        //         linebuffer(...)
        //         dispatch(...)
        //         stmt...
        //       }
        string stream_name = kernel.name + ".stencil.stream";
        string update_stream_name = kernel.name + ".stencil_update.stream";
        Expr stream_var = Variable::make(Handle(), stream_name);
        Expr update_stream_var = Variable::make(Handle(), update_stream_name);

        vector<Expr> linebuffer_args({update_stream_var, stream_var});
        // extract the buffer size, and put it into args
        for (size_t i = 0; i < kernel.dims.size(); i++) {
            debug(3) << "linebuffer_aries\n";
            debug(3) << "min | max : " << kernel.dims[i].store_bound.min << " | " << kernel.dims[i].store_bound.max << "\n\n";
            //Expr store_extent = simplify(kernel.dims[i].store_bound.max -
            //                             kernel.dims[i].store_bound.min + 1);
            Expr store_extent;
            Expr temp = up_step_difference(kernel.dims[i].store_bound.max); 
            debug(3) << "Temp " << temp << "\n";
            store_extent = simplify(expand_mul_expr(temp) -
                                     expand_mul_expr(kernel.dims[i].store_bound.min));
           if (is_zero(store_extent)) {
                store_extent = make_one(Int(32));
            }
            linebuffer_args.push_back(store_extent);
        }
        Stmt linebuffer_call = Evaluate::make(Call::make(Handle(), "linebuffer", linebuffer_args, Call::Intrinsic));
        Stmt dispatch_call = create_dispatch_call(kernel);
        Stmt buffer_calls = Block::make(linebuffer_call, dispatch_call);

        // create a realization of the stencil of the window-size
        Region window_bounds;
        for (StencilDimSpecs dim: kernel.dims) {
            window_bounds.push_back(Range(0, dim.size));
        }
        ret = Realize::make(stream_name, kernel.func.output_types(), window_bounds, const_true(), Block::make(linebuffer_call, Block::make(dispatch_call, s)));
    } else {
        user_warning << "No linebuffer inserted after function " << kernel.name
                     << ".\n";

        // Before mutation:
        //       stmt...
        //
        // After mutation:
        //       dispatch(...)
        //       stmt...

        // Ad-hoc fix: place a fifo buffer at the inputs if the inputs is not linebuffered.
        // This works around the issue that read stencil call from a interface stream
        // in the compute kernel cannot fully unrolled
        int force_buffer_depth = 0;
        if (kernel.input_streams.empty() && kernel.consumer_stencils.size() == 1)
            force_buffer_depth = 1;
        Stmt dispatch_call = create_dispatch_call(kernel, force_buffer_depth);
        ret = Block::make(dispatch_call, s);
    }
    return ret;
}


Stmt transform_kernel(Stmt s, const HWKernelDAG &dag, const Scope<Expr> &scope) {
    Stmt ret;
    const Block *op = s.as<Block>();
    if (op) {
        // It should be a block of a Producer-Comsumer pair
        const ProducerConsumer *produce = op->first.as<ProducerConsumer>();
        const ProducerConsumer *consume = op->rest.as<ProducerConsumer>();
        internal_assert(produce && consume &&
                        produce->name == consume->name);

        const HWKernel &kernel = dag.kernels.find(produce->name)->second;
        
        debug(3) << "Entering op " << produce->name << " " << consume->name << " " << kernel.name << "\n";

        internal_assert(!kernel.is_output);
        if (kernel.is_inlined) {
            // if it is a function inlined into the output function,
            // skip transforming this funciton

            // FIXME it is buggy as the inlined function should really
            // nested in the scan loops of the output function
            internal_error;
        }

        // Before mutation:
        //    produce func {...}
        //    consume func
        //
        // After mutation:
        //     realize func.stencil_update.stream {
        //       produce func.stencil_update.stream {
        //         for scan_loops {
        //           realize input1.stencil {
        //             produce input1.stencil {
        //               read_stream(input1.stencil.stream, input1.stencil)
        //             }
        //             ...
        //             realize func.stencil {
        //               produce func.stencil {...}
        //               write_stream(func.stencil_update.stream, func.stencil)
        //       } } } }
        //       realize func.stencil.stream {
        //         linebuffer(...)
        //         dispatch(...)
        //         consume func.stencil.stream
        //       }
        //     }
        string stencil_name = kernel.name + ".stencil";
        string stream_name = need_linebuffer(dag, kernel) ?
            kernel.name + ".stencil_update.stream" : kernel.name + ".stencil.stream";
        Expr stencil_var = Variable::make(Handle(), stencil_name);
        Expr stream_var = Variable::make(Handle(), stream_name);

        // replacing the references to the original realization with refences to stencils
        Stmt new_produce = ReplaceReferencesWithStencil(kernel, dag, &scope).mutate(produce->body);

        debug(3) << "\nNew_produce: " << new_produce << "\n";

        // syntax for write_stream()
        // write_stream(des_stream, src_stencil)
        vector<Expr> write_args({stream_var, stencil_var});
        Stmt write_call = Evaluate::make(Call::make(Handle(), "write_stream", write_args, Call::Intrinsic));

        // realize and PC node for func.stencil
        Stmt stencil_pc = Block::make(ProducerConsumer::make(stencil_name, true, new_produce),
                                      ProducerConsumer::make(stencil_name, false, write_call));

        // create a realization of the stencil of the step-size
        Region step_bounds;
        for (StencilDimSpecs dim: kernel.dims) {
            step_bounds.push_back(Range(0, dim.step));
        }
        Stmt stencil_realize = Realize::make(stencil_name, kernel.func.output_types(), step_bounds, const_true(), stencil_pc);

        // add read_stream for each input stencil (producers fed to func)
        for (const string& s : kernel.input_streams) {
            const auto it = dag.kernels.find(s);
            internal_assert(it != dag.kernels.end());
            stencil_realize = add_input_stencil(stencil_realize, kernel, it->second);
        }

        // insert scan loops
        Stmt scan_loops = stencil_realize;
        int scan_dim = 0;
        for(size_t i = 0; i < kernel.dims.size(); i++) {
            if (kernel.dims[i].loop_var == "undef" )
                continue;

            string loop_var_name = kernel.name + "." + kernel.func.args()[i]
                + ".__scan_dim_" + std::to_string(scan_dim++);

                debug(3) << "Loop_var_name : " << loop_var_name << "\n";

            // Aries Add
            // Check the condition in which the up_step is not zero. Hence, the extent realization can be correctly computed.
            Expr store_extent;
            //if (kernel.dims[i].up_step != 0) {
                Expr temp = up_step_difference(kernel.dims[i].store_bound.max); 
                debug(3) << "Temp change from max: " << kernel.dims[i].store_bound.max << " and min: " << kernel.dims[i].store_bound.min << " to "<< up_step_difference(kernel.dims[i].store_bound.max) << "\n";
                store_extent = simplify(expand_mul_expr(temp) -
                                         expand_mul_expr(kernel.dims[i].store_bound.min));
                if (is_zero(store_extent)) {
                    store_extent = make_one(Int(32));
                }
            //} else {
            //    store_extent = simplify(kernel.dims[i].store_bound.max -
            //                             kernel.dims[i].store_bound.min + 1);

            //}
            debug(3) << "kernel " << kernel.name << " store_extent = " << store_extent << '\n';

            // check the condition for the new loop for sliding the update stencil
            const IntImm *store_extent_int = store_extent.as<IntImm>();

            internal_assert(store_extent_int);
            if (store_extent_int->value % kernel.dims[i].step != 0) {
                // we cannot handle this scenario yet
                //internal_error
                //    << "Line buffer extent (" << store_extent_int->value
                //    << ") is not divisible by the stencil step " << kernel.dims[i].step << '\n';
            }
            int loop_extent = store_extent_int->value / kernel.dims[i].step + ((store_extent_int->value % kernel.dims[i].step != 0) ? 1 : 0);

            // add letstmt to connect old loop var to new loop var_name
            // FIXME this is not correct in general
            scan_loops = LetStmt::make(kernel.dims[i].loop_var, Variable::make(Int(32), loop_var_name), scan_loops);
            scan_loops = For::make(loop_var_name, 0, loop_extent, ForType::Serial, DeviceAPI::Host, scan_loops);
        }

        // Recurse
        Stmt stream_consume = transform_kernel(consume->body, dag, scope);

        // Add line buffer and dispatcher
        Stmt stream_realize = add_linebuffer(stream_consume, dag, kernel);

        // create the PC node for update stream
        Stmt stream_pc = Block::make(ProducerConsumer::make(stream_name, true, scan_loops),
                                     ProducerConsumer::make(stream_name, false, stream_realize));

        // create a realizeation of the stencil stream
        ret = Realize::make(stream_name, kernel.func.output_types(), step_bounds, const_true(), stream_pc);
    } else {
        debug(3) << "Entering else\n";
        // this is the output kernel of the dag
        const HWKernel &kernel = dag.kernels.find(dag.name)->second;
        internal_assert(kernel.is_output);

        string stencil_name = kernel.name + ".stencil";
        string stream_name = kernel.name + ".stencil.stream";
        Expr stream_var = Variable::make(Handle(), stream_name);
        Expr stencil_var = Variable::make(Handle(), stencil_name);

        // replacing the references to the original realization with refences to stencils
        debug(3) << "\nARIES BEFORE PRODUCE: " << s << "\n\n";

        Stmt produce = ReplaceReferencesWithStencil(kernel, dag, &scope).mutate(s);

        debug(3) << "\nARIES PRODUCE: " << produce << "\n\n";

        // syntax for write_stream()
        // write_stream(des_stream, src_stencil)
        vector<Expr> write_args({stream_var, stencil_var});
        // for dag output kernel, we want to record the scan loop vars,
        // so that code gen knows when to assert TLAST signal
        int scan_dim = 0;
        for (size_t i = 0; i < kernel.dims.size(); i++) {
            if (kernel.dims[i].loop_var != "undef") {
                string loop_var_name = kernel.name + "." + kernel.func.args()[i]
                    + ".__scan_dim_" + std::to_string(scan_dim++);
                Expr store_extent = simplify(kernel.dims[i].store_bound.max -
                                             kernel.dims[i].store_bound.min + 1);
                const IntImm *store_extent_int = store_extent.as<IntImm>();
                internal_assert(store_extent_int);
                int loop_extent = store_extent_int->value / kernel.dims[i].step;

                Expr loop_var = Variable::make(Int(32), loop_var_name);
                Expr loop_max = make_const(Int(32), loop_extent - 1);


                debug(3) << "Debug output dag : " << kernel.name << "\n";
                debug(3) << "loop_var : " << loop_var << "\n";
                debug(3) << "loop_max : " << loop_max << "\n";
                debug(3) << "up_step : " << kernel.dims[i].step << "\n";
                debug(3) << "store extent : " << store_extent_int->value << "\n";
                debug(3) << "loop_extent : " << loop_extent << "\n\n";

                write_args.push_back(loop_var);
                write_args.push_back(loop_max);
            }
        }
        Stmt write_call = Evaluate::make(Call::make(Handle(), "write_stream", write_args, Call::Intrinsic));

        Stmt stencil_pc = Block::make(ProducerConsumer::make(stencil_name, true, produce),
                                      ProducerConsumer::make(stencil_name, false, write_call));

        // Detect if the input stream has different dims (e.g. up_step)
        // Aries
        /*
        std::list<std::pair<std::string, std::vector<int>>> sorted_scale_factors;
        std::vector<int> kernel_fac (kernel.dims.size());

        for (const string& s : kernel.input_streams) {
            const auto it = dag.kernels.find(s);
            internal_assert(it != dag.kernels.end());
            
            const HWKernel cur_kernel = it->second;
            std::vector<int> factor(kernel.dims.size());

            for (size_t i = 0; i < kernel.dims.size(); i++) {
                if (kernel.dims[i].loop_var == "undef") 
                    continue;

                debug(3) << "Test";
                if (cur_kernel.dims[i].up_step != kernel.dims[i].up_step ) {
                    // Insert for loop and then add_input_stencil
                    factor[i] = (cur_kernel.dims[i].up_step > kernel.dims[i].up_step) ? cur_kernel.dims[i].up_step : kernel.dims[i].up_step;
                } else {
                    factor[i] = 0;
                }
                debug(3) << "Factor: " << i << " is " << factor[i] << "\n";
            }

            // Update step
            for (size_t i = 0; i < kernel.dims.size(); i++) {
                if (factor[i] > 0) {
                    kernel_fac[i] = factor[i];
                }
            }

            debug(3) << "Insert the new scale information\n";

            if (sorted_scale_factors.empty()) {
                sorted_scale_factors.push_back( std::pair<std::string, std::vector<int>>( cur_kernel.name, factor ));
            } else {
                // Scan throught the scale factors
                std::list<std::pair<std::string, std::vector<int>>>::iterator factor_it = sorted_scale_factors.begin();
                for (const auto& p : sorted_scale_factors) {
                    if (p.second[0] <  factor[0] ) { // To be fixed}
                        factor_it++;
                    } else {
                        break;
                    }
                }

                sorted_scale_factors.insert(factor_it, std::pair<std::string, std::vector<int>>( cur_kernel.name, factor ));
            }

        }
        */
        // create a realization of the stencil image
        Region bounds;
        for (StencilDimSpecs dim: kernel.dims) {
            //if (kernel_fac[0] != 0)
            //    bounds.push_back(Range(0, kernel_fac[0]));
            //else
                bounds.push_back(Range(0, dim.step));
        }
        Stmt stencil_realize = Realize::make(stencil_name, kernel.func.output_types(), bounds, const_true(), stencil_pc);

        // add read_stream for each input stencil (producers fed to func)
        for (const string& s : kernel.input_streams) {
            /*
        int factor_temp = 0;
        for (const auto& p : sorted_scale_factors) {
            const string& s = p.first;

            if ((p.second)[0] > factor_temp) {
                // Add for_loop with the new factor scale

                Stmt scan_loops = stencil_realize;
                scan_dim = 0;

                for(size_t i = 0; i < p.second.size(); i++) {
                    if (kernel.dims[i].loop_var == "undef" )
                        continue;

                    string loop_var_name = kernel.name + "." + kernel.func.args()[i]
                        + ".__scale_scan_dim_" + std::to_string(scan_dim++);

                    int loop_extent = p.second[i];

                    // add letstmt to connect old loop var to new loop var_name
                    // FIXME this is not correct in general
                    scan_loops = LetStmt::make(kernel.dims[i].loop_var, Variable::make(Int(32), loop_var_name + "_scale"), scan_loops);
                    scan_loops = For::make(loop_var_name, 0, loop_extent, ForType::Serial, DeviceAPI::Host, scan_loops);
               }

               stencil_realize = Block::make(ProducerConsumer::make(stream_name, true, scan_loops),
                          ProducerConsumer::make(stream_name, false, Evaluate::make(0)));

            }

            factor_temp = (p.second)[0];
                */
            const auto it = dag.kernels.find(s);
            internal_assert(it != dag.kernels.end());
            stencil_realize = add_input_stencil(stencil_realize, kernel, it->second);
        }

        // insert scan loops
        Stmt scan_loops = stencil_realize;
        scan_dim = 0;
        for(size_t i = 0; i < kernel.dims.size(); i++) {
            if (kernel.dims[i].loop_var == "undef" )
                continue;

            string loop_var_name = kernel.name + "." + kernel.func.args()[i]
                + ".__scan_dim_" + std::to_string(scan_dim++);

            Expr store_extent = simplify(kernel.dims[i].store_bound.max -
                                         kernel.dims[i].store_bound.min + 1);
            debug(3) << "kernel " << kernel.name << " store_extent = " << store_extent << '\n';

            // check the condition for the new loop for sliding the update stencil
            const IntImm *store_extent_int = store_extent.as<IntImm>();
            internal_assert(store_extent_int);
            if (store_extent_int->value % kernel.dims[i].step != 0) {
                // we cannot handle this scenario yet
                //internal_error
                //    << "Line buffer extent (" << store_extent_int->value
                //    << ") is not divisible by the stencil step " << kernel.dims[i].step << '\n';
            }
            int loop_extent = store_extent_int->value / kernel.dims[i].step;

            // add letstmt to connect old loop var to new loop var_name
            // FIXME this is not correct in general
           // if (kernel_fac[i] != 0) {
            //    scan_loops = LetStmt::make(kernel.dims[i].loop_var + "_scale", Variable::make(Int(32), loop_var_name) * IntImm::make(Int(32), kernel_fac[i]), scan_loops);
            //} else {
                scan_loops = LetStmt::make(kernel.dims[i].loop_var, Variable::make(Int(32), loop_var_name), scan_loops);
            //}
            scan_loops = For::make(loop_var_name, 0, loop_extent, ForType::Serial, DeviceAPI::Host, scan_loops);
        }

        ret = Block::make(ProducerConsumer::make(stream_name, true, scan_loops),
                          ProducerConsumer::make(stream_name, false, Evaluate::make(0)));
    }
    return ret;
}


class TransformTapStencils : public IRMutator {
    const map<string, HWTap> &taps;

    using IRMutator::visit;

    // Replace calls to ImageParam with calls to Stencil
    void visit(const Call *op) {
        if (taps.count(op->name) == 0) {
            IRMutator::visit(op);
        } else if (op->call_type == Call::Image || op->call_type == Call::Halide) {
            debug(3) << "replacing " << op->name << '\n';
            const HWTap &tap = taps.find(op->name)->second;

            // Replace the call node of func with call node of func.tap.stencil
            string stencil_name = op->name + ".tap.stencil";
            vector<Expr> new_args(op->args.size());

            // Mutate the arguments.
            // The value of the new argment is the old_value - min_pos
            // b/c stencil indices always start from zero
            internal_assert(tap.dims.size() == op->args.size());
            for (size_t i = 0; i < op->args.size(); i++) {
                 new_args[i] = op->args[i]- tap.dims[i].min_pos;
            }
            expr = Call::make(op->type, stencil_name, new_args, Call::Intrinsic);
        } else {
            internal_error << "unexpected call_type\n";
        }
    }

public:
    TransformTapStencils(const map<string, HWTap> &t) : taps(t) {}
};

// Perform streaming optimization for all functions
class StreamOpt : public IRMutator {
    const HWKernelDAG &dag;
    Scope<Expr> scope;

    using IRMutator::visit;

    void visit(const For *op) {
        debug(3) << "Visit For " << op->name << "\n";
        if (!dag.store_level.match(op->name) && !dag.loop_vars.count(op->name)) {
            IRMutator::visit(op);
        } else if (dag.compute_level.match(op->name)) {
            internal_assert(dag.loop_vars.count(op->name));

            debug(3) << "Entering dag.compute_level.match()\n";

            // walk inside of any let statements
            Stmt body = op->body;

            vector<pair<string, Expr>> lets;
            while (const LetStmt *let = body.as<LetStmt>()) {
                body = let->body;
                scope.push(let->name, simplify(expand_expr(let->value, scope)));
                lets.push_back(make_pair(let->name, let->value));
            }

            Stmt new_body = transform_kernel(body, dag, scope);

            // insert line buffers for input streams
            for (const string &kernel_name : dag.input_kernels) {
                for (const auto &pair : dag.kernels) {
                    std::cout << pair.first << ": " << pair.second << '\n';
                }
                const HWKernel &input_kernel = dag.kernels.find(kernel_name)->second;
                debug(3) << "debug seg fault : " << kernel_name << "\n";
                new_body = add_linebuffer(new_body, dag, input_kernel);
            }

            // Rewrap the let statements
            for (size_t i = lets.size(); i > 0; i--) {
                new_body = LetStmt::make(lets[i-1].first, lets[i-1].second, new_body);
            }

            // remove the loop statement if it is one of the scan loops
            stmt = new_body;
        } else if (dag.loop_vars.count(op->name)){
            debug(3) << "Entering dag.loop_vars.match() " << op->name << "\n";

            // remove the loop statement if it is one of the scan loops
            stmt = mutate(op->body);
        } else {
            internal_assert(dag.store_level.match(op->name));
            debug(3) << "find the pipeline producing " << dag.name << "\n";

            // walk inside of any let statements
            Stmt body = op->body;

            vector<pair<string, Expr>> lets;
            while (const LetStmt *let = body.as<LetStmt>()) {
                body = let->body;
                scope.push(let->name, simplify(expand_expr(let->value, scope)));
                lets.push_back(make_pair(let->name, let->value));
            }

            debug(3) << "\nBody before mutating for " << dag.name << "\n";
            debug(3) << body << "\n\n";

            Stmt new_body = mutate(body);

            //stmt = For::make(dag.name + ".accelerator", 0, 1, ForType::Serial, DeviceAPI::Host, body);
            const string target_name = "_hls_target." + dag.name;
            new_body = Block::make(ProducerConsumer::make(target_name, true, new_body),
                                   ProducerConsumer::make(target_name, false, Evaluate::make(0)));

            debug(3) << "\nBody for " << target_name << "\n";
            debug(3) << new_body << "\n\n";

            // add declarations of inputs and output (external) streams outside the hardware pipeline IR
            vector<string> external_streams;
            external_streams.push_back(dag.name);
            external_streams.insert(external_streams.end(), dag.input_kernels.begin(), dag.input_kernels.end());
            for (const string &name : external_streams) {
                const HWKernel kernel = dag.kernels.find(name)->second;
                string stream_name = need_linebuffer(dag, kernel) ?
                    kernel.name + ".stencil_update.stream" : kernel.name + ".stencil.stream";

                string direction = kernel.is_output ? "stream_to_buffer" : "buffer_to_stream";
                Expr stream_var = Variable::make(Handle(), stream_name);

                // derive the coordinate of the sub-image block
                internal_assert(kernel.func.output_types().size() == 1);
                vector<Expr> image_args;
                for (size_t i = 0; i < kernel.dims.size(); i++) {
                    image_args.push_back(kernel.dims[i].store_bound.min);
                }
                Expr address_of_subimage_origin = Call::make(Handle(), Call::address_of, {Call::make(kernel.func, image_args, 0)}, Call::Intrinsic);
                Expr buffer_var = Variable::make(type_of<struct buffer_t *>(), kernel.name + ".buffer");

                // add intrinsic functions to convert memory buffers to streams
                // syntax:
                //   stream_subimage(direction, buffer_var, stream_var, address_of_subimage_origin,
                //                   dim_0_stride, dim_0_extent, ...)
                vector<Expr> stream_call_args({direction, buffer_var, stream_var, address_of_subimage_origin});
                for (size_t i = 0; i < kernel.dims.size(); i++) {
                    stream_call_args.push_back(Variable::make(Int(32), kernel.name + ".stride." + std::to_string(i)));
                    stream_call_args.push_back(simplify(kernel.dims[i].store_bound.max - kernel.dims[i].store_bound.min + 1));
                }
                Stmt stream_subimg = Evaluate::make(Call::make(Handle(), "stream_subimage", stream_call_args, Call::Intrinsic));

                Region bounds;
                for (StencilDimSpecs dim: kernel.dims) {
                    debug(3) << "StencilDimSpecs information\n";
                    debug(3) << "dim " << dim.loop_var << " size: " << dim.size << " step: " << dim.step << " up_step: " << dim.up_step << "\n";
                    if (need_linebuffer(dag, kernel)) {
                        // bounds.push_back(Range(0, 1));
                        bounds.push_back(Range(0, dim.step));
                    } else{
                        bounds.push_back(Range(0, dim.step));
                    }
                }
                new_body = Realize::make(stream_name, kernel.func.output_types(), bounds, const_true(), Block::make(stream_subimg, new_body));
            }

            // Handle tap values
            new_body = TransformTapStencils(dag.taps).mutate(new_body);

            // Declare and initialize tap stencils with buffers
            // TODO move this call out side the tile loops over the kernel launch
            for (const auto &p : dag.taps) {
                const HWTap &tap = p.second;
                const string stencil_name = tap.name + ".tap.stencil";

                Expr buffer_var = Variable::make(type_of<struct buffer_t *>(), tap.name + ".buffer");
                Expr stencil_var = Variable::make(Handle(), stencil_name);
                vector<Expr> args({buffer_var, stencil_var});
                Stmt convert_call = Evaluate::make(Call::make(Handle(), "buffer_to_stencil", args, Call::Intrinsic));

                // create a realizeation of the stencil
                Region bounds;
                for (const auto &dim : tap.dims) {
                    bounds.push_back(Range(0, dim.size));
                }
                vector<Type> types = tap.is_func ? tap.func.output_types() :
                    vector<Type>({tap.param.type()});
                new_body = Realize::make(stencil_name, types, bounds, const_true(), Block::make(convert_call, new_body));
            }

            // Rewrap the let statements
            for (size_t i = lets.size(); i > 0; i--) {
                new_body = LetStmt::make(lets[i-1].first, lets[i-1].second, new_body);
            }
            stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, new_body);
        }
    }

    // Remove the realize node for intermediate functions in the hardware pipeline,
    // as we will create linebuffers in the pipeline to hold these values
    void visit(const Realize *op) {
        if (dag.kernels.count(op->name) &&
            !dag.kernels.find(op->name)->second.is_inlined && // is a linebuffered function
            dag.name != op->name && // not the output
            dag.input_kernels.count(op->name) == 0 // not a input
            ) {
            stmt = mutate(op->body);
            // check constraints
            /*
            for (size_t i = 0; i < kernel.dims.size(); i++) {
                Expr store_extent = simplify(kernel.dims[i].store_bound.max -
                                             kernel.dims[i].store_bound.min + 1);
                if (!is_zero(simplify(op->bounds[i].extent - store_extent))) {
                    user_error << "linebuffer (" << store_extent << ") for " << op->name
                               << " is not equal to realize extent (" << op->bounds[i].extent << ").\n";
                }
            }
            */
        } else {
            IRMutator::visit(op);
        }
    }

    void visit(const LetStmt *op) {
        Expr new_value = simplify(expand_expr(op->value, scope));
        scope.push(op->name, new_value);
        Stmt new_body = mutate(op->body);
        if (new_value.same_as(op->value) &&
            new_body.same_as(op->body)) {
            stmt = op;
        } else {
            stmt = LetStmt::make(op->name, new_value, new_body);
        }
        scope.pop(op->name);
    }

public:
    StreamOpt(const HWKernelDAG &d)
        : dag(d) {}
};

Stmt stream_opt(Stmt s, const HWKernelDAG &dag) {
    debug(3) << s << "\n";
    s = StreamOpt(dag).mutate(s);
    debug(3) << s << "\n";
    return s;
}

}
}
