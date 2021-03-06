#pragma once
#include <yuni/yuni.h>
#include "classdef.h"
#include "details/utils/clid.h"
#include <vector>
#include <memory>




namespace Nany
{

	// forward declaration
	class Atom;




	class Signature final
	{
	public:
		struct Paramtype final
		{
			//! Kind of the parameter
			nytype_t kind = nyt_any;
			//! Atom attached to it, if any (kind == nyt_any)
			Atom* atom = nullptr;
			//! Qualifiers (ref, const...)
			Qualifiers qualifiers;

			bool operator == (const Paramtype& rhs) const;
			bool operator != (const Paramtype& rhs) const;
		};

		struct Parameters final
		{
			//! Reserve the number of parameters
			void resize(uint count);
			//! The total number of parameters
			uint size() const;
			//! Get if empty
			bool empty() const;

			const Paramtype& operator [] (uint32_t index) const;
			Paramtype& operator [] (uint32_t index);


			bool operator == (const Parameters& rhs) const;

		private:
			friend class Signature;
			void hash(size_t&) const;
			//! Each type for each parameter
			std::vector<Paramtype> pParamtypes;
		};


	public:
		//! Get the hash of this singature
		size_t hash() const;

		bool operator == (const Signature& rhs) const;
		bool operator != (const Signature& rhs) const;

	public:
		//! The return type
		Classdef returnType;

		//! Function parameters (func (a, b , c))
		Parameters parameters;
		//! Template parameters
		Parameters tmplparams;

	}; // class Signature





} // namespace Nany

#include "signature.hxx"
