#pragma once
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/fwd.h"
#include "libnany-config.h"




namespace Nany
{
namespace IR
{
namespace Producer
{

	inline Scope::Scope(Context& context)
		: pContext(context)
		, broadcastNextVarID(false)
	{}


	inline Scope::Scope(Scope& scope)
		: pContext(scope.pContext)
		, pNextVarID(scope.pNextVarID)
		, parentScope(&scope)
		, kind(scope.kind)
	{}


	inline Scope::~Scope()
	{
		if (broadcastNextVarID)
		{
			assert(parentScope != nullptr); // broadcast new values to the parent
			parentScope->pNextVarID = pNextVarID;
		}
	}


	inline Logs::Report& Scope::report()
	{
		return pContext.report;
	}


	inline Program& Scope::program()
	{
		return pContext.program;
	}


	inline AnyString Scope::acquireString(const AnyString& string)
	{
		return pContext.program.stringrefs.refstr(string);
	}


	inline bool Scope::visitAST(Node& node)
	{
		return visitASTStmt(node);
	}


	inline void Scope::comment(const AnyString& text)
	{
		program().emitComment(text);
	}


	inline void Scope::comment()
	{
		program().emitComment();
	}


	inline void Scope::addDebugCurrentPosition(uint line, uint offset)
	{
		if (pContext.debuginfo)
		{
			if (not Config::removeRedundantDbgOffset or offset != pContext.pPreviousDbgOffset or line != pContext.pPreviousDbgLine)
			{
				program().emitDebugpos(line, offset);
				pContext.pPreviousDbgOffset = offset;
				pContext.pPreviousDbgLine = line;
			}
		}
	}


	inline void Scope::addDebugCurrentFilename(const AnyString& filename)
	{
		program().emitDebugfile(filename);
	}


	inline void Scope::addDebugCurrentFilename()
	{
		addDebugCurrentFilename(pContext.dbgSourceFilename);
	}


	inline void Scope::resetLocalCounters(LVID localvarStart)
	{
		// 0: null
		// 1: reserved for namespace lookup
		pNextVarID = localvarStart;

		// force debug info
		pContext.pPreviousDbgOffset = 0;
		pContext.pPreviousDbgLine = 0;
	}

	inline uint32_t Scope::nextvar()
	{
		return ++pNextVarID;
	}

	inline uint32_t Scope::reserveLocalVariable()
	{
		return ++pNextVarID;
	}

	inline LVID Scope::createLocalBuiltinVoid(const Node& node)
	{
		emitDebugpos(node);
		return program().emitStackalloc(nextvar(), nyt_void);
	}


	inline LVID Scope::createLocalBuiltinAny(const Node& node)
	{
		emitDebugpos(node);
		return program().emitStackalloc(nextvar(), nyt_any);
	}


	inline LVID Scope::createLocalBuiltinFloat64(const Node& node, nytype_t type, double value)
	{
		emitDebugpos(node);
		return program().emitStackallocConstant(nextvar(), type, value);
	}


	inline LVID Scope::createLocalBuiltinInt64(const Node& node, nytype_t type, yuint64 value)
	{
		emitDebugpos(node);
		return program().emitStackallocConstant(nextvar(), type, value);
	}


	inline bool Scope::hasDebuginfo() const
	{
		return pContext.debuginfo;
	}


	inline bool Scope::hasIRDebuginfo() const
	{
		return pContext.debuginfoIR;
	}


	inline bool Scope::isWithinClass() const
	{
		Scope* scope = parentScope;
		while (scope != nullptr)
		{
			switch (scope->kind)
			{
				case Kind::kclass:
					return true;
				case Kind::kfunc:
					return false;
				case Kind::undefined:
					break;
			}

			// go to previous
			scope = scope->parentScope;
		}
		return false;
	}


	inline void Scope::setErrorFrom(Logs::Report& report, const Node& node) const
	{
		uint line, offset;
		fetchLineAndOffsetFromNode(node, line, offset);
		report.message.origins.location.pos.line = line;
		report.message.origins.location.pos.offset = offset;
	}


	inline void Scope::setErrorFrom(const Node& node) const
	{
		setErrorFrom(pContext.report, node);
	}


	inline void Scope::emitDebugpos(const Node* node)
	{
		if (node)
			emitDebugpos(*node);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
