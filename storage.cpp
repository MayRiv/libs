#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifdef _WITH_BZIP2_LIB
	#include <bzlib.h>
	#define Z_OK BZ_OK
	#define Z_BUF_ERROR BZ_OUTBUFF_FULL
#else
	#include <zlib.h>
#endif

#include "storage.hpp"
#include "log.hpp"

using namespace lb;

const char *Storage::Error::errorStrs[] =  {
	"Undefined error",
	"Cannot reallock new mem",
	"Cannot get element out of range",
	"I/O file error"
};

Storage::Storage(const Storage &st)
{
	maxSize = st.size + 1;
	mem = (Byte*)malloc(maxSize);
	if (!mem)
		throw Error(STORAGE_ERROR_REALLOCK);
	size = st.size;
	memcpy(mem, st.mem, size);
	curPOS = st.curPOS;
	pos = mem + curPOS;
}

Storage::Storage(Word32 startSize)
{
	maxSize = startSize;
	mem = (Byte*)malloc(maxSize);
	if (!mem)
		throw Error(STORAGE_ERROR_REALLOCK);
	size = 0;
	pos = mem;
	curPOS = 0;
}

Storage::Storage(Byte *commem, Word32 size, Word32 unpacksize)
{
	maxSize = unpacksize;
	mem = (Byte*)malloc(maxSize);
	
#ifdef _WITH_BZIP2_LIB
	int res = BZ2_bzBuffToBuffDecompress ( (char*)mem, (unsigned int*) &unpacksize, (char*)commem, size, 0, 0);
#else
	int res = uncompress(mem, (uLongf*)&unpacksize, commem, size);
#endif

	if (res != Z_OK)
	{
		free(commem);
		free(mem);
		throw Error(STORAGE_ERROR_OUTOFRANGE);
	}
	free(commem); 
	this->size = unpacksize;
	pos = mem;
	curPOS = 0;
}

Storage::Storage(Byte *mem, Word32 size)
{
	maxSize = size;
	this->mem = mem;
	this->size = size;
	pos = mem;
	curPOS = 0;
}

Storage::Storage(const char *gzFileName)
{
	mem = NULL;
	FILE *gzF=fopen(gzFileName, "rb");
	if (!gzF)
		throw Error(STORAGE_ERROR_IOERROR);

	Word32 comlen = 0;
	if (!fread(&comlen, sizeof(comlen), 1, gzF))
	{
		fclose(gzF);
		throw Error(STORAGE_ERROR_IOERROR);
	}
	if (!comlen || comlen > MAX_STORAGE_SIZE)
	{
		fclose(gzF);
		throw Error(STORAGE_ERROR_REALLOCK);
	}
		
	if (!fread(&maxSize, sizeof(maxSize), 1, gzF))
	{
		fclose(gzF);
		throw Error(STORAGE_ERROR_IOERROR);
	}
	if (!maxSize || maxSize > MAX_STORAGE_SIZE)
	{
		fclose(gzF);
		throw Error(STORAGE_ERROR_REALLOCK);
	}
	
	Byte *commem = (Byte*)malloc(comlen);
	if (!fread(commem, comlen, 1, gzF))
	{
		free(commem);
		fclose(gzF);
		throw Error(STORAGE_ERROR_IOERROR);
	}
	
	fclose(gzF);
	
	mem = (Byte*)malloc(maxSize);
	
#ifdef _WITH_BZIP2_LIB
	if ((BZ2_bzBuffToBuffDecompress ( (char*)mem, (unsigned int*) &maxSize, (char*)commem, comlen, 0, 0)) != Z_OK)
#else
	if ((uncompress(mem, (uLongf*)&maxSize, commem, comlen)) != Z_OK)
#endif
	{
		free(commem);
		throw Error(STORAGE_ERROR_IOERROR);
	}
	
	free(commem);
	
	size = maxSize;
	pos = mem;
	curPOS = 0;
}

Storage::~Storage()
{
	free(mem);
}

Word32 Storage::SetData(Storage &st)
{
	Start();
	return AddData(st.mem, st.Size());
}

Word32 Storage::AddData(Storage &st)
{
	return AddData(st.mem, st.Size());
}

void Storage::CopyData(Byte *data)
{
	memcpy(data, mem, size);
}

lb::Byte *Storage::Compress(Word32 &destlen, int level)
{
	destlen = (Word32)(size*1.5+600);
        
	Byte *commem = NULL;
        commem = (Byte *)malloc(destlen);
        if (!commem)
            throw Error(STORAGE_ERROR_REALLOCK);
#ifdef _WITH_BZIP2_LIB
	int res = BZ2_bzBuffToBuffCompress( (char*)commem, (unsigned int*) &destlen, (char*)mem, size, level, 0, 0);
#else
	int res=compress2(commem, (uLongf*)&destlen, (Bytef*)mem, size, level);
#endif
	if (res != Z_OK)
	{
            
		free(commem);
		return NULL;
	}
	return commem;
}

