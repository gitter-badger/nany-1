#include "details/ast/ast.h"

using namespace Yuni;



namespace Nany
{
namespace AST
{


	Node* createNodeFunc(const AnyString& name)
	{
		Node* func = new Node{rgFunction};
		func->children.reserve(4);

		// no visibility, using the default one
		// // visibility: public
		// // funcGetNode.children.push_back(new Node{rgVisibility, "public"});

		// function-kind
		// |   function-kind-function (+2)
		// |	   symbol-name
		// |		   identifier: main
		Node::Ptr funcKind = new Node{rgFunctionKind};
		func->children.push_back(funcKind);
		{
			Node::Ptr funcKindFunc = new Node{rgFunctionKindFunction};
			funcKind->children.push_back(funcKindFunc);
			{
				Node::Ptr symname = new Node{rgSymbolName};
				funcKindFunc->children.push_back(symname);

				symname->children.push_back(createNodeIdentifier(name, true));
			}
		}
		return func;
	}


	Node* createNodeFuncCrefParam(const AnyString& name, const AnyString& paramname)
	{
		auto* func = createNodeFunc(name);

		// func-params (+3)
		// |   func-param (+2)
		// |   |   cref
		// |   |   identifier: rhs
		Node::Ptr params = new Node{rgFuncParams};
		func->children.push_back(params);

		Node::Ptr firstParam = new Node{rgFuncParam};
		params->children.push_back(firstParam);

		firstParam->children.push_back(new Node{rgCref});
		firstParam->children.push_back(createNodeIdentifier(paramname, true));
		return func;
	}





} // namespace AST
} // namespace Nany
