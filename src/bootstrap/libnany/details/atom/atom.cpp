#include "atom.h"
#include "details/ir/program.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"

using namespace Yuni;





namespace Nany
{

	Atom::Atom(const AnyString& name, Atom::Type type)
		: type(type)
		, name(name)
	{}


	Atom::Atom(Atom& rootparent, const AnyString& name, Atom::Type type)
		: type(type)
		, parent(&rootparent)
		, name(name)
	{
		rootparent.pChildren.insert(std::pair<AnyString, Atom::Ptr>(name, this));
	}


	Atom::~Atom()
	{
		assert(instances.size() == pSymbolInstances.size());
		if (opcodes.owned)
			delete opcodes.program;
	}


	uint32_t Atom::findInstance(IR::Program*& program, const Signature& signature)
	{
		auto it = pInstancesBySign.find(signature);
		if (it != pInstancesBySign.end())
		{
			program = it->second.second;
			return it->second.first;
		}
		return (uint32_t) -1;
	}


	bool Atom::canAccessTo(const Atom& atom) const
	{
		// same ancestor ? requesting access to a namespace ?
		if ((parent == atom.parent) or (atom.type == Type::namespacedef))
			return true;

		// get if the targets are identical
		if (origin.target == atom.origin.target)
		{
			// determining whether the current atom is an ancestor of the provided one
			// TODO fix accessibility (with visibility)
			return true;
		}
		else
		{
			// not on the same target ? Only public elements are accessible
			return atom.isPublicOrPublished();
		}
		return false; // fallback
	}


	void Atom::printFullname(Yuni::String& out, bool clearBefore) const
	{
		if (clearBefore)
			out.clear();

		if (parent)
		{
			if (parent->parent)
			{
				parent->printFullname(out, false);
				out += '.';
			}

			if (name.first() != '^')
			{
				out += name;
			}
			else
			{
				if (name.startsWith("^default-var-%"))
				{
					AnyString sub{name.c_str() + 14, name.size() - 14};
					auto ix = sub.find('-');

					if (ix < sub.size())
					{
						++ix;
						AnyString varname{sub.c_str() + ix, sub.size() - ix};
						out << varname;
					}
					else
						out << "<invalid-field>";
				}
				else
					out.append(name.c_str() + 1, name.size() - 1);
			}
		}
	}


	template<class T>
	void Atom::doAppendCaption(YString& out, const T* table) const
	{
		printFullname(out, false);

		switch (type)
		{
			case Type::funcdef:
			{
				if (name.startsWith("^default-var-%")) // keep it simple
					break;

				out << '(';
				bool first = true;

				for (uint i = 0; i != parameters.size(); ++i)
				{
					AnyString paramname = parameters.name(i);

					// avoid the first virtual parameter
					if (i == 0 and paramname == "self")
						continue;

					if (not first)
						out << ", ";
					first = false;

					out << paramname;

					const Vardef& vardef = parameters.vardef(i);
					if (table)
					{
						if (table) // and table->hasClassdef(vardef.clid))
						{
							out << ": ";
							table->classdef(vardef.clid).print(out, *table, false);
						}
						else
							out << ": void";
					}
					else
					{
						if (not vardef.clid.isVoid())
							out << ": any";
						else
							out << ": void";
					}
				}
				out << ')';

				if (table)
				{
					if (not returnType.clid.isVoid() and table->hasClassdef(returnType.clid))
					{
						out.write(": ", 2);
						table->classdef(returnType.clid).print(out, *table, false);
						break;
					}
				}
				else
				{
					out << ((returnType.clid.isVoid()) ? ": void" : ": ref");
					break;
				}

				// no return type
				out.write(": void", 6);
				break;
			}

			case Type::classdef:
			case Type::namespacedef:
			case Type::vardef:
				break;
		}
	}


	void Atom::appendCaption(YString& out) const
	{
		doAppendCaption<ClassdefTableView>(out, nullptr);
	}

	void Atom::appendCaption(YString& out, const ClassdefTableView& table) const
	{
		doAppendCaption(out, &table);
	}



	inline void Atom::doPrint(Logs::Report& report, const ClassdefTableView& table, uint depth) const
	{
		auto entry = report.trace();
		for (uint i = depth; i--; )
			entry.message.prefix << "    ";

		if (parent != nullptr)
		{
			entry.message.prefix << table.keyword(*this) << ' ';
			appendCaption(entry.data().message, table);
			entry << " [id:" << atomid;
			if (isMemberVariable())
				entry << ", field: " << varinfo.effectiveFieldIndex;
			entry << ']';
		}
		else
			entry << "{global namespace}";


		++depth;
		for (auto& child: pChildren)
			child.second->doPrint(report, table, depth);

		if (Type::classdef == type)
			report.trace(); // for beauty
	}


	void Atom::print(Logs::Report& report, const ClassdefTableView& table) const
	{
		doPrint(report, table, 0);
	}