lb::Byte *Storage::Compress(int level)
{
	Word32 maxLen = (size * 1.1 + 600);
	Word32 destlen = maxLen;
	Byte *commem = (Byte *)malloc(destlen);
#ifdef _WITH_BZIP2_LIB
	int res = BZ2_bzBuffToBuffCompress( (char*)commem, (unsigned int*) &destlen, (char*)mem, size, level, 0, 0);
#else
	int res=compress2(commem, (uLongf*)&destlen, (Bytef*)mem, size, level);
#endif
	if (res != Z_OK)
	{
		free(commem);
		return NULL;
	}

	free(mem);
	mem = commem;
	size = destlen;
	maxSize = maxLen;
	Begin();

	return commem;
}


Word32 Storage::SaveToGzFile(char *fileName)
{
	Word32 destlen = 0;
	Byte *commmem = Compress(destlen);
	if (!commmem)
		return 0;
	FILE *gzF=fopen(fileName, "wb+");
	if (gzF)
	{
		if (!fwrite(&destlen, sizeof(destlen), 1, gzF))
		{
			free(commmem);
			return 0;
		}
		if (!fwrite(&size, sizeof(size), 1, gzF))
		{
			free(commmem);
			return 0;
		}
		if (!fwrite(commmem, destlen, 1, gzF))
		{
			free(commmem);
			return 0;
		}

		fclose(gzF);
		free(commmem);
		return destlen;
	}
	else
	{
		free(commmem);
		return 0;
	}
}

int Storage::WriteToFile(FILE *file)
{
	if (!fwrite(mem,  size, 1, file))
		return 0;
	return 1;
}
int Storage::WriteToFile(int fD)
{
	if (!write(fD, mem,  size))
		return 0;
	return 1;
}

int Storage::AddFromFile(FILE *file, const Word32 leng)
{
	if (!FitInStorage(leng))
		throw Error(STORAGE_ERROR_REALLOCK);
	if (!fread(pos, leng, 1, file))
		return 0;
	pos += leng;
	size += leng;
	return 1;
}

int Storage::AddFromFile(int fD, const Word32 leng)
{
	if (!FitInStorage(leng))
		throw Error(STORAGE_ERROR_REALLOCK);
	if (!read(fD, pos, leng))
		return 0;
	pos += leng;
	size += leng;
	return 1;
}

int Storage::LoadFromFile(FILE *file, const Word32 leng)
{
	Start();
	if (!FitInStorage(leng))
		throw Error(STORAGE_ERROR_REALLOCK);
	if (!fread(pos, leng, 1, file))
		return 0;
	size += leng;
	return 1;
}

int Storage::LoadFromFile(int fD, const Word32 leng)
{
	Start();
	if (!FitInStorage(leng))
		throw Error(STORAGE_ERROR_REALLOCK);
	if (!read(fD, pos, leng))
		return 0;
	size += leng;
	return 1;
}

int Storage::LoadFromGzFile(int fD, const Word32 uncompressedLeng)
{
	Start();
	if (!FitInStorage(uncompressedLeng))
		throw Error(STORAGE_ERROR_REALLOCK);

	gzFile gz = gzdopen(dup(fD), "rb");
	if (gz == NULL)
		return 0;

	int res = gzread(gz, pos, uncompressedLeng);
	gzclose(gz);
	if (res != (int) uncompressedLeng)
		return 0;

	size += uncompressedLeng;
	return 1;
}

Word32 Storage::Space(Word32 leng)
{
	if (!FitInStorage(leng))
		throw Error(STORAGE_ERROR_REALLOCK);
	pos += leng;
	size += leng;
	return size - leng;
}
Word32 Storage::Skip(Word32 leng)
{
	if (curPOS  + leng > size )
		throw Error(STORAGE_ERROR_OUTOFRANGE);
	pos += leng;
	curPOS += leng;
	return curPOS;
}

Word32 Storage::SkipString()
{
	Byte leng = (Byte)*(pos);
	curPOS++;
	pos++;
	if (curPOS + leng > size )
		throw Error(STORAGE_ERROR_OUTOFRANGE);
	pos += leng;
	curPOS += leng;
	return curPOS;
}

Word32 Storage::GetData(Storage &st, const Word32 leng)
{
	if (curPOS + leng > size )
		throw Error(STORAGE_ERROR_OUTOFRANGE);
	st.Begin();
	st.AddData(pos, leng);
	pos += leng;
	curPOS += leng;
	return curPOS;
}

Word32 Storage::GetData(void *data, Word32 leng)
{
	if (curPOS + leng > size )
		throw Error(STORAGE_ERROR_OUTOFRANGE);
	memcpy(data, pos, leng);
	pos += leng;
	curPOS += leng;
	return curPOS;
}

lb::Byte *Storage::Attach(Word32 leng)
{
	if (curPOS + leng > size )
		throw Error(STORAGE_ERROR_OUTOFRANGE);
	Byte *old = pos;
	pos += leng;
	curPOS += leng;
	return old;
}

lb::Byte *Storage::Detach()
{
	Byte *old = mem;
	size = 0;
	curPOS = 0;
	pos = 0;
	mem = 0;

	return old;
}

