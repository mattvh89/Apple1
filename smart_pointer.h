/*	Smart Pointer template class
*
* Author - Matthew Van Helden
* Date - August 2023
*
*	This class is a facsimile of the shared_pointer STL class. It's goal is to automatically
* handle memory management for any type of data.
*
*	This class also acts as a smart pointer to an array that can be dynamically resized.
*
*	Using the smart pointer in a program you would still dereference it and use the arrow operator
* just like you would a normal pointer. Likewise, writing the name of the variable returns a pointer to your object.
*/

#ifndef SMART_POINTER_H
#define SMART_POINTER_H

#include <cstdint>
#include <stdexcept>
#include "Debug.h"

// Macro for easier typing
#define Ptr smart_pointer

#define ARRAY true

template<class T>
class smart_pointer
{
public:
	// Default Constructor
	smart_pointer()
		: _data(nullptr), _ref_count(nullptr), _size(0)
	{
	}

	// Create null pointers
	smart_pointer(bool array, const size_t& size)
		: _data(nullptr), _ref_count(nullptr), _size(size)
	{
		if (array)
		{
			_data = new T[size];
			DEBUG_OUT("Creating smart pointer to array");
		}
		else
		{
			// We're only creating the array if we want the array. Otherwise we want nullptrs
//			_data = new T;
			DEBUG_OUT("Creating new empty smart_pointer object");
		}
		_ref_count = new size_t(1);
		_data_memory += static_cast<uint64_t>(sizeof(T) * size);
		_total_memory += static_cast<uint64_t>(sizeof(T) * size + sizeof(size_t));
	}

	// Creating a smart_pointer with a new T argument, optional size
	smart_pointer(T* d, const size_t& size = 1)
		: _data(d), _ref_count(new size_t(1)), _size(size)
	{
		DEBUG_OUT("Creating Object: New T already created");
		DEBUG_OUT("\tReferences: " << *_ref_count);
		_data_memory += static_cast<uint64_t>(sizeof(T) * size);
		_total_memory += static_cast<uint64_t>(sizeof(T) * size + sizeof(size_t));
	}

	// Create smart_pointer from a regular value
	smart_pointer(const T& t)
		: _data(new T(t)), _ref_count(new size_t(1)), _size(1)
	{
		DEBUG_OUT("Creating new object for user");
		DEBUG_OUT("References: " << *_ref_count);
		_data_memory += static_cast<uint64_t>(sizeof(T) * _size);
		_total_memory += static_cast<uint64_t>(sizeof(T) * _size + sizeof(size_t));
	}

	// Copy Constructor
	smart_pointer(const smart_pointer<T>& sp)
		: _data(sp._data), _ref_count(sp._ref_count), _size(sp._size)
	{
		if (sp._data != nullptr)
		{
			DEBUG_OUT("Copy Constructor");
			++(*_ref_count);
			DEBUG_OUT("\tReferences: " << *_ref_count);
		}
	}

	// Move Constructor
	smart_pointer(smart_pointer<T>&& ptr)  noexcept
		: _data(ptr._data), _ref_count(ptr._ref_count), _size(ptr._size)
	{
		if (this != &ptr)
		{
			DEBUG_OUT("Move constructor called");
			ptr._data = nullptr;
			ptr._ref_count = nullptr;
			ptr._size = 0;
		}
	}

	// Destructor:
	// Check if there are no more references. If true delete, otherwise
	// dereference object
	~smart_pointer() {
		DEBUG_OUT("Destructor called");
		// If reference count is 0 after decremneting
		if (_ref_count && _data && (--(*_ref_count) == 0))
		{
			DEBUG_OUT("\tReferences: 0");
			DEBUG_OUT("\t\tDeleting memory");
			// Is it a single object
			if (_size == 1)
			{
				DEBUG_OUT("\t\t\tSingle object");
			}
			// Otherwise it's an array
			else
			{
				DEBUG_OUT("\t\t\t" << "Array of " << _size << " objects");
			}

			if (_size > 1)
				delete [] _data;
			else
				delete    _data;

			delete _ref_count;
			_data_memory  -= static_cast<uint64_t>(sizeof(T) * _size);
			_total_memory -= static_cast<uint64_t>(sizeof(T) * _size + sizeof(size_t));
			++_deletions;
		}
		else
		{
			if (_ref_count && *_ref_count)
			{
				DEBUG_OUT("\tReferences left: " << *_ref_count);
			}
			else
			{
				DEBUG_OUT("\tnullptr");
			}
		}
		_data = nullptr;
		_ref_count = nullptr;
		_size = 0;
	}

