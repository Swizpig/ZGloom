#pragma once

#include <cstdint>

class Quick
{
	public:

	void SetVal(int32_t v) { val = v; };
	void SetInt(int32_t v) { val = v<<16; };
	int32_t GetVal() { return val; };
	int16_t GetInt() { return val >> 16; };
	uint16_t GetFrac() { return val & 0xFFFF; };

	Quick operator + (Quick const &obj) 
	{
		Quick t;
		t.val =  val + obj.val;
		return t; 
	}

	Quick operator - (Quick const &obj)
	{
		Quick t;
		t.val = val - obj.val;
		return t;
	}

	Quick operator * (Quick const &obj)
	{
		Quick t;
		int64_t t64 = obj.val;
		t64 *= val;
		t64 >>= 16;
		t.val = static_cast<int32_t>(t64);
		return t;
	}

	Quick operator / (Quick const &obj)
	{
		Quick t;
		int64_t t64 = val;
		t64 <<= 16;
		t64 /= obj.val;
		t.val = static_cast<int32_t>(t64);
		return t;
	}

	bool operator < (Quick const &obj)
	{
		return val < obj.val;
	}

	private:
		int32_t val;

};