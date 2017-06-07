#include <stdlib.h>
#include "rand.hpp"
#include "log.hpp"

Word32 *PreCalcRand::_rndValues = NULL;
Word32 PreCalcRand::_countRndValue = 0;
Word32 PreCalcRand::_curVal = 0;

void PreCalcRand::Init(const Byte countBits)
{
	for (Byte i = 0; i < countBits; i++)
		_countRndValue |= (1<<i);
	L(LOG_WARNING, "Begin init rnd on 0x%x values\n", _countRndValue);
	_rndValues = new Word32[_countRndValue];
	ReCalc();
}

void PreCalcRand::ReCalc()
{
	for (Word32 i = 0; i < _countRndValue; i++)
		_rndValues[i] = rand();
}
