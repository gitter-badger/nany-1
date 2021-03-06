#pragma once
#include <yuni/yuni.h>
#include "details/fwd.h"




namespace Nany
{
namespace Sema
{

	class Metadata final
	{
	public:
		//! Create a new metadata object
		static Metadata* create(Nany::Node* originalNode);
		//! Create a copy of a metadata object
		static Metadata* clone(const Metadata&);
		//! Delete a metadata object
		static void release(Metadata* ptr);

		//! Initialize metadata handlers
		static void initialize();


	public:
		//! Reset the metadata
		void reset();


	public:
		//! Parent node (weak pointer)
		Nany::Node* parent = nullptr;
		//! Node from the native
		bool fromASTTransformation = true;

		//! The original node, from the immutable AST
		const Nany::Node* originalNode = nullptr;


	private:
		//! Default constructor
		Metadata(Nany::Node* originalNode);
		//! Copy constructor
		Metadata(const Metadata&) = default;
		//! Destructor
		~Metadata() = default;

	}; // class Metadata





} // namespace Sema
} // namespace Nany
