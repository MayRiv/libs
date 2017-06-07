#pragma once

#include "xtypes.hpp"
using namespace lb;

class PreCalcRand
{
public:
	static void Init(const Byte countBits);
	static Word32 GetRand()
	{
		return _rndValues[(++_curVal) & _countRndValue];
	}
	static void ReCalc();
private:
	static Word32 *_rndValues;
	static Word32 _countRndValue;
	static Word32 _curVal;
};