lb::Byte *Storage::AddAttach(Word32 leng)
{
	if (!FitInStorage(leng))
		throw Error(STORAGE_ERROR_REALLOCK);
	Byte *oldPos = pos;
	pos += leng;
	size += leng;
	return oldPos;
}

Word32 Storage::AddData(const void *data, Word32 leng)
{
	if (!FitInStorage(leng))
		throw Error(STORAGE_ERROR_REALLOCK);
	memcpy(pos, data, leng);
	pos += leng;
	size += leng;
	return size;
}

Word32 Storage::AddString(const RString &str)
{
	Byte strLen = str.Length();
	if (!FitInStorage(strLen + sizeof(Byte)))
		throw Error(STORAGE_ERROR_REALLOCK);
	*pos = strLen;
	pos++;
	if (strLen)
	{
		memcpy(pos, str.Data(), strLen);
		pos += strLen;
	}
	size += strLen+sizeof(Byte);
	return size;
}

Word32 Storage::AddString(const char *str) // Add string max 255
{
	Byte strLen = strlen(str);
	if (!FitInStorage(strLen + sizeof(Byte)))
		throw Error(STORAGE_ERROR_REALLOCK);
	*pos = strLen;
	pos++;
	if (strLen)
	{
		memcpy(pos, str, strLen);
		pos += strLen;
	}
	size += strLen+sizeof(Byte);
	return size;
}

Word32 Storage::AddString(const String &str) // Add string max 255
{
	if (!FitInStorage(str.GetLength() + sizeof(Byte)))
		throw Error(STORAGE_ERROR_REALLOCK);
	Byte strLen = str.GetLength();
	*pos = strLen;
	pos++;
	if (strLen)
	{
		memcpy(pos, str.c_str(), strLen);
		pos += strLen;
	}
	size += strLen+sizeof(Byte);
	return size;
}

Word32 Storage::GetString(String &str)
{
	Byte leng = (Byte)*(pos);
	curPOS++;
	pos++;
	if (curPOS + leng > size )
		throw Error(STORAGE_ERROR_OUTOFRANGE);
	str.Set((char*)pos, (int)leng);
	
	pos += leng;
	curPOS += leng;
	return curPOS;
}

int Storage::FitInStorage(Word32 leng)
{
	if (size + leng < maxSize)
		return 1;
	else
	{
		return ReAllock(size + leng);
	}
}

int Storage::ReAllock(Word32 reqSize)
{
	if ((Word32)reqSize*STORAGE_MUL_KOEF < MAX_STORAGE_SIZE)
	{
		maxSize = (Word32)(reqSize*STORAGE_MUL_KOEF);
		Byte *newmem =(Byte*)malloc(maxSize);
		if (newmem)
		{
			memcpy(newmem, mem, size);
			free(mem);
			mem = newmem;
			pos = mem + size;
			return 1;
		}
		else
		{
			L(LOG_WARNING, "[ST] Can't mallock %u bytes of memory (%s - %u)", maxSize, strerror(errno), errno);
			return 0;
		}

/*		if ((newmem=(Byte*)realloc(mem, maxSize)) != NULL)
		{
			mem = newmem;
			pos = mem + size;
			return 1;
		}
		else
		{
			L(LOG_WARNING, "[ST] Can't rellock %u bytes of memory (%s - %u)", maxSize, strerror(errno), errno);
		}*/
	}
	return 0;
}

void Storage::Start()
{
	size = 0;
	pos = mem;
	curPOS = 0;
}

void Storage::Begin()
{
	pos = mem;
	curPOS = 0;
}

int Storage::Unpack(Word32 unpackSize)
{
	Byte *unpack = (Byte*)malloc(unpackSize);
#ifdef _WITH_BZIP2_LIB
	int res = BZ2_bzBuffToBuffDecompress ( (char*)unpack, (unsigned int*) &unpackSize, (char*)mem, size, 0, 0);
#else
	int res = uncompress(unpack, (uLongf*)&unpackSize, mem, size);
#endif
	if (res != Z_OK)
	{
		free(mem);
		free(unpack);
		throw Error(STORAGE_ERROR_IOERROR);
	}
	
	free(mem);
	mem = unpack;
	size = unpackSize;
	maxSize = size;
	Begin();
	return res;
}

int Storage::LoadUnpack(Byte *comdata, Word32 packSize, Word32 unpackSize)
{
	Start();
	if (!FitInStorage(unpackSize))
		throw Error(STORAGE_ERROR_REALLOCK);

#ifdef _WITH_BZIP2_LIB
	int res = BZ2_bzBuffToBuffDecompress ( (char*)mem, (unsigned int*) &unpackSize, (char*)comdata, packSize, 0, 0);
#else
	int res = uncompress(mem, (uLongf*)&unpackSize, comdata, packSize);
#endif
	if (res != Z_OK && res != Z_BUF_ERROR) // can load only part of comm data
		throw Error(STORAGE_ERROR_IOERROR);
	size = unpackSize;
	Begin();
	return res;
}