	bool Atom::performNameLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name)
	{
		assert(not name.empty());

		// the current scope
		Atom& scope = (nullptr == scopeForNameResolution) ? *this : *scopeForNameResolution;

		if (scope.isFunction() and name == "^()") // shorthand for function calls
		{
			list.push_back(std::ref(scope));
			return true;
		}

		bool success = false;
		scope.eachChild(name, [&](Atom& child) -> bool
		{
			list.push_back(std::ref(child));
			success = true;
			return true; // let's continue
		});

		return success;
	}


	bool Atom::performNameLookupFromParent(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name)
	{
		assert(not name.empty());

		// the current scope
		Atom& scope = (nullptr == scopeForNameResolution) ? *this : *scopeForNameResolution;

		// rule: do not continue if there are some matches and if the scope is a class
		// this is to avois spurious name resolution:
		//
		//     func foo() -> 42;
		//
		//     class SomeClass
		//     {
		//         func foo() -> 12;
		//         func bar -> foo(); // must resolve to `SomeClass.foo`, and not `foo`
		//     }
		//

		if (scope.parent)
		{
			// should start from the very bottom
			bool askToParentFirst = not scope.isClass();;
			if (askToParentFirst)
			{
				if (scope.parent->performNameLookupFromParent(list, name))
					return true;
			}

			// try to resolve locally
			bool found = scope.performNameLookupOnChildren(list, name);

			if (not found and (not askToParentFirst)) // try again
				found = scope.parent->performNameLookupFromParent(list, name);

			return found;
		}
		else
		{
			// try to resolve locally
			return scope.performNameLookupOnChildren(list, name);
		}

		return false;
	}


	uint32_t Atom::assignInvalidInstance(const Signature& signature)
	{
		pInstancesBySign.insert(std::make_pair(signature, std::make_pair((uint32_t) -1, nullptr)));
		return (uint32_t) -1;
	}


	uint32_t Atom::assignInstance(const Signature& signature, IR::Program* program, const AnyString& symbolname)
	{
		assert(program != nullptr);
		uint32_t iid = (uint32_t) instances.size();
		instances.emplace_back(program);
		pSymbolInstances.emplace_back(symbolname);
		pInstancesBySign.insert(std::make_pair(signature, std::make_pair(iid, program)));
		assert(instances.size() == pSymbolInstances.size());
		return iid;
	}


	void Atom::printInstances(Clob& out, const AtomMap& atommap) const
	{
		assert(instances.size() == pSymbolInstances.size());
		Clob prgm;

		for (size_t i = 0; i != instances.size(); ++i)
		{
			out << pSymbolInstances[i] << " // " << atomid << " #" << i << "\n{\n";

			instances[i].get()->print(prgm, &atommap);
			prgm.replace("\n", "\n    ");
			prgm.trimRight();
			out << prgm << "\n}\n";
		}
	}


	uint32_t Atom::findInstanceID(const IR::Program& program) const
	{
		for (size_t i = 0; i != instances.size(); ++i)
		{
			if (&program == instances[i].get())
				return static_cast<uint32_t>(i);
		}
		return (uint32_t) -1;
	}


	AnyString Atom::findInstanceCaption(const IR::Program& program) const
	{
		for (size_t i = 0; i != instances.size(); ++i)
		{
			if (&program == instances[i].get())
				return pSymbolInstances[i];
		}
		return AnyString{};
	}


	const Atom* Atom::findChildByMemberFieldID(uint32_t field) const
	{
		const Atom* ret = nullptr;
		eachChild([&](const Atom& child) -> bool
		{
			if (child.varinfo.fieldindex == field)
			{
				ret = &child;
				return false;
			}
			return true; // let's continue
		});
		return ret;
	}


	AnyString Atom::keyword() const
	{
		switch (type)
		{
			case Type::funcdef:
			{
				return (name[0] != '^')
					? "func"
					: ((not name.startsWith("^default-var-%"))
						? "operator"
						: "<default-init>");
			}
			case Type::classdef:     return "class";
			case Type::namespacedef: return "namespace";
			case Type::vardef:       return "var";
		}
		return "auto";
	}


	uint32_t Atom::findFuncAtom(Atom*& out, const AnyString& name)
	{
		// first, try to find the dtor function
		Atom* atomA = nullptr;
		uint32_t count = 0;

		eachChild([&](Atom& child) -> bool
		{
			if (child.isFunction())
			{
				if (child.name == name)
				{
					if (likely(atomA == nullptr))
					{
						atomA = &child;
					}
					else
					{
						count = 2;
						return false;
					}
				}
			}
			return true;
		});

		if (count != 0)
			return count;

		out = atomA;
		return atomA ? 1 : 0;
	}


	uint32_t Atom::findVarAtom(Atom*& out, const AnyString& name)
	{
		// first, try to find the dtor function
		Atom* atomA = nullptr;
		uint32_t count = 0;

		eachChild([&](Atom& child) -> bool
		{
			if (child.isMemberVariable())
			{
				if (child.name == name)
				{
					if (likely(atomA == nullptr))
					{
						atomA = &child;
					}
					else
					{
						count = 2;
						return false;
					}
				}
			}
			return true;
		});

		if (count != 0)
			return count;

		out = atomA;
		return atomA ? 1 : 0;
	}


	void Atom::renameChild(const AnyString& from, const AnyString& to)
	{
		Ptr child;
		auto range = pChildren.equal_range(from);
		for (auto it = range.first; it != range.second; )
		{
			if (unlikely(!!child)) // error
				return;
			child = it->second;
			it = pChildren.erase(it);
		}

		child->name = to;
		pChildren.insert(std::pair<AnyString, Atom::Ptr>(to, child));
	}




} // namespace Nany
