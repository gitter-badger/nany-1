#pragma once
#include "classdef-table.h"



namespace Nany
{

	class ClassdefTableView final
	{
	public:
		ClassdefTableView(ClassdefTable&);
		ClassdefTableView(ClassdefTableView&);
		ClassdefTableView(ClassdefTableView&, LVID atomid, uint count);
		~ClassdefTableView();


		const Classdef& classdef(const CLID&) const;
		const Classdef& classdefFollowClassMember(const CLID&) const;
		const Classdef& rawclassdef(const CLID&) const;
		bool hasClassdef(const CLID& clid) const;

		Atom* findClassdefAtom(const Classdef& classdef) const;
		Atom* findRawClassdefAtom(const Classdef&) const;
		Match isSimilarTo(const CTarget* target, const Classdef& cdef, const Classdef& to, bool allowImplicit = false) const;

		/*!
		** \brief Get the keyword associated to an atom (class, func, var, cref, namespace...)
		** \see Nany::Atom::keyword()
		*/
		AnyString keyword(const Atom&) const;


		//! Get if the current view has a substitute for a given CLID
		bool hasSubstitute(const CLID&) const;
		//! Create a new substiture in the current layer
		Classdef& substitute(LVID);
		//! Append a new substitute
		Classdef& addSubstitute(nytype_t kind, Atom* atom, const Qualifiers& qualifiers);

		//! Get the atom id of the current layer
		LVID substituteAtomID() const;

		//! Resize the substitutes for the current layer
		void substituteResize(uint count);

		void mergeSubstitutes();

		//! \name Atoms
		//@{
		const AtomMap& atoms() const;
		AtomMap& atoms();
		//@}


		//! \name Debugging
		//@{
		//! Print all types
		void print(Yuni::String& out, bool clearBefore = true) const;
		//! Print a single type
		void printClassdef(Yuni::String& out, const CLID&, const Classdef&) const;
		//@}



	private:
		ClassdefTable& table;
		ClassdefTable::LayerItem previous;
		bool canSwap = false;
	};



} // namespace Nany

#include "classdef-table-view.hxx"
