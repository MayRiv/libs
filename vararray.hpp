#pragma once

#include "xtypes.hpp"

namespace lb
{
	const int REALLOCK_BYTE = 4;

	typedef Byte TMinVar; // NOW ONLY BYTE !!!!!!!!

	class VarArray
	{
	public:
		VarArray(int words);
		VarArray()
		{
			words = NULL;
			maxWords = 0;
			curWord = 0;
		}
		~VarArray();
		inline void Add(Word32 number);
		
		inline Word32 Get();
		Word32 Size() { return curWord; }
		Word32 MaxSize() { return maxWords; }
		void Start()
		{
			curWord = 0;
		}
		void SetSeek(const Word32 seek)
		{
			if (seek < maxWords)
				curWord = seek;
		}
		int NotEnd() 
		{
			if (curWord < maxWords)
				return 1;
			else
				return 0;
		}
		
		TMinVar *Data() { return words; }
		void Attach(class Storage &st, Word32 size);
		void Detach()
		{
			words = NULL;
			maxWords = 0;
		}
		void ResizeAndDetach();
		void ReAttach(VarArray &src)
		{
			words = src.words;
			maxWords = src.maxWords;
			src.Detach();
		}
		bool Cmp(VarArray &b)
		{
			if (Size() != b.Size())
				return false;
			for (Word16  i = 0; i < Size(); i++)
				if (words[i] != b.words[i])
					return false;
			return true;
		}

		class ErrorReallock {}; // for exception only
	protected:
		void Reallock(int newLength);
		TMinVar *words;
		Word16 curWord;
		Word16 maxWords;
	};

	inline bool operator ==(VarArray &a, VarArray &b)
	{
		return a.Cmp(b);
	}

	const int MAX_TEMP_WORDS = 5;

	inline void VarArray::Add(Word32 src)
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

	inline Word32 VarArray::Get()
	{
		Word32 data = 0;
		while (curWord < maxWords)
		{
			if (words[curWord] & 0x80)
  			data |= words[curWord] & 0x7F;
			else
			{
				data |= words[curWord];
				curWord++;
				break;
			}
  		data <<= 7;
  		curWord++;
		}
		return data;
	}
};

