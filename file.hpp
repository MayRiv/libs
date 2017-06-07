#pragma once

#include "tools.hpp"

namespace lb
{
	class File
	{
	public:
		class Error {};
		File(FILE *fd)
			: _fd(fd), _ownFD(false)
		{
		}
		File(const char *fileName, const char *mode, const bool create = false)
			: _fd(NULL), _ownFD(true)
		{
			Open(fileName, mode, create);
		}
		void Open(const char *fileName, const char *mode, const bool create = false)
		{
			Close();
			_fd = fopen(fileName, mode);
			if (!_fd)
			{
				if (create)
				{
					_fd = fopen(fileName, "w+");
					if (!_fd)
						throw Error();
				}
				else
					throw Error();
			}
		}
		~File()
		{
			if (_fd && _ownFD)
				fclose(_fd);
		}
		FILE *FD()
		{
			return _fd;
		}
		Word32 FileSize()
		{
			return lb::FileSize(_fd);
		}
		void Close()
		{
			if (_fd)
			{
				fclose(_fd);
				_fd = NULL;
			}
		}
	protected:
		FILE *_fd;
		bool _ownFD;
	};

	class CacheFile : public File
	{
	public:
		CacheFile(FILE *fd, const Word32 maxBlockUse, const Word32 blockSize);
		CacheFile(const char *fileName, const char *mode, const Word32 maxBlockUse, const Word32 blockSize);
		~CacheFile();
		bool Read(const Word32 seek, const Word32 size, Byte *mem);
		
	protected:
		bool _LoadMemBlock(const Word32 blockNum);
		void _Init();

		Word32 _maxBlockUse;
		Word32 _blockSize;
		Word32 _fileSize;
		Word32 _blockUse;
		Word32 _maxBlockCount;

		class MemBlock 
		{
		public:
			MemBlock(const Word32 blockSize)
				: _fromSeek(0), _accessCount(0), _size(0)
			{
				_mem = (Byte*)malloc(blockSize);
			}
			~MemBlock()
			{
				free(_mem);
			}
			Word32 Load(Byte *mem, const Word32 seek, const Word32 size);
			Word32 AccessCount() { return _accessCount; }
			Byte *Mem() { return _mem; }
			void ResetAccessCount()
			{
				_accessCount = 0;
			}
			bool Read(FILE *fd, const Word32 startSeek, const Word32 resSize);
		private:
			Word32 _fromSeek;
			Word32 _accessCount;
			Word32 _size;
			Byte *_mem;
		};

		MemBlock **_memBlocks;
	};

	class SequenceRead : public CacheFile
	{
	public:
		SequenceRead(FILE *fd, const Word32 blockSize)
			: CacheFile(fd, 1, blockSize)
		{
			_curSeek = ftell(_fd);
		}
		SequenceRead(const char *fileName, const char *mode, const Word32 blockSize)
			: CacheFile(fileName, mode, 1, blockSize), _curSeek(0)
		{
		}
		bool Read(const Word32 size, Byte *mem)
		{
			if (CacheFile::Read(_curSeek, size, mem))
			{
				_curSeek += size;
				return true;
			}
			else
				return false;
		}
		void Skip(const Word32 size)
		{
			_curSeek += size;
		}
		bool IsEnd()
		{
			if (_curSeek >= _fileSize)
				return true;
			else
				return false;
		}
		Word32 CurSeek()
		{
			return _curSeek;
		}
	private:
		Word32 _curSeek;
	};
};
