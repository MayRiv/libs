#pragma once

#include <vector>
#include "storage.hpp"
#include "hash.hpp"



namespace lb
{
	const Byte STORAGE_STATUS_OK				= 0;
	const Byte STORAGE_STATUS_CLOSED		= 1;
	const Byte STORAGE_STATUS_DESTROYED	= 2;
	const Byte STORAGE_STATUS_SORTED 		= 3;


	
	class StorageSystem
	{
	public:
		static const Word32 MAX_STORAGE_SIZE = 1638 * 1024 * 1024;
		static const Word32 MAX_32BIT_FILE = (1945 * 1024 * 1024); // max 32 bit funcs file

		StorageSystem(Byte storageID, const char *fileName, Word32 maxStorageSize = MAX_STORAGE_SIZE);
		StorageSystem(const char *fileName, Word32 maxStorageSize = MAX_STORAGE_SIZE);
		
		~StorageSystem();
		Byte Status() { return _status; }
		Byte StorageID() { return _storageID; }
		Word32 CurSeek() { return _curSeek; }
		Word32 HeaderSize();
		String &FileName()
		{
			return _fileName;
		}

		FILE *File()
		{
			return _file;
		}
		int IsOpen() 
		{
			if (_curSeek >= _maxStorageSize)
				return 0;
			if (_curSeek >= MAX_32BIT_FILE)
				return 0;
				
			if (_status == STORAGE_STATUS_OK)
				return 1;
			else
				return 0;
		}
		int IsFree();
		void SetStatus(Byte status)
		{
			_status = status;
		}
		int Add(class Storage &st, Word32 *seek);
		Word32 AddToCur(class Storage &st); // add to cur file position
		int Add(const Byte *data, const Word32 size, Word32 *seek);
		int Save(class Storage &st, const Word32 seek);
		int Save(const Byte *data, const Word32 seek, const Word32 size);
		int Load(class Storage &st, const Word32 seek, const Word32 size);
		int Load(Byte *data, const Word32 seek, const Word32 size);
		Word32 LoadCompressData(class Storage &st, const Word32 seek);
		Word32 AddCompressToStore(class Storage &st, Word32 *seek);
		Word32 AddCompressToStore(Byte *data, Word32 destlen, Word32 unpackSize, Word32 *seek);
		Word32 AddUnCompressToStore(class Storage &st, Word32 *seek);
		Word32 LoadUnCompressData(class Storage &st, const Word32 seek);
		Word32 LoadAllData(class Storage &st, const Word32 seek);
		class Storage *LoadDataInStorage(Word32 &curSeek);
		Word32 OverwriteCompressData(Byte *data, Word32 destlen, Word32 unpackSize, const Word32 seek);
		Word32 LoadCompressPackData(class Storage &comSt, const Word32 seek);
		void Trunc();
		void SetSeekToEnd();
		void SaveStorageHeader();
		
		class Error_FileError {};

	protected:
		struct LoadBlock
		{
			Word32 size;
			Word32 nextSeek;
		};

		FILE *_CreateNewStorage();
		void _LoadStorageHeader();
		Byte _storageID;
		String _fileName;
		Word32 _curSeek;
		Byte _status;
		FILE *_file;
		Word32 _maxStorageSize;
	};

	class MemBlock 
	{
	public:
		MemBlock(Byte *mem, Word32 fromSeek, Word32 size, Word32 lastAccess)
		{
  		this->mem = mem;
  		this->fromSeek = fromSeek;
  		this->size = size;
  		this->lastAccess = lastAccess;
		}
		Word32 Load(Storage &st, Word32 seek, Word32 size, class Time &curTime);
		Word32 LastAccessTime() { return lastAccess; }
		Byte *Mem() { return mem; }
	protected:

		Word32 lastAccess;
		Word32 fromSeek;
		Word32 size;
		Byte *mem;
	};

	class CachedStorageSystem : public StorageSystem
	{
	public:
		class EmptyStorage {};
		class DataOutOfRange {};
		class CannotReadData {}; 
		CachedStorageSystem(char *fileName, Word32 blockSize, Word32 maxMemoryBlockUse);
		~CachedStorageSystem();
		int Load(class Storage &st, const Word32 seek, const Word32 size, class Time &curTime);
		void LoadMemBlock(Word32 blockNum, class Time &curTime);
	protected:
		Word32 blockSize;
		Word32 maxBlockCount;
		Word32 maxMemoryBlockUse;
		Word32 memoryBlockUse;
		MemBlock **memBlocks;
		
	};

