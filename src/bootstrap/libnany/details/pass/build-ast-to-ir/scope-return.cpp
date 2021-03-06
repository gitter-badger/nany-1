#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprReturn(Node& node)
	{
		assert(node.rule == rgReturn);
		// assert(not node.children.empty()); -- a return may be empty

		bool success = true;
		bool hasReturnValue = false;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			switch (child.rule)
			{
				case rgExpr:
				{
					IR::OpcodeScopeLocker opscope{program()};
					LVID localvar;
					success &= visitASTExpr(child, localvar);

					// generate error on the begining of the expr and not the return itself
					emitDebugpos(child);

					uint32_t copy = program().emitStackalloc(nextvar(), nyt_any);
					{
						auto& operands    = program().emit<ISA::Op::follow>();
						operands.follower = copy;
						operands.lvid     = 1; // return type
						operands.symlink  = 0; // true
					}
					program().emitInheritQualifiers(copy, /*ret*/1);

					program().emitAssign(copy, localvar, false);

					program().emitReturn(copy);
					hasReturnValue = true;
					break;
				}

				default:
					return ICEUnexpectedNode(child, "[ir/return]");
			}
		}

		if (not hasReturnValue)
		{
			emitDebugpos(node);
			program().emitReturn();
		}
		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
