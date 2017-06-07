#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "file.hpp"
#include "log.hpp"

using namespace lb;

CacheFile::CacheFile(FILE *fd, const Word32 maxBlockUse, const Word32 blockSize)
 : File(fd), _maxBlockUse(maxBlockUse), _blockSize(blockSize)
{
	_Init();
}
CacheFile::CacheFile(const char *fileName, const char *mode, const Word32 maxBlockUse, const Word32 blockSize)
	: File(fileName, mode), _maxBlockUse(maxBlockUse), _blockSize(blockSize)
{
	_Init();
}

void CacheFile::_Init()
{
	_fileSize = FileSize();
	_blockUse = 0;
	_maxBlockCount = (_fileSize / _blockSize) + 1;

	_memBlocks = new MemBlock*[_maxBlockCount];
	for (Word32 i = 0; i < _maxBlockCount; i++)
		_memBlocks[i] = NULL;
}

CacheFile::~CacheFile()
{
	for (Word32 i = 0; i < _maxBlockCount; i++)
		delete _memBlocks[i];
	delete[] _memBlocks;
}

bool CacheFile::Read(const Word32 seek, const Word32 size, Byte *mem)
{
	if (seek + size > _fileSize)
		return false;

	Word32 startBlockNum =  seek / _blockSize;
	Word32 endBlockNum = (seek + size) / _blockSize;

	Word32 resSize = size;
	Word32 resSeek = seek;
	Word32 curLoad = 0;
	for (Word32 i = startBlockNum; i <=	endBlockNum; i++)
	{
		if (!_memBlocks[i])
			if (!_LoadMemBlock(i))
				return false;
		Word32 res = _memBlocks[i]->Load(mem + curLoad, resSeek, resSize);
		if (!res)
			return false;
		resSize -= res;
		resSeek += res;
		curLoad += res;
		if (!resSize)
			break;
	}
	return true;
}

bool CacheFile::_LoadMemBlock(const Word32 blockNum)
{
	Word32 startSeek = blockNum * _blockSize;

	MemBlock *mem = NULL;
	if (_blockUse < _maxBlockUse)
	{
		mem =  new MemBlock(_blockSize);	
		_blockUse++;
	}
	else
	{
		Word32 minAccCount = 0;
		Word32 minBlock = 0;
		Word32 i = 0;
		
		for (; i < _maxBlockCount; i++)
			if (_memBlocks[i])
			{
				minAccCount = _memBlocks[i]->AccessCount();
				minBlock = i;
				_memBlocks[i]->ResetAccessCount();

				break;
			}
		if (minAccCount > 0)
		{
			for (; i < _maxBlockCount; i++)
				if (_memBlocks[i])
				{
					if (_memBlocks[i]->AccessCount() < minAccCount)
					{
						minAccCount = _memBlocks[i]->AccessCount();
						minBlock = i;
						if (minAccCount == 0)
							break;
					}
					_memBlocks[i]->ResetAccessCount();
				}
		}
		mem = _memBlocks[minBlock];
		_memBlocks[minBlock] = NULL;
	}

	Word32 resSize = _blockSize; // if end block
	if (_blockSize + startSeek > _fileSize)
		resSize = _fileSize - startSeek;
	
	if (!mem->Read(_fd, startSeek, resSize))
	{
		delete mem;
		_blockUse--;
		return false;
	}
	_memBlocks[blockNum]  = mem;
	return true;
}

bool CacheFile::MemBlock::Read(FILE *fd, const Word32 startSeek, const Word32 resSize)
{
	_fromSeek = startSeek;
	_size = resSize;
	fseek(fd, _fromSeek, SEEK_SET);

	if (fread(_mem, resSize, 1, fd) > 0)
		return true;
	else
		return false;
}

Word32 CacheFile::MemBlock::Load(Byte *mem, const Word32 seek, const Word32 size)
{
	if (seek >= _fromSeek && seek < _fromSeek + _size)
	{
		Word32 resSize = size;
		if (seek + size > _fromSeek + _size)
			resSize = (_fromSeek + _size) - seek;
		memcpy(mem, _mem + (seek - _fromSeek), resSize);
		_accessCount++;
		return resSize;
	}
	return 0;
}