	class QueueCachedStorageSystem : public StorageSystem
	{
	public:
		class EmptyStorage {};
		class DataOutOfRange {};
		class CannotReadData {}; 
		QueueCachedStorageSystem(const char *fileName, Word32 memSize);
		int LoadNextData();
		int Load(class Storage &st, Word32 size);
		Word32 CurSeek()
		{
			return curLoadSeek + curMem.CurPos();
		}
		Word32 MaxSeek()
		{
			return maxSeek;
		}
		void GetData(Byte *data, const Word32 size);
		void SkipData(Word32 size);
	protected:
		Storage curMem;
		Word32 maxSeek;
		Word32 curLoadSeek;
		Word32 loadTo;
	};

	struct PackDataHeader
	{
		Word32 packSize;
		Word32 unpackSize;
	};

	class StorageManager
	{
	public:
		class StorageError {};
		class CannotFindStorage : public StorageError {};
		class CannotAddStorage : public StorageError {};
		class CannotSaveStorage : public StorageError {};
		class CannotLoadStorage : public StorageError {};
		class BadStorageID : public StorageError {};

		StorageManager();

		void LoadFromStore(class Storage &st, const Byte storageID, const Word32 seek, const Word32 size);
		void LoadFromStoreToMem(Byte *mem, const Byte storageID, const Word32 seek, const Word32 size);

		void SaveToStore(class Storage &st, const Byte storageID, const Word32 seek);
		void SaveToStore(const Byte *mem, const Word32 size, const Byte storageID, const Word32 seek);

		void AddToStore(const Byte *mem, const Word32 size, Byte *storageID, Word32 *seek);
		void AddToStore(class Storage &st, Byte *storageID, Word32 *seek);
		void AddToStoreBalance(class Storage &st, Byte *storageID, Word32 *seek);
		
		Word32 AddToStore(class Storage &st, const Byte storageID);
		Word32 AddToStore(const Byte *mem, const Word32 size, const Byte storageID);

		Word32 AddCompressToStore(class Storage &st, Byte *storageID, Word32 *seek);
		Word32 UpdateCompressToStore(class Storage &st, Byte *storageID, Word32 *seek);
		Word32 AddUnCompressToStore(class Storage &st, Byte *storageID, Word32 *seek);
		Word32 LoadCompressData(class Storage &st, const Byte storageID, const Word32 seek);
		Word32 LoadUnCompressData(class Storage &st, const Byte storageID, const Word32 seek);
		Word32 LoadAllData(class Storage &st, const Byte storageID, const Word32 seek);
		
		void AddStorageSystem(const char *fileName, const Word32 maxStorageSize = StorageSystem::MAX_STORAGE_SIZE);
		StorageSystem *GetDefaultStorage()
		{
			if (!_defaultAddStorage->IsOpen())
				if (!FindDefaultStorage())
					throw CannotFindStorage();
			return _defaultAddStorage;
		}
		StorageSystem *FindFreeStorage();
		StorageSystem *FindStorage(const Byte storageID)
		{
			if (storageID >= _storages.size())
				throw BadStorageID();
			return _storages[storageID];
		}
		Word32 CountStorages() const
		{
			return _storages.size();
		}
		void SaveStorageSystem();
		void SetSeekToEnd();
		int FindDefaultStorage();
	protected:
		typedef std::vector<StorageSystem*> TStorageVector;
		TStorageVector _storages;
		StorageSystem *_defaultAddStorage;
	};

	class VersionStorageManager : public StorageManager
	{
	public:
		void FlushDeletedSeek();
		void MarkDeletedSeek(const Byte storageID, const Word32 seek);
		void AddStorageSystem(const char *fileName);
		void Trunc(const Byte storageID);
	private:
		typedef std::vector<FILE *> TRemoveSeekFD;
		TRemoveSeekFD _removeSeekFDs;
	};

	

	class VersionStorageSystem : public StorageSystem
	{
	public:
		VersionStorageSystem(const char *fileName);
		Word32 LoadCompressData(class Storage &st, const Word32 seek, Word32 &skip);
		Word32 LoadCompressPackData(class Storage &comSt, const Word32 seek, Word32 &skip);
		Word32 IsPurged(const Word32 seek)
		{
			return purgeSeeks.Find(seek);
		}
		void Trunc();
	protected:
		Hash<Word32> purgeSeeks;
	};
};


