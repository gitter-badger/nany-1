#pragma once
#include <yuni/yuni.h>
#include <nany/nany.h>
#include "details/fwd.h"
#include "opcodes.h"
#include <iosfwd>


// forward declaration
namespace Nany { class AtomMap; namespace IR { class Program; class Instruction; } }


namespace Nany
{
namespace IR
{
namespace ISA
{

	template<enum Op> struct Operand final {};


	template<> struct Operand<Op::nop> final
	{
		constexpr static const char* opname() { return "nop"; }
		uint32_t opcode;
	};


	template<> struct Operand<Op::fieldget> final
	{
		constexpr static const char* opname() { return "fieldget"; }
		uint32_t opcode;
		uint32_t lvid; // dest pointer
		uint32_t self;
		uint32_t var;
	};

	template<> struct Operand<Op::fieldset> final
	{
		constexpr static const char* opname() { return "fieldset"; }
		uint32_t opcode;
		uint32_t lvid; // value
		uint32_t self;
		uint32_t var;
	};



	template<> struct Operand<Op::stacksize> final
	{
		constexpr static const char* opname() { return "stacksize"; }
		uint32_t opcode;
		uint32_t add;
	};


	template<> struct Operand<Op::comment> final
	{
		constexpr static const char* opname() { return "comment"; }
		uint32_t opcode;
		uint32_t text;
	};

	template<> struct Operand<Op::stackalloc> final
	{
		constexpr static const char* opname() { return "stackalloc"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t type;    // nytype_t
		uint32_t atomid;  // atom id if (type == any)
	};

	template<> struct Operand<Op::storeConstant> final
	{
		constexpr static const char* opname() { return "storeConstant"; }
		uint32_t opcode;
		uint32_t lvid;
		union { uint64_t u64; double f64; } value;
	};

	template<> struct Operand<Op::storeText> final
	{
		constexpr static const char* opname() { return "storeText"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t text;
	};


	template<> struct Operand<Op::store> final
	{
		constexpr static const char* opname() { return "store"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t source;
	};

	template<> struct Operand<Op::ret> final
	{
		constexpr static const char* opname() { return "ret"; }
		uint32_t opcode;
		uint32_t lvid;
	};

	template<> struct Operand<Op::push> final
	{
		constexpr static const char* opname() { return "push"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t name; // if named parameter
	};

	template<> struct Operand<Op::call> final
	{
		constexpr static const char* opname() { return "call"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t ptr2func; // or atomid
		uint32_t instanceid;
	};

	template<> struct Operand<Op::intrinsic> final
	{
		constexpr static const char* opname() { return "intrinsic"; }
		uint32_t opcode;
		uint32_t lvid;
		//! intrinsic name when compiling, id when running
		uint32_t intrinsic;
	};

	template<> struct Operand<Op::debugfile> final
	{
		constexpr static const char* opname() { return "debugfile"; }
		uint32_t opcode;
		uint32_t filename;
	};

	template<> struct Operand<Op::namealias> final
	{
		constexpr static const char* opname() { return "namealias"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t name;
	};


	template<> struct Operand<Op::debugpos> final
	{
		constexpr static const char* opname() { return "debugpos"; }
		uint32_t opcode;
		uint32_t line;
		uint32_t offset;
	};

	template<> struct Operand<Op::scope> final
	{
		constexpr static const char* opname() { return "scope"; }
		uint32_t opcode;
	};

	template<> struct Operand<Op::end> final
	{
		constexpr static const char* opname() { return "end"; }
		uint32_t opcode;
	};


	template<> struct Operand<Op::qualifiers> final
	{
		constexpr static const char* opname() { return "qualifiers"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t qualifier; // 1: ref, 2: const
		uint32_t flag;
	};


	template<> struct Operand<Op::inherit> final
	{
		constexpr static const char* opname() { return "inherit"; }
		uint32_t opcode;
		//! 1: type (unused), 2: qualifiers
		uint32_t inherit;
		uint32_t lhs;
		uint32_t rhs;
	};



	template<> struct Operand<Op::memalloc> final
	{
		constexpr static const char* opname() { return "memalloc"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t regsize;
	};


	template<> struct Operand<Op::memfree> final
	{
		constexpr static const char* opname() { return "memfree"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t regsize;
	};





	template<> struct Operand<Op::pragma> final
	{
		constexpr static const char* opname() { return "pragma"; }
		uint32_t opcode;
		uint32_t pragma;

		union
		{
			uint32_t codegen;
			uint32_t error;
			uint32_t namespacedef;
			uint32_t visibility;
			uint32_t blueprintsize;
			struct { uint32_t name; uint32_t atomid; } blueprint;
			struct { uint32_t name; uint32_t lvid;   } param;
			struct { uint32_t name; uint32_t lvid;   } vardef;
		}
		value;
	};


	template<> struct Operand<Op::self> final
	{
		constexpr static const char* opname() { return "self"; }
		uint32_t opcode;
		uint32_t self;
	};


	template<> struct Operand<Op::identify> final
	{
		constexpr static const char* opname() { return "identify"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t self;
		uint32_t text;
	};

	template<> struct Operand<Op::assign> final
	{
		constexpr static const char* opname() { return "assign"; }
		uint32_t opcode;
		uint32_t lhs;
		uint32_t rhs;
		uint32_t disposelhs;
	};


	template<> struct Operand<Op::follow> final
	{
		constexpr static const char* opname() { return "follow"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t follower;
		uint32_t symlink;
	};

	template<> struct Operand<Op::typeisobject> final
	{
		constexpr static const char* opname() { return "typeisobject"; }
		uint32_t opcode;
		uint32_t lvid;
	};

	template<> struct Operand<Op::classdefsizeof> final
	{
		constexpr static const char* opname() { return "classdefsizeof"; }
		uint32_t opcode;
		uint32_t lvid;
		uint32_t type; // lvid or atomid
	};

	template<> struct Operand<Op::ref> final
	{
		constexpr static const char* opname() { return "ref"; }
		uint32_t opcode;
		uint32_t lvid;
	};

	template<> struct Operand<Op::unref> final
	{
		constexpr static const char* opname() { return "unref"; }
		uint32_t opcode;
		uint32_t lvid;

		// atomid
		uint32_t atomid; // or atomid
		// destructor to call
		uint32_t instanceid;
	};


	template<> struct Operand<Op::allocate> final
	{
		constexpr static const char* opname() { return "allocate"; }
		uint32_t opcode;
		uint32_t lvid;
		// atomid
		uint32_t atomid; // or atomid
	};


	template<> struct Operand<Op::dispose> final
	{
		constexpr static const char* opname() { return "dispose"; }
		uint32_t opcode;
		uint32_t lvid;

		// atomid
		uint32_t atomid; // or atomid
		// destructor to call
		uint32_t instanceid;
	};









	Yuni::String print(const Program&, const Nany::IR::Instruction&, const AtomMap* = nullptr);

	template<enum Op O>
	inline Yuni::String print(const Program& program, const Nany::IR::ISA::Operand<O>& operands, const AtomMap* map = nullptr)
	{
		return print(program, reinterpret_cast<const Nany::IR::Instruction&>(operands), map);
	}



} // namespace ISA
} // namespace IR
} // namespace Nany
