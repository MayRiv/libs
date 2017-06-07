#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vararray.hpp"
#include "storage.hpp"

using namespace lb;

VarArray::VarArray(int words)
{
	curWord = 0;
	maxWords = words;
	this->words = (TMinVar*)malloc(maxWords*sizeof(TMinVar));
}

VarArray::~VarArray()
{
	free(words);
}

/*
void VarArray::Add(Word32 src)
{
	TMinVar twords[MAX_TEMP_WORDS];
	int i = 0;
	
  for (; i < MAX_TEMP_WORDS; i++)
  {
  	twords[i] = src & 0x7F;
    if (!(src & (~0x7F))) // can put in byte
    	break;
  	src >>= 7;
  }
  
 	if (curWord + i + 1 >= maxWords)
  		Reallock(curWord+i+REALLOCK_BYTE);

  for (; i > 0; i--, curWord++)
  	words[curWord] = twords[i] | 0x80;
  	
  words[curWord] = twords[i];
  curWord++;
}
*/
void VarArray::Reallock(int newLength)
{
	TMinVar *newdata = (TMinVar*)realloc (words, newLength * sizeof(TMinVar));
	if (!newdata)
	{
		printf("Cannot realock data\n");
		throw ErrorReallock();
	}
	maxWords = newLength;		
	words = newdata;
}


void VarArray::Attach(class Storage &st, Word32 size)
{
	words = st.Attach(size);
	curWord = 0;
	maxWords = size;
}

void VarArray::ResizeAndDetach()
{
	Word16 newmaxWords = maxWords - curWord;
	TMinVar *newdata = 	(TMinVar*)malloc(newmaxWords*sizeof(TMinVar));
	memcpy(newdata, words+curWord, newmaxWords);
	words = newdata;
	maxWords = newmaxWords;
	curWord = 0;
}