	// Move assignment operator
	smart_pointer<T>& operator=(smart_pointer<T>&& ptr) noexcept
	{
		DEBUG_OUT("Move assignment operator called");
		if (this != &ptr)
		{
			// Make sure this smart_pointer doesn't have any data. If it does, delete it and the reference
			// if the reference count is 0 after decremneting
			if (_ref_count && _data && --(*_ref_count) == 0)
			{
				DEBUG_OUT("\tDeleting old data");
				if (_data)
				{
					if (_size > 1) delete[] _data;
					else		   delete   _data;
				}
				delete _ref_count;
				_data = nullptr;
				_ref_count = nullptr;

				// extraneous information
				++_deletions;
				_data_memory -= static_cast<uint64_t>(sizeof(T) * _size);
				_total_memory -= static_cast<uint64_t>(sizeof(T) * _size + sizeof(int));
			}
			_data = ptr._data;
			_size = ptr._size;
			_ref_count = ptr._ref_count;

			ptr._data = nullptr;
			ptr._ref_count = nullptr;
			ptr._size = 0;
		}
		return *this;
	}

	// Copy Assignment operator
	smart_pointer<T>& operator=(const smart_pointer<T>& sp)
	{
		DEBUG_OUT("Assignment operator");
		if (this == &sp)
		{
			DEBUG_OUT("\tSame objects");
			return *this;
		}

		// Decrease the reference count of the current object
		// If reference count is then 0, delete everything and set to nullptr
		if (_ref_count && --(*_ref_count) == 0)
		{
			DEBUG_OUT("\tDeleting lvalue");
			if (_size > 1) delete [] _data;
			else		   delete    _data;
			delete _ref_count;
			_data = nullptr;
			_ref_count = nullptr;
			_data_memory -= static_cast<uint64_t>(sizeof(T) * _size);
			_total_memory -= static_cast<uint64_t>(sizeof(T) * _size + sizeof(size_t));
			_size = 0;
			++_deletions;
		}
		// If other data is not set
		if (sp._data == nullptr)
		{
			// Assin nullptrs
			DEBUG_OUT("\trvalue is nullptr");
			_data = nullptr;
			_ref_count = nullptr;
		}
		// else other has data, increment reference count and copy data
		else
		{
			DEBUG_OUT("\tAssigning new data");
			_data = sp._data;
			++(*sp._ref_count);
			_ref_count = sp._ref_count;
			_size = sp._size;
		}
		return *this;
	}

	// Resize:
	// @param:
	//		size_t - the amount by which to increase the array
	// @returns:
	//		size_t - the total size of the array
	size_t increaseBy(const size_t& size)
	{
		// If smart pointer doesn't point to any data, allocate an array sized size
		if (_data == nullptr)
		{
			DEBUG_OUT("Resizing uninitialized smart pointer");
			_data = new T[size];
			_ref_count = new size_t(1);
			_size = size;
			_data_memory += static_cast<uint64_t>(sizeof(T) * size);
			_total_memory += static_cast<uint64_t>(sizeof(T) * size + sizeof(size_t));
		}
		// Else smart pointer already holds data
		else
		{
			DEBUG_OUT("Resizing a smart pointer to an array");
			T* temp = new T[_size + size];
			for (size_t i = 0; i < _size; ++i)
			{
				*(temp + i) = *(_data + i);
			}
			if (_size > 1) delete[] _data;
			else		   delete   _data;
			_data = temp;
			_size += size;
			_data_memory += static_cast<uint64_t>(sizeof(T) * size);
			_total_memory += static_cast<uint64_t>(sizeof(T) * size);
		}
		DEBUG_OUT("\tIncreasing by " << size);
		return _size;
	}

	// Getter function for _data, returns raw pointer, probably shouldn't use this and so it probably shouldn't even be present but whatever
	inline T* data(void) const noexcept { return _data; }

	// Gets the size of the object. 1 is a single object, anything else is an array
	inline size_t size(void) const noexcept { return _size; }

	// Explicit function to check if _data is nullptr or points to data
	inline bool isSet(void) const noexcept { return _data == nullptr ? false : true; }

	// Static member returns the amount of memory used
	// @param: bool total
	//		if true return total memory used
	//		else return memory used by the data T iself in total
	static uint64_t memoryUsage(bool total = false)
	{
		if (total) return _total_memory;
		return _data_memory;
	}

	// Static function to check the number of times the desctructor
	// has actually deleted data
	static size_t deletions() { return _deletions; }

