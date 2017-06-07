#pragma once

#include "xtypes.hpp"
#include "string.hpp"
#include "exception.hpp"
#include <stdio.h>

namespace lb
{
	const Word32 MAX_STORAGE_SIZE = 100000000;  // 100 ��
	const float STORAGE_MUL_KOEF = 1.5;

	const int STORAGE_DEFAULT_ZLIB  = 9;

	const int STORAGE_ERROR_REALLOCK		= 1;
	const int STORAGE_ERROR_OUTOFRANGE	= 2;
	const int STORAGE_ERROR_IOERROR	= 3;
	const int _START_STORAGE_SIZE = 1000; // 1kb

	class Storage
	{
	public:
		Storage(const Storage &st);
		Storage(Word32 startSize = _START_STORAGE_SIZE);
		Storage(Byte *mem, Word32 size);
		Storage(Byte *commem, Word32 size, Word32 unpacksize);
		Storage(const char *gzFileName); // load from gz file
		~Storage();
		void Start();
		void Begin();
		int Unpack(Word32 unpackSize);
		Word32 Space(Word32 leng); // save space for feature write set
		
		Word32 Skip(Word32 leng);
		Word32 SkipString();
		int WriteToFile(FILE *file);
		int WriteToFile(int fD);

		int LoadFromFile(FILE *file, const Word32 leng);
		int LoadFromFile(int fD, const Word32 leng);
		int LoadFromGzFile(int fD, const Word32 uncompressedLeng);
		int AddFromFile(int fD, const Word32 leng);
		int AddFromFile(FILE *file, const Word32 leng);
		

		int LoadUnpack(Byte *comdata, Word32 packSize, Word32 unpackSize);

		Word32 CurPos()	const {	return curPOS; }
		void SetPos(Word32 setpos)
		{
			if (setpos > size )
				throw Error(STORAGE_ERROR_OUTOFRANGE);	
			pos = mem+setpos;
			curPOS = setpos;
		}
		void RollBack(Word32 setpos)
		{
			if (setpos > size )
				throw Error(STORAGE_ERROR_OUTOFRANGE);	
			pos = mem+setpos;
			curPOS = setpos;
			size = setpos;
		}
		Byte *GetPos()
		{
			return pos;
		}

		Word32 GetData(void *data, Word32 leng);
		Word32 GetData(Storage &st, const Word32 leng);

		Word32 AddData(const void *data, Word32 leng);
		Byte *AddAttach(Word32 leng);
		Word32 SetData(Storage &st);
		Word32 AddData(Storage &st);

		Word32 AddString(const String &str); // Add string max 255
		Word32 AddString(const RString &str); // Add RString max 255
		Word32 AddString(const char *str); // Add string max 255
		Word32 GetString(String &str);

		template<class D> Word32 AddString(const String &str)
		{
			if (!FitInStorage(str.GetLength() + sizeof(D)))
				throw Error(STORAGE_ERROR_REALLOCK);
			D strLen = str.GetLength();
			memcpy(pos, &strLen, sizeof(D));
			pos += sizeof(D);
			if (strLen)
			{
				memcpy(pos, str.c_str(), strLen);
				pos += strLen;
			}
			size += strLen + sizeof(D);
			return size;
		}
		template<class D> Word32 AddString(const char *str)
		{
			D strLen = strlen(str);
			if (!FitInStorage(strLen + sizeof(D)))
				throw Error(STORAGE_ERROR_REALLOCK);
			memcpy(pos, &strLen, sizeof(D));
			pos += sizeof(D);
			if (strLen)
			{
				memcpy(pos, str, strLen);
				pos += strLen;
			}
			size += strLen + sizeof(D);
			return size;
		}
		template<class D> Word32 GetString(String &str)
		{
			D leng = *((D*)pos);
			curPOS += sizeof(D);
			pos += sizeof(D);
			if (curPOS + leng > size )
				throw Error(STORAGE_ERROR_OUTOFRANGE);
			str.Set((char*)pos, (int)leng);
			
			pos += leng;
			curPOS += leng;
			return curPOS;
		}



		Word32 Size() { return size; }
		Byte *Compress(Word32 &destlen, int level = STORAGE_DEFAULT_ZLIB);
		Byte *Compress(int level = STORAGE_DEFAULT_ZLIB);
		Byte *Data()
		{
			return mem;
		}
		
		Word32 SaveToGzFile(char *fileName);
		
		void CopyData(Byte *src);

		template<class D> Word32 Add(const D &value)
		{
			if (!FitInStorage(sizeof(D)))
				throw Error(STORAGE_ERROR_REALLOCK);
			memcpy(pos, &value, sizeof(D));
			pos += sizeof(D);
			size += sizeof(D);
			return size;
		}

		template<class D> Word32 Set(Word32 pos, D value)
		{
				if (pos >= size)
					throw Error(STORAGE_ERROR_OUTOFRANGE);
				memcpy(mem+pos, &value, sizeof(D));
				return pos;
		}

		template<class D> Word32 Get(D *value)
		{
			if (curPOS  + sizeof(D) > size )
				throw Error(STORAGE_ERROR_OUTOFRANGE);
			//memcpy(value, pos, sizeof(D));
			*value = *((D*)pos);
			pos += sizeof(D);
			curPOS += sizeof(D);
			return curPOS;
		}

		template<class D> void GetFromPos(Word32 fromPos, D *value)
		{
			if (fromPos  + sizeof(D) > size )
				throw Error(STORAGE_ERROR_OUTOFRANGE);
			//memcpy(value, pos, sizeof(D));
			*value = *((D*)(mem+fromPos));
		}

		Byte *Attach(Word32 leng);
		Byte *Detach();

		void SetMaxMemmoryUsage(const Word32 reqMaxSize)
		{
			if (maxSize > reqMaxSize * STORAGE_MUL_KOEF)
			{
				Start();
				ReAllock(reqMaxSize);
			}
		}
		Word32 MaxSize()
		{
			return maxSize;
		}
		class Error : public Exception
		{
		public:
			Error(int type)
				: Exception(type)
			{
				this->type = type;
				this->str = errorStrs[type];
			}
			virtual const char *GetStr()
			{
				return str;
			}
		protected:
			static const char *errorStrs[];
			const char *str;
		};
		friend class ReadStorage;
	protected:	
		
		int FitInStorage(Word32 leng);
		int ReAllock(Word32 reqSize);

		Word32 size;
		Word32 maxSize;
		Word32 curPOS;
		Byte *pos;
		Byte *mem;
	};

	class ReadStorage : public Storage
	{
	public:
		ReadStorage(Storage &st)
			: Storage(st.mem, st.size)
		{
		}
		ReadStorage(Byte *mem, Word32 size)
			: Storage(mem, size)
		{
		}

		~ReadStorage()
		{
			mem = NULL;
		}
	};
};

