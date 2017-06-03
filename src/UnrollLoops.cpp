#include "UnrollLoops.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Simplify.h"
#include "Substitute.h"

using std::vector;

namespace Halide {
namespace Internal {

class UnrollLoops : public IRMutator {
    using IRMutator::visit;

    void visit(const For *for_loop) {
        if (for_loop->for_type == ForType::Unrolled) {
            // Give it one last chance to simplify to an int
            Expr extent = simplify(for_loop->extent);
            const IntImm *e = extent.as<IntImm>();
            user_assert(e)
                << "Can only unroll for loops over a constant extent.\n"
                << "Loop over " << for_loop->name << " has extent " << extent << ".\n";
            Stmt body = mutate(for_loop->body);

            if (e->value == 1) {
                user_warning << "Warning: Unrolling a for loop of extent 1: " << for_loop->name << "\n";
            }

            vector<Stmt> iters;
            // Make n copies of the body, each wrapped in a let that defines the loop var for that body
            for (int i = 0; i < e->value; i++) {
                iters.push_back(substitute(for_loop->name, for_loop->min + i, body));
            }
            stmt = Block::make(iters);

        } else {
            IRMutator::visit(for_loop);
        }
    }

    void visit(const Realize *op) {

        debug(3) << "Visit Realize : " << op->name << "\n";
        debug(3) << "  Type        : " << op->types[0] << "\n";
        
        IRMutator::visit(op);
    }

    void visit(const Variable *op) {
        debug(3) << "Visit Variable : " << op->name << "\n";
        debug(3) << "  Type        : " << op->type << "\n";
        
        IRMutator::visit(op);
        
    }

    void visit(const Provide *op) {
        debug(3) << "Visit Proide : " << op->name << "\n";
        //debug(3) << "  Type        : " << op->type << "\n";
        
        IRMutator::visit(op);
    }
};

Stmt unroll_loops(Stmt s) {
    return UnrollLoops().mutate(s);
}

}
}