	// Access data members of T just like a normal pointer
	inline T* operator->(void) { return _data; }

	// Dereferencing the smart_pointer will return the value of the data
	// smart_pointer<int> ptr(15);
	// if(*ptr == 15) std::cout << "True";
	inline T& operator*(void) { return *_data; }

	// Simply referring to this smart_pointer itself should return a pointer
	// to the data just like a normal pointer
	inline operator T* () const { return _data; }

	// An explicit function to get a reference to this smart_pointer itself
	inline smart_pointer<T>& getReferenceToThis() const noexcept { return *this; }

	// An explicit function to get the memory address of this smart_pointer itself
	inline smart_pointer<T>* getAddressOfThis() const noexcept { return this; }

	// Check equality of the smart_pointer objects itself, since the smart_pointer
	// is supposed to act a normal pointer, regular equality should check the equality
	// of the data itself
	inline bool isEqualTo(const smart_pointer<T>& sp) const noexcept { return this == sp.getAddressOfThis(); }

	inline bool isNotEqual(const smart_pointer<T>& sp) const noexcept { return this != sp.getAddressOfThis(); }

	// Not operator checks not _data
	inline bool operator!(void) const noexcept { return !_data; }

	// Equality operator checks equality of _data so that smart_pointer functions as a normal pointer
	inline bool operator==(const smart_pointer<T>& sp) const noexcept { return _data == sp._data; }

	// Not Equality operator, same as above but inverse
	inline bool operator!=(const smart_pointer<T>& sp) const noexcept { return _data != sp._data; }

	// Index operator to allow indexing the pointer like a normal array
	inline T& operator[](const size_t& index) const
	{
		if (index > _size - 1) throw std::out_of_range("Out of range");
		return *(_data + index);
	}

	// pre-increment operator so one can create a pointer to an array and use that traverse it
	inline T& operator++() noexcept
	{
		++(*_data);
		return *_data;
	}

	// pre-decrement operator
	inline T& operator--()
	{
		--(*_data);
		return *_data;
	}

	// post-increment operator
	inline T& operator++(int)
	{
		T t = *_data;
		++_data;
		return t;
	}

	// post-decrement operator
	T operator--(int) {
		T t = *_data;
		--_data;
		return t;
	}

	//
	// 
	// Define an iterator class to be able to loop through all the elements of the array
	class Iterator
	{
	public:
		// Constructor
		Iterator(T* t) : ptr(t)
		{
		}

		// Copy Constructor
		Iterator& operator=(const Iterator& other)
		{
			ptr = other.ptr;
		}

		// dereference operator - dereferences the data to whic our poitner points
		inline T& operator*() const
		{
			return *ptr;
		}

		// increment operator
		Iterator& operator++()
		{
			++ptr;
			return *this;
		}

		// post incremnt operator
		Iterator& operator++(int)
		{
			Iterator temp = *this;
			++ptr;
			return temp;
		}

		// decremnet operator
		Iterator& operator--()
		{
			--ptr;
			return *this;
		}

		// post decremnt operator
		Iterator& operator--(int)
		{
			Iterator temp = *this;
			--ptr;
			return *this;
		}

		// Equality operator
		inline bool operator==(const Iterator& other) const noexcept
		{
			return ptr == other.ptr ? true : false;
		}

		// Inequality operator
		inline bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}

		// Less than operator
		inline bool operator<(const Iterator& other) const
		{
			return ptr < other.ptr;
		}

		// Greater than operator
		inline bool operator>(const Iterator& other) const
		{
			return ptr > other.ptr;
		}

	private:
		T* ptr;
	};

	// Define smart_pointer<T>::begin() function to get Iterator to first object
	Iterator begin()
	{
		return Iterator(_data);
	}

	// Define smart_pointer<T>::end() function to get where the array ends (_size is already last index + 1)
	Iterator end()
	{
		return Iterator(_data + _size);
	}

private:
	static uint64_t _data_memory, _total_memory;	// track the amount of memory we're using
	static size_t _deletions;						// Count how many smart_pointners have been deleted

	T* _data;										// The archaic pointer itself
	size_t* _ref_count;								// Count how many refrences there are to _data
	size_t _size;									// Determine whether it's a single object or array
};

// Static variables to keep track of accumalitve memory usage and memory usage of a single object
template<class T>
uint64_t smart_pointer<T>::_data_memory = 0;

template<class T>
uint64_t smart_pointer<T>::_total_memory = 0;

template<class T>
size_t smart_pointer<T>::_deletions = 0;

#endif
