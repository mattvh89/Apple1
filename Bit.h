#pragma once
#include <string>
#include <sstream>
#include <cstdlib>


#define BIT_VALUE(X) 1 << (X - 1)

enum Hex
{
	A = 10, B, C, D, E, F
};

using Bit     = bool;
using Byte    = uint8_t;
using Word    = uint16_t;
using DWord   = uint32_t;
using QWord   = uint64_t;
using S_Byte  = int8_t;
using S_Word  = int16_t;
using S_DWORD = int32_t;
using S_QWord = int64_t;

// Get the count of the last bit, i.e. 1, 8, 16, 32, 64
template<typename Data>
constexpr Byte LastBit = sizeof(Data) * 8;

template<typename Data>
class Bits
{
public:
	Bits()
		: m_num(0) {	}

	Bits(const Data& t)
		: m_num(t) {	}

	Bits& operator=(const Data& data)
	{
		m_num = data;
		return *this;
	}

	Data operator++()
	{
		return ++m_num;
	}

	Data operator++(int)
	{
		Data d = m_num++;
		return d;
	}

	Data operator--()
	{
		return --m_num;
	}

	Data operator--(int)
	{
		Data d = m_num--;
		return d;
	}

	Data operator+(const Data& other)
	{
		return m_num + other;
	}

	inline Data& get()
	{
		return m_num;
	}

	inline Data getCopy() const
	{
		return m_num;
	}

	inline void assign(const Data& t)
	{
		m_num = t.m_num;
	}

	inline Bits<Data>& And(const Data& t)
	{
		m_num &= t;
		return *this;
	}

	inline Bits<Data>& And(const Bits<Data>& t)
	{
		m_num &= t.getCopy();
		return *this;
	}

	inline Bits<Data>& Nand(const Data& t)
	{
		m_num &= ~t;
		return *this;
	}

	inline Bits<Data>& Or(const Data& t)
	{
		m_num |= t;
		return *this;
	}

	inline Bits<Data>& Xor(const Data& t)
	{
		m_num ^= t;
		return *this;
	}

	inline Bits<Data>& Las(const size_t& x = 1)
	{
		m_num = (m_num << x) & ((1 << (sizeof(Data) * 8)) - 1);
		return *this;
	}

	inline Bits<Data>& Ras(const size_t& x = 1)
	{
		m_num = (m_num >> x) & ((1 << (sizeof(Data) * 8)) - 1);
		return *this;
	}

	inline Data AND(const Data& t) const
	{
		return m_num & t;
	}

	inline Data OR(const Data& t) const
	{
		return m_num | t;
	}

	inline std::string ToBinaryString(const size_t& spacingIndex = 8) const
	{
		return Bits<Data>::ToBinaryString(m_num, spacingIndex);
	}

	inline size_t BitCount(bool ones = true) const
	{
		return Bits<Data>::BitCount(m_num, ones);
	}

	inline void SetBit(const size_t& bit)
	{
		return Bits<Data>::SetBit(m_num, bit);
	}

	inline void SetAll()
	{
		return Bits<Data>::SetAll(m_num);
	}

	inline void ClearBit(const size_t& bit)
	{
		return Bits<Data>::ClearBit(m_num, bit);
	}

	inline void ClearAll(const size_t& bit)
	{
		return Bits<Data>::ClearAll(m_num, bit);
	}

	inline void ToggleBit(const size_t& bit)
	{
		return Bits<Data>::ToggleBit(m_num, bit);
	}

	inline bool CheckBit(const size_t& bit)
	{
		return Bits<Data>::CheckBit(m_num, bit);
	}

	inline void RotateLeft(const size_t& bit = 1)
	{
		return Bits<Data>::RotateLeft(m_num, bit);
	}

	inline void RotateRight(const size_t& bit = 1)
	{
		return Bits<Data>::RotateRight(m_num, bit);
	}

public: // Static functions
	static inline std::string ToBinaryString(const Data& data, const size_t& spacingIndex = 8)
	{
		std::stringstream ss;						// Input it all into a string stream in reverse order
		Data value = BitValue(LastBit<Data>);		// Value of the last bit for the particular size
		for (size_t i = 0; i < LastBit<Data>; ++i)	// Looping from 0 to LastBit but really starting last bit and wroking down
		{
			ss << ((data & value) ? '1' : '0');		// 1 or 0
			if (((i+1) % spacingIndex == 0) and (i != LastBit<Data> - 1) and i != 0) ss << ' ';	// add a space at desired location
			value = value >> 1;						// Right shift our value to the next bit
		}
		return ss.str();
	}

	static inline Data BitValue(const size_t& bit)
	{
		return static_cast<Data>(1 << (bit - 1));
	}

	static inline size_t BitCount(const Data& data, bool ones = true)
	{
		size_t count = 0;									// counts the quantiy of bit we want
		for (size_t i = 1,									// i will be the check bit
					j = 0; j != LastBit<Data>; ++j)			// j with the be iterator
		{
			if (ones)										// if we want ones, default argument
				count += (data & i) ? 1 : 0;
			else
				count += (!(data & i)) ? 1 : 0;				// if we want zeros
			i = i << 1;										// shift left to the next bit
		}
		return count;
	}

	static inline void SetBit(Data& data, const size_t& bit)
	{
		data |= BitValue(bit);
	}

	static inline void SetAll(Data& data)
	{
		data = ~static_cast<Data>(0);
	}

	static inline void ClearBit(Data& data, const size_t& bit)
	{
		data &= ~BitValue(bit);
	}

	static inline void ClearAll(Data& data)
	{
		data ^= data;
	}

	static inline void ToggleBit(Data& data, const size_t& bit)
	{
		data ^= BitValue(bit);
	}

	static inline bool CheckBit(const Data& data, const size_t& bit)
	{
		return data & BitValue(bit);
	}

	static inline void RotateLeft(Data& data, const size_t& times)
	{
		for (size_t i = 0; i < times; ++i)
		{
			bool carry = data & BitValue(LastBit<Data>);
			data = data << 1;
			data += carry ? 1 : 0;
		}
	}

	static inline void RotateRight(Data& data, const size_t& times)
	{
		for (size_t i = 0; i < times; ++i)
		{
			bool carry = data & 1;
			data = data >> 1;
			data += carry ? BitValue(LastBit<Data>) : 0;
		}
	}

	static inline void Las(Data& data, const size_t& times = 1)
	{
		data = data << times;
	}

	static inline void Ras(Data& data, const size_t& times = 1)
	{
		data = data >> times;
	}

private:
	Data m_num;
};
