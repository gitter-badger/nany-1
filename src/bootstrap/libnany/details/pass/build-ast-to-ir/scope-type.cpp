#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{

	bool Scope::visitASTExprTypeDecl(Node& node, LVID& localvar)
	{
		assert(node.rule == rgTypeDecl);
		localvar = 0;

		if (node.children.size() == 1)
		{
			auto& identifier = *(node.children[0]);
			if (identifier.children.empty())
			{
				if (identifier.text == "any") // no real information, already 'any'
				{
					localvar = LVID(-1);
					return true;
				}

				if (identifier.text == "void")
				{
					localvar = 0;
					return true;
				}
			}
		}

		IR::Producer::Scope scope{*this};
		OpcodeCodegenDisabler codegenDisabler{program()};
		return scope.visitASTExpr(node, localvar);
	}



	bool Scope::visitASTType(Node& node, LVID& localvar)
	{
		assert(node.rule == rgType);
		assert(not node.children.empty());

		bool success = true;
		bool reallyVoid = false;
		bool isRef = false;
		bool isConst = false;
		localvar = 0; // reset

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			switch (child.rule)
			{
				case rgTypeDecl:
				{
					if (unlikely(localvar != 0))
						return ICEUnexpectedNode(child, "[ir/new/several calls]");

					bool status = visitASTExprTypeDecl(child, localvar);
					success &= status;

					if (status and localvar == 0)
						reallyVoid = true;
					break;
				}

				case rgTypeQualifier:
				{
					for (auto& qualifier: child.children)
					{
						switch (qualifier->rule)
						{
							case rgRef:   isRef   = true; break;
							case rgConst: isConst = true; break;
							case rgCref:  isRef   = true; isConst = true; break;
							default:
								success = ICEUnexpectedNode(child, "[ir/type-qualifier]");
						}
					}
					break;
				}

				default:
					success = ICEUnexpectedNode(child, "[ir/type]");
			}
		}

		// create a value even if nothing to always have an attached value
		if (localvar == 0 and not reallyVoid /*any*/)
			localvar = reserveLocalVariable();

		if (0 != localvar)
		{
			if (localvar == (uint32_t) -1)
				localvar = program().emitStackalloc(reserveLocalVariable(), nyt_any);

			program().emitQualifierRef(localvar, isRef);
			program().emitQualifierConst(localvar, isConst);
		}
		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
