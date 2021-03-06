#pragma once
#include "qualifiers.h"



namespace Nany
{

	inline void Qualifiers::merge(const Qualifiers& rhs)
	{
		if (rhs.nullable)
			nullable = true;

		if (rhs.constant)
			constant = true;

		if (rhs.ref)
			ref = true;
	}


	inline bool Qualifiers::operator == (const Qualifiers& rhs) const
	{
		return (constant == rhs.constant) and (ref == rhs.ref) and (nullable == rhs.nullable);
	}

	inline bool Qualifiers::operator != (const Qualifiers& rhs) const
	{
		return not (operator == (rhs));
	}



} // namespace Nany
