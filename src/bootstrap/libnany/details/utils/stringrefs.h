#pragma once
#include <yuni/yuni.h>
#include <yuni/core/dictionary.h>
#include <deque>



namespace Nany
{

	/*!
	** \brief Placeholder for storing duplicate strings
	*/
	class StringRefs final
	{
	public:
		//! Default constructor
		StringRefs();
		//! Copy constructor
		StringRefs(const StringRefs&) = delete;
		//! Destructor
		~StringRefs() = default;

		/*!
		** \brief Add a new entry within the catalog
		*/
		AnyString refstr(const AnyString& text);

		/*!
		** \brief Get the unique id of a string
		*/
		uint32_t ref(const AnyString& text);

		/*!
		** \brief Get if a given string is already indexed
		*/
		bool exists(const AnyString& text) const;


		//! Get if the container is empty
		bool empty() const;

		//! Clear the container
		void clear();


		void checkInternalPointer(const AnyString& name) const;

		size_t inspectMemoryUsage() const;


		//! \name Operators
		//@{
		//! Retrieve a stored string
		AnyString operator [] (uint32_t ix) const;
		//! Copy operator
		StringRefs& operator = (const StringRefs&) = delete;
		//@}


	private:
		//! Register string
		uint32_t keepString(const AnyString& text);

	private:
		struct StringImmutablePointer final
		{
			StringImmutablePointer() = default;
			StringImmutablePointer(const AnyString&);
			StringImmutablePointer(const StringImmutablePointer&) = delete;
			StringImmutablePointer(StringImmutablePointer&&);
			~StringImmutablePointer();
			StringImmutablePointer& operator = (const StringImmutablePointer&) = delete;
			StringImmutablePointer& operator = (StringImmutablePointer&&) = delete;
			AnyString toString() const;
			char* text = nullptr;
			uint32_t size = 0;
		};
		//! Index for all unique references (name <-> index)
		std::vector<StringImmutablePointer> pRefs;
		//! Index for fast retrieval
		std::unordered_map<AnyString, uint32_t> pIndex;

	}; // class StringRefs





} // namespace Nany

#include "stringrefs.hxx"
