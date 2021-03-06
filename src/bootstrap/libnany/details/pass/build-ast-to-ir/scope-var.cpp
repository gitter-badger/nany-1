#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/ast/ast.h"
#include "libnany-config.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{

	inline bool
	Scope::visitASTVarValueInitialization(LVID& localvar, Node& varAssign, const Node& varnodeDecl, const AnyString& varname)
	{
		for (auto& assignptr: varAssign.children)
		{
			auto& assignChild = *assignptr;
			switch (assignChild.rule)
			{
				case rgExpr:
				{
					setErrorFrom(assignChild);
					bool success = visitASTExpr(assignChild, localvar);

					if (unlikely(not success))
					{
						error(varnodeDecl) << "could not declare '" << varname << "': error in initialization";
						return false;
					}
					break;
				}
				case rgOperatorKind:
				{
					break; // operator =
				}
				default:
					return ICEUnexpectedNode(assignChild, "[var]");
			}
		}
		return true;
	}


	inline bool Scope::generateTypeofForClassVar(LVID& lvid, const Node& varAssign)
	{
		// typeof (+2)
		//	 tk-typeof, typeof
		//	 call (+3)
		//		 tk-parenthese-open, (
		//		 call-parameter
		//		 |   expr
		//		 |	   identifier: A
		Node::Ptr typeofn = new Node{rgTypeof};

		Node::Ptr call = new Node{rgCall};
		typeofn->children.push_back(call);

		// first parameter - the expr
		{
			Node::Ptr param = new Node{rgCallParameter};
			call->children.push_back(param);

			uint index = varAssign.findFirst(rgExpr);
			if (unlikely(not (index < varAssign.children.size())))
			{
				assert(false and "no node <expr> in <var-assign>");
				return false;
			}
			param->children.push_back(varAssign.children[index]);
		}
		return visitASTExprTypeof(*typeofn, lvid);
	}


	inline bool Scope::generateInitFuncForClassVar(const AnyString& varname, LVID lvid, const Node& varAssign)
	{
		// name of the generated func for initialize the class variable
		ShortString64 funcName;
		funcName << "^default-var-%" << lvid << '-' << varname;
		assert(lvid > 0);


		// generating INIT
		//
		// variable member initialization is merely a call to the intrinsic `__nanyc_fieldset`
		// the first parameter is the result of the default value
		// the second parameter is the register where the name of member has been stored

		auto funcInitNode = AST::createNodeFunc(funcName);
		{
			Node::Ptr funcBody = new Node{rgFuncBody};
			funcInitNode->children.push_back(funcBody);

			Node::Ptr expr = new Node{rgExpr};
			funcBody->children.push_back(expr);

			// intrinsic (+2)
			//       entity (+3)
			//       |   identifier: nanyc
			//       |   identifier: fieldset
			//       call (+7)
			//           call-parameter
			//           |   expr
			//           |       <expr A>
			//           call-parameter
			//           |   expr
			//           |       <expr B>
			Node::Ptr intrinsic = new Node{rgIntrinsic};
			expr->children.push_back(intrinsic);
			intrinsic->children.push_back(AST::createNodeIdentifier("__nanyc_fieldset"));

			Node::Ptr call = new Node{rgCall};
			intrinsic->children.push_back(call);

			// param 2 - expr
			{
				Node::Ptr callparam = new Node{rgCallParameter};
				call->children.push_back(callparam);

				uint index = varAssign.findFirst(rgExpr);
				if (unlikely(not (index < varAssign.children.size())))
				{
					assert(false and "no node <expr> in <var-assign>");
					return false;
				}
				callparam->children.push_back(varAssign.children[index]);
			}

			// param text varname
			{
				Node::Ptr callparam = new Node{rgCallParameter};
				call->children.push_back(callparam);
				Node::Ptr pexpr = new Node{rgExpr};
				callparam->children.push_back(pexpr);

				Node::Ptr value  = new Node{rgStringLiteral};
				pexpr->children.push_back(value);
				value->text = varname;
			}
		}
		return visitASTFunc(*funcInitNode);
	}


	/*
	inline bool Scope::visitASTVarClass(Node& varAssign, Node& varnodeDecl, const AnyString& varname)
	{
		// the local id of the class variable
		LVID lvid = reserveLocalVariable();

		// generate the opcode for registering the variable definition
		{
			auto& operands = program().append<ISA::Opcode::blueprintClassVar>();
			operands.lvid = lvid;
			operands.name  = stringrefs().acquire(varname).c_str();
			operands.originFilename = stringrefs().acquire(pContext.filename).c_str();
			operands.visibility = nyv_default;
			operands.isProperty = false;
			operands.atomid = (yuint32) -1;
			fetchLineAndOffsetFromNode(varnodeDecl, operands.originLine, operands.originOffset);
		}

		// as a reminder, here is what an intrinsic looks like:
		// |   intrinsic (+2)
		// |	   entity
		// |	   |   identifier: intrinsic_func_name
		// |	   call (+5)
		// |		   call-parameter
		// |		   |   expr
		// |		   |	   identifier: a
		// |		   call-parameter
		// |		   |   expr
		// |		   |	   identifier: b

		ShortString64 generatedName;
		ShortString128 hiddenVarname;
		hiddenVarname << '^' << varname;

		// generating an AST to produce the corresponding opcodes
		// generating GETTER
		{
			generatedName.clear();
			auto funcGetNode = AST::createNodeFunc(generatedName << "@_get_" << lvid);

			// func-return-type (+2)
			// |   tk-colon, :
			// |   type
			// |	   type-qualifier
			// |		   ref, ref
			{
				Node::Ptr funcReturnType = new Node{rgFuncReturnType};
				funcGetNode->children.push_back(funcReturnType);
				Node::Ptr type = new Node{rgType};
				funcReturnType->children.push_back(type);
				Node::Ptr typeQualifier = new Node{rgTypeQualifier};
				typeQualifier->children.push_back(new Node{rgRef});
				type->children.push_back(typeQualifier);
			}

			// func-body
			//	return (+2)
			//	   expr
			{
				Node::Ptr funcBody = new Node{rgFuncBody};
				funcGetNode->children.push_back(funcBody);

				Node::Ptr funcReturn = new Node{rgReturn};
				funcBody->children.push_back(funcReturn);
				Node::Ptr expr = new Node{rgExpr};
				funcReturn->children.push_back(expr);
				expr->children.push_back(AST::createNodeIdentifier(hiddenVarname));
			}

			visitASTFunc(*funcGetNode);
		}

		// generating SETTER
		{
			generatedName.clear();
			auto funcSetNode = AST::createNodeFunc(generatedName << "@_set_" << lvid);

			// func-params (+3)
			// |   func-param (+2)
			// |   |   ref, ref
			{
				Node::Ptr funcParams = new Node{rgFuncParams};
				funcSetNode->children.push_back(funcParams);
				Node::Ptr firstParam = new Node{rgFuncParam};
				funcParams->children.push_back(firstParam);
				firstParam->children.push_back(new Node{rgRef});
				// 'value' is the (pseudo) reserved keyword for properties
				firstParam->children.push_back(AST::createNodeIdentifier("value"));
			}

			// func-body
			//	 expr
			//		 identifier '='
			//			 call
			//				 call-parameter
			//					 expr
			//						identifier 'var'
			//				 call-parameter
			//					 expr
			//						<value>
			Node::Ptr funcBody = new Node{rgFuncBody};
			funcSetNode->children.push_back(funcBody);

			Node::Ptr expr = new Node{rgExpr};
			funcBody->children.push_back(expr);
			Node::Ptr identifierCall = AST::createNodeIdentifier("=", true);
			expr->children.push_back(identifierCall);

			Node::Ptr call = new Node{rgCall};
			identifierCall->children.push_back(call);

			// first parameter - the target variable
			{
				Node::Ptr param = new Node{rgCallParameter};
				call->children.push_back(param);
				Node::Ptr prmexpr = new Node{rgExpr};
				param->children.push_back(prmexpr);
				prmexpr->children.push_back(AST::createNodeIdentifier(hiddenVarname, true));
			}

			// second parameter - the default value itself
			{
				Node::Ptr param = new Node{rgCallParameter};
				call->children.push_back(param);
				Node::Ptr prmexpr = new Node{rgExpr};
				param->children.push_back(prmexpr);
				prmexpr->children.push_back(AST::createNodeIdentifier("value", true));
			}

			visitASTFunc(*funcSetNode);
		}

		return true;
	}
	*/


	bool Scope::visitASTVar(Node& node)
	{
		assert(node.rule == rgVar);
		assert(not node.children.empty());

		// variable name
		AnyString varname;
		bool ref = false;
		bool constant = false;
		Node* varnodeDecl = nullptr;
		Node* varAssign = nullptr;
		bool isProperty = false;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgIdentifier:
				{
					varnodeDecl = &child;
					varname = child.text;
					setErrorFrom(child);
					if (unlikely(not checkForValidIdentifierName(pContext.report, child, varname, false)))
						return false;
					break;
				}
				case rgVarAssign:
				{
					varAssign = &child;
					break;
				}

				case rgVarProperty:
				{
					isProperty = true;
					varAssign = &child;
					break;
				}

				case rgVarType:
				{
					if (varnodeDecl)
						error(*varnodeDecl) << "variable '" << varname << "': type declaration not implemented";
					else
						error(node) << "variable '" << varname << "': type declaration not implemented";
					return false;
				}

				case rgVarByValue:
				{
					break; // nothing to do
				}

				case rgConst:
				{
					constant = true;
					warning(child) << "'const' in var declaration is currently not supported";
					(void) constant;
					break;
				}
				case rgRef:
				{
					ref = true;
					break;
				}
				case rgCref:
				{
					warning(child) << "'const' in var declaration is currently not supported";
					ref = true;
					constant = true;
					break;
				}

				default:
					return ICEUnexpectedNode(child, "[var]");
			}
		}

		if (unlikely(varnodeDecl == nullptr))
		{
			report().ICE() << "invalid null pointer for the var-decl node";
			return false;
		}

		if (unlikely(varAssign == nullptr)) // the variable currently must have a default value
		{
			error(*varnodeDecl) << "value initialization is missing for '" << varname << "'";
			return false;
		}

		if (not isProperty)
		{
			switch (kind)
			{
				case Kind::kclass:
				{
					if (debugmode)
					{
						program().emitComment();
						program().emitComment(String{"class var "} << varname);
					}
					// the new member variable
					auto mbvar = nextvar();
					program().emitStackalloc(mbvar, nyt_any);

					// the type of the expression
					LVID lvid;
					// type definition, via typeof(varAssign)
					if (not generateTypeofForClassVar(lvid, *varAssign))
						return false;
					if (0 == lvid)
						return false;

					// follow
					{
						auto& operands	= program().emit<ISA::Op::follow>();
						operands.follower = mbvar;
						operands.lvid	 = lvid;
						operands.symlink  = 0;
					}
					// preserve var / ref / const
					program().emitQualifierRef(mbvar, ref);
					if (constant)
						program().emitQualifierConst(mbvar, true);

					// variable definition
					emitDebugpos(*varnodeDecl);
					program().emitBlueprintVardef(mbvar, varname);

					// generating an INIT func for the variable
					if (not generateInitFuncForClassVar(varname, mbvar, *varAssign))
						return false;
					break;
				}
				case Kind::kfunc:
				{
					// create the variable itself
					uint32_t varlvid  = program().emitStackalloc(nextvar(), nyt_any);
					program().emitQualifierRef(varlvid, ref);
					program().emitQualifierConst(varlvid, constant);

					{
						LVID rhs = 0;
						IR::OpcodeScopeLocker opscope{program()};
						if (not visitASTVarValueInitialization(rhs, *varAssign, *varnodeDecl, varname))
							return false;

						assert(rhs != 0);
						if (not ref)
						{
							program().emitAssign(varlvid, rhs, false);
						}
						else
						{
							program().emitStore(varlvid, rhs); // re-acquire to keep the value alive
							program().emitRef(varlvid);
						}
					}

					// important: the alias must be declared *after* the right value
					// (otherwise it may be used by the code)
					emitDebugpos(*varnodeDecl); // reset the debug position
					program().emitNameAlias(varlvid, varname);
					break;
				}
				default:
				{
					ICE(*varnodeDecl) << "var declaration: invalid scope type";
					return false;
				}
			}
		}
		else
		{
			// the declaration is a property
			ICE(*varnodeDecl) << "properties not implemented";
			return false;
		}

		return true;
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
