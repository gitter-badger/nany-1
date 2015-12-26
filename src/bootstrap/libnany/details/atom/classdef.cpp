#include "details/atom/classdef.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/atom/atom.h"
#include "details/reporting/report.h"

using namespace Yuni;





namespace Nany
{

	/*static*/ const Classdef Classdef::nullcdef;



	Classdef::Classdef()
		: qualifiers()
	{
		// keep the symbol local
	}


	Classdef::Classdef(const CLID& clid)
		: kind(nyt_any)
		, clid(clid)
	{
	}


	template<class T, class TableT>
	inline void Classdef::doPrint(T& out, const TableT& table) const
	{
		// qualifiers
		{
			bool cIsConst = qualifiers.constant;
			bool cIsRef   = qualifiers.ref;

			if (cIsConst or cIsRef)
			{
				if (cIsConst and cIsRef)
					out << "cref ";
				else
					out << ((cIsConst) ? "const " : "ref ");
			}
		}


		const Atom* selfAtom = table.findClassdefAtom(*this);
		if (selfAtom)
		{
			if (kind == nyt_pointer)
				out << "ptr -> ";
			selfAtom->appendCaption(out, table);
		}
		else
			out << nany_type_to_cstring(kind);

		if (unlikely(qualifiers.nullable))
			out << '?';
	}


	void Classdef::print(Yuni::String& out, const ClassdefTableView& table, bool clearBefore) const
	{
		if (clearBefore)
			out.clear();
		doPrint(out, table);
	}


	void Classdef::print(Logs::Report& report, const ClassdefTableView& table) const
	{
		doPrint(report.data().message, table);
	}


	bool Classdef::isClass() const
	{
		return isLinkedToAtom() and (atom->type == Atom::Type::classdef);
	}


	bool Classdef::isPointerToFunc() const
	{
		return isLinkedToAtom() and (atom->type == Atom::Type::funcdef);
	}


	bool Classdef::isClass(const AnyString& name) const
	{
		return isClass() and (atom->name == name);
	}







} // namespace Nany