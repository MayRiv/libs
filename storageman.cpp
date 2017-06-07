#include <stdlib.h>
#include "dir.hpp"
#include "log.hpp"
#include "storage.hpp"
#include "time.hpp"
#include "storageman.hpp"


using namespace lb;


StorageManager::StorageManager()
{
	_defaultAddStorage = NULL;
}

void StorageManager::AddStorageSystem(const char *fileName, const Word32 maxStorageSize)
{
	_storages.push_back(new StorageSystem(_storages.size(), fileName, maxStorageSize));
}

void VersionStorageManager::AddStorageSystem(const char *fileName)
{
	RString buf;
	buf.snprintf("%s.rm", fileName);
	FILE *fp = fopen(buf.Data(), "a");
	if (!fp)
		throw CannotAddStorage();
	_removeSeekFDs.push_back(fp);
	StorageManager::AddStorageSystem(fileName);
}

void VersionStorageManager::FlushDeletedSeek()
{
	for (TRemoveSeekFD::iterator i =  _removeSeekFDs.begin(); i != _removeSeekFDs.end(); i++)
		fflush(*i);
}

void VersionStorageManager::MarkDeletedSeek(const Byte storageID, const Word32 seek)
{
	if (storageID >= _storages.size())
		throw BadStorageID();
	if (!fwrite(&seek, sizeof(seek), 1, _removeSeekFDs[storageID]))
		throw CannotAddStorage();
}

void VersionStorageManager::Trunc(const Byte storageID)
{
	if (storageID >= _storages.size())
		throw BadStorageID();
	RString fileName;
	fileName.snprintf("%s.rm", _storages[storageID]->FileName().c_str());
	_removeSeekFDs[storageID] = freopen(fileName.Data(), "w+", _removeSeekFDs[storageID]);
	_storages[storageID]->Trunc();
}

VersionStorageSystem::VersionStorageSystem(const char *fileName)
	: StorageSystem(fileName), purgeSeeks(10)
{
		RString buf;
		buf.snprintf("%s.rm", fileName);
		FILE *pFD = fopen(buf.Data(), "r");
		if (pFD)
		{
			while (!feof(pFD))
			{
				Word32 purgeSeek = 0;
				fread(&purgeSeek, sizeof(purgeSeek), 1, pFD);
				if (purgeSeek)
					purgeSeeks.Add(purgeSeek);
			}
			fclose(pFD);
		}
};

Word32 VersionStorageSystem::LoadCompressData(class Storage &st, const Word32 seek, Word32 &skip)
{
	skip = 0;
	if (purgeSeeks.Find(seek))
	{
		if (fseek(_file, seek, SEEK_SET))
			return 0;
		PackDataHeader ph;
		if (!fread(&ph, sizeof(PackDataHeader), 1, _file))
			return 0;
		skip = seek;
		return seek + ph.packSize + sizeof(PackDataHeader);
	}
	else
		return StorageSystem::LoadCompressData(st, seek);
}

Word32 VersionStorageSystem::LoadCompressPackData(class Storage &comSt, const Word32 seek, Word32 &skip)
{
	skip = 0;
	if (purgeSeeks.Find(seek))
	{
		if (fseek(_file, seek, SEEK_SET))
			return 0;
		PackDataHeader ph;
		if (!fread(&ph, sizeof(PackDataHeader), 1, _file))
			return 0;
		skip = seek;
		return seek + ph.packSize + sizeof(PackDataHeader);
	}
	else
		return StorageSystem::LoadCompressPackData(comSt, seek);
}

StorageSystem *StorageManager::FindFreeStorage()
{
	for (TStorageVector::iterator i = _storages.begin(); i != _storages.end(); i++)
		if ((*i)->IsFree())
			return (*i);
	return 0;
}

void StorageManager::SetSeekToEnd()
{
	for (TStorageVector::iterator i = _storages.begin(); i != _storages.end(); i++)
		(*i)->SetSeekToEnd();
}

void StorageManager::SaveStorageSystem()
{
	for (TStorageVector::iterator i = _storages.begin(); i != _storages.end(); i++)
		(*i)->SaveStorageHeader();
}

int StorageManager::FindDefaultStorage()
{
	for (TStorageVector::iterator i = _storages.begin(); i != _storages.end(); i++)
		if ((*i)->IsOpen())
		{
			_defaultAddStorage = (*i);
			return 1;
		}
	return 0;
}


StorageSystem::StorageSystem(const Byte storageID, const char *fileName, const Word32 maxStorageSize)
	: _storageID(storageID), _fileName(fileName), _maxStorageSize(maxStorageSize)
{
	L(LOG_WARNING, "Open storage %s\n", fileName);
	_file = fopen(fileName, "r+");
	if (!_file)
		_file = _CreateNewStorage();
	else
	{
		_LoadStorageHeader();
		if (_storageID != storageID)
		{
			L(LOG_CRITICAL, "Data safety destroyed %s (%u!=%u)\n", fileName, storageID, _storageID );
			throw StorageSystem::Error_FileError();
		}
	}
}

StorageSystem::StorageSystem(const char *fileName, const Word32 maxStorageSize)
	: _fileName(fileName), _maxStorageSize(maxStorageSize)
{
	L(LOG_WARNING, "Open storage %s\n", fileName);
	_file = fopen(fileName, "r+");
	if (!_file)
			throw StorageSystem::Error_FileError();
	else
		_LoadStorageHeader();
}


StorageSystem::~StorageSystem()
{
	fclose(_file);
}

const char *STORAGE_SIG = "D$";
const char STORAGE_SIG_SIZE = 2;

void StorageSystem::SaveStorageHeader()
{
	fseek(_file, 0, SEEK_SET);
	fwrite(STORAGE_SIG, STORAGE_SIG_SIZE, 1, _file);
	fwrite(&_storageID, sizeof(_storageID), 1, _file);
	fwrite(&_curSeek, sizeof(_curSeek), 1, _file);
	fwrite(&_status, sizeof(_status), 1, _file);
	fflush(_file);
}

void StorageSystem::Trunc()
{
	_curSeek = HeaderSize();
	_file = freopen(_fileName.c_str(), "w+", _file);
	SaveStorageHeader();
}

void VersionStorageSystem::Trunc()
{
	StorageSystem::Trunc();
	purgeSeeks.Flush();
}

void StorageSystem::_LoadStorageHeader()
{
	fseek(_file, 0, SEEK_SET);
	char sig[STORAGE_SIG_SIZE+1];
	fread(sig, STORAGE_SIG_SIZE, 1, _file);
	if (memcmp(sig, STORAGE_SIG, STORAGE_SIG_SIZE))
	{
		sig[(unsigned char)STORAGE_SIG_SIZE]  = 0;
		L(LOG_CRITICAL, "Bad signature '%s' -  load fail\n", sig);
		throw StorageSystem::Error_FileError();
	}
	
	fread(&_storageID, sizeof(_storageID), 1, _file);
	fread(&_curSeek, sizeof(_curSeek), 1, _file);
	fread(&_status, sizeof(_status), 1, _file);
	L(LOG_WARNING, "Load storage %s (%u) header curSeek %u status %u\n", _fileName.c_str(), _storageID, _curSeek, _status);
}

Word32 StorageSystem::HeaderSize()
{
 return STORAGE_SIG_SIZE + sizeof(_storageID) + sizeof(_curSeek) + sizeof(_status);
}

int StorageSystem::IsFree()
{
	if (IsOpen() && _curSeek == HeaderSize())
		return 1;
	else return 0;
}

FILE *StorageSystem::_CreateNewStorage()
{
	L(LOG_WARNING, "Create storage %s\n", _fileName.c_str());
	_status = STORAGE_STATUS_OK;
	_file = fopen(_fileName.c_str(), "w+");
	if (!_file)
	{
		L(LOG_WARNING, "Cannot create storage %s\n", _fileName.c_str());
		throw StorageSystem::Error_FileError();
	}
	_curSeek = HeaderSize();
	SaveStorageHeader();
	return _file;
}

int StorageSystem::Add(const Byte *data, const Word32 size, Word32 *seek)
{
	if (_curSeek >= MAX_32BIT_FILE)
		return 0;
	*seek = _curSeek;
	if (fseek(_file, _curSeek, SEEK_SET))
		return 0;
	if (!fwrite(data, size, 1, _file))
		return 0;
	_curSeek += size;
	return 1;
}

Word32 StorageSystem::AddToCur(class Storage &st)
{
	Word32 lastSeek = _curSeek;
	if (!st.WriteToFile(_file))
		return 0;
	_curSeek += st.Size();
	return lastSeek;
}

int StorageSystem::Add(class Storage &st, Word32 *seek)
{
	if (_curSeek >= MAX_32BIT_FILE)
		return 0;
	*seek = _curSeek;
	fseek(_file, _curSeek, SEEK_SET);
	if (!st.WriteToFile(_file))
		return 0;
	_curSeek += st.Size();
	return 1;
}

int StorageSystem::Save(class Storage &st, const Word32 seek)
{
	if (fseek(_file, seek, SEEK_SET))
		return 0;
	if (!st.WriteToFile(_file))
		return 0;
	return 1;
}

int StorageSystem::Save(const Byte *data, const Word32 seek, const Word32 size)
{
	if (fseek(_file, seek, SEEK_SET))
		return 0;
	if (!fwrite(data, size, 1, _file))
		return 0;
	return 1;
}

int StorageSystem::Load(Byte *data, const Word32 seek, const Word32 size)
{
	if (seek && fseek(_file, seek, SEEK_SET))
		return 0;
	if (!fread(data, size, 1, _file))
		return 0;
	return 1;
}
int StorageSystem::Load(class Storage &st, const Word32 seek, const Word32 size)
{
	if (seek && fseek(_file, seek, SEEK_SET)) // if !seek - no seek
		return 0;
	if (!st.LoadFromFile(_file, size))
		return 0;
	return 1;
}
Word32 StorageSystem::AddUnCompressToStore(class Storage &st, Word32 *seek)
{
	if (_curSeek >= MAX_32BIT_FILE)
		return 0;

 	Word32 unpackSize = st.Size();
	
	*seek = _curSeek;
	fseek(_file, _curSeek, SEEK_SET);
	
	if (!fwrite(&unpackSize, sizeof(unpackSize), 1, _file))
		return 0;
	if (!st.WriteToFile(_file))
		return 0;
		
	_curSeek += unpackSize + sizeof(unpackSize);

	return 1;	
}

class Storage *StorageSystem::LoadDataInStorage(Word32 &curSeek)
{
	LoadBlock lb;
	if (fseek(_file, curSeek, SEEK_SET))
		return NULL;
	if (!fread(&lb, sizeof(lb), 1, _file))
		return NULL;
	Storage *st = new Storage(lb.size+1);
	if (!st->AddFromFile(_file, lb.size))
		return NULL;
	curSeek = lb.nextSeek;
	st->Begin();
	return st;
}

Word32 StorageSystem::LoadAllData(class Storage &st, const Word32 seek)
{
	Word32 curSeek = seek;
	LoadBlock lb;
	st.Start();
	while (curSeek)
	{
		if (fseek(_file, curSeek, SEEK_SET))
			return 0;
		if (!fread(&lb, sizeof(lb), 1, _file))
			return 0;
		if (!st.AddFromFile(_file, lb.size))
			return 0;
		curSeek = lb.nextSeek;
	}
	st.Begin();
	return ftell(_file);
}

Word32 StorageSystem::LoadUnCompressData(class Storage &st, const Word32 seek)
{
	if (fseek(_file, seek, SEEK_SET))
		return 0;

	Word32 unpackSize;
	if (!fread(&unpackSize, sizeof(unpackSize), 1, _file))
		return 0;
	if (!st.LoadFromFile(_file, unpackSize))
		return 0;
	return ftell(_file);
}

Word32 StorageSystem::AddCompressToStore(Byte *data, Word32 destlen, Word32 unpackSize, Word32 *seek)
{
	if (_curSeek >= MAX_32BIT_FILE)
		return 0;

	PackDataHeader ph;
 	ph.unpackSize = unpackSize;
	ph.packSize = destlen;

	*seek = _curSeek;
	fseek(_file, _curSeek, SEEK_SET);
	
	if (!fwrite(&ph, sizeof(ph), 1, _file))
		return 0;
	if (!fwrite(data, destlen, 1, _file))
		return 0;

	_curSeek += destlen + sizeof(ph);

	return destlen;
}

void StorageSystem::SetSeekToEnd()
{
	if (fseek(_file, 0, SEEK_END))
		return;
	_curSeek = ftell(_file);
	L(LOG_WARNING, "Set seek to %u\n", _curSeek);
}

Word32 StorageSystem::AddCompressToStore(class Storage &st, Word32 *seek)
{
	if (_curSeek >= MAX_32BIT_FILE)
		return 0;

	PackDataHeader ph;
 	ph.unpackSize = st.Size();
	
	Word32 destlen = 0;
	Byte *data = st.Compress(destlen);
	ph.packSize = destlen;

	*seek = _curSeek;
	fseek(_file, _curSeek, SEEK_SET);
	
	if (!fwrite(&ph, sizeof(ph), 1, _file))
	{
		free(data);
		return 0;
	}
	if (!fwrite(data, destlen, 1, _file))
	{
		free(data);
		return 0;
	}

	_curSeek += destlen + sizeof(ph);

	free(data);
	return destlen;
}
const unsigned int MAX_DATA_SIZE = 100000000;

Word32 StorageSystem::LoadCompressData(class Storage &st, const Word32 seek)
{
	if (fseek(_file, seek, SEEK_SET))
		return 0;

	PackDataHeader ph;
	if (!fread(&ph, sizeof(PackDataHeader), 1, _file))
		return 0;
	if (ph.packSize >= MAX_DATA_SIZE || ph.unpackSize >= MAX_DATA_SIZE) // only befor new reindex
	{
		L(LOG_WARNING, "Bad ph.packsize = %u, ph.unpackSize=%u\n",ph.packSize, ph.unpackSize);
		return 0;
	}
	if (!st.LoadFromFile(_file, ph.packSize))
		return 0;
	st.Unpack(ph.unpackSize);
	return ftell(_file);
}

Word32 StorageSystem::LoadCompressPackData(class Storage &comSt, const Word32 seek)
{
	if (fseek(_file, seek, SEEK_SET))
		return 0;

	PackDataHeader ph;
	if (!fread(&ph, sizeof(PackDataHeader), 1, _file))
		return 0;
	if (ph.packSize >= MAX_DATA_SIZE || ph.unpackSize >= MAX_DATA_SIZE) // only befor new reindex
	{
		L(LOG_WARNING, "Bad ph.packsize = %u, ph.unpackSize=%u\n",ph.packSize, ph.unpackSize);
		return 0;
	}
	comSt.Start();
	comSt.AddData(&ph, sizeof(PackDataHeader));
	Word32 dataStarts = comSt.Size();
	if (!comSt.AddFromFile(_file, ph.packSize))
		return 0;
	comSt.SetPos(dataStarts);
	return ftell(_file);
}

Word32 StorageManager::LoadCompressData(class Storage &st, const Byte storageID, const Word32 seek)
{
	if (storageID >= _storages.size())
		throw BadStorageID();
	Word32 lastSeek = 0;
	if ((lastSeek = _storages[storageID]->LoadCompressData(st, seek)) == 0)
		throw CannotLoadStorage();
	return lastSeek;
}

Word32 StorageManager::LoadUnCompressData(class Storage &st, const Byte storageID, const Word32 seek)
{
	if (storageID >= _storages.size())
		throw BadStorageID();
	Word32 lastSeek = 0;
	if ((lastSeek = _storages[storageID]->LoadUnCompressData(st, seek)) == 0)
		throw CannotLoadStorage();
	return lastSeek;
}

Word32 StorageManager::LoadAllData(class Storage &st, const Byte storageID, const Word32 seek)
{
	if (storageID >= _storages.size())
		throw BadStorageID();
	Word32 lastSeek = 0;
	if ((lastSeek = _storages[storageID]->LoadAllData(st, seek)) == 0)
		throw CannotLoadStorage();
	return lastSeek;
}

void StorageManager::LoadFromStore(class Storage &st, const Byte storageID, const Word32 seek, const Word32 size)
{
	if (storageID >= _storages.size())
		throw BadStorageID();
	if (!_storages[storageID]->Load(st, seek, size))
		throw CannotLoadStorage();
}

void StorageManager::LoadFromStoreToMem(Byte *mem, const Byte storageID, const Word32 seek, const Word32 size)
{
	if (storageID >= _storages.size())
		throw BadStorageID();
	if (!_storages[storageID]->Load(mem, seek, size))
		throw CannotLoadStorage();
}

void StorageManager::AddToStore(class Storage &st, Byte *storageID, Word32 *seek)
{
	if (!_defaultAddStorage->IsOpen())
	{
		if (!FindDefaultStorage())
		{
			L(LOG_ERROR, "CannotFindStorage\n");
			throw CannotFindStorage();
		}
	}
	if (!_defaultAddStorage->Add(st, seek))
	{
		L(LOG_ERROR, "CannotAddStorage\n");
		throw CannotAddStorage();
	}
	*storageID = _defaultAddStorage->StorageID();
}

void StorageManager::AddToStoreBalance(class Storage &st, Byte *storageID, Word32 *seek)
{
	TStorageVector goodStorages;
	for (TStorageVector::iterator i = _storages.begin(); i != _storages.end(); i++)
	{	
		if ((*i)->IsOpen())
			goodStorages.push_back(*i);
	}
	if (goodStorages.empty())
	{
		L(LOG_ERROR, "AddToStoreBalance CannotFindStorage\n");
		throw CannotFindStorage();
	}
	StorageSystem *stSystem = goodStorages[rand() % goodStorages.size()];
	if (!stSystem->Add(st, seek))
	{
		L(LOG_ERROR, "CannotAddStorage %u\n", stSystem->StorageID());
		throw CannotAddStorage();
	}
	*storageID = stSystem->StorageID();
}

void StorageManager::AddToStore(const Byte *mem, const Word32 size, Byte *storageID, Word32 *seek)
{
	if (!_defaultAddStorage->IsOpen())
	{
		if (!FindDefaultStorage())
		{
			L(LOG_ERROR, "CannotFindStorage %u\n", storageID);
			throw CannotFindStorage();
		}
	}
	if (!_defaultAddStorage->Add(mem, size, seek))
	{
		L(LOG_ERROR, "CannotAddStorage %u\n", storageID);
		throw CannotAddStorage();
	}
	*storageID = _defaultAddStorage->StorageID();
}

Word32 StorageManager::AddToStore(class Storage &st, const Byte storageID)
{
	if (storageID >= _storages.size())
	{
		L(LOG_ERROR, "Bad storage id %u\n", storageID);
		throw BadStorageID();
	}
	Word32 seek = 0;
	if (!_storages[storageID]->Add(st, &seek))
	{
		L(LOG_ERROR, "CannotAddStorage %u\n", storageID);
		throw CannotAddStorage();
	}
	return seek;
}

Word32 StorageManager::AddToStore(const Byte *mem, const Word32 size, const Byte storageID)
{
	if (storageID >= _storages.size())
	{
		L(LOG_ERROR, "Bad storage id %u\n", storageID);
		throw BadStorageID();
	}
	Word32 seek = 0;
	if (!_storages[storageID]->Add(mem, size, &seek))
		throw CannotAddStorage();
	return seek;
}

Word32 StorageManager::AddCompressToStore(class Storage &st, Byte *storageID, Word32 *seek)
{
	if (!_defaultAddStorage->IsOpen())
		if (!FindDefaultStorage())
			throw CannotFindStorage();
	long 	destlen = 0;
	if ((destlen = _defaultAddStorage->AddCompressToStore(st, seek)) == 0)
		throw CannotAddStorage();
	*storageID = _defaultAddStorage->StorageID();
	
	return destlen;
}
 
Word32 StorageManager::AddUnCompressToStore(class Storage &st, Byte *storageID, Word32 *seek)
{
	if (!_defaultAddStorage->IsOpen())
		if (!FindDefaultStorage())
			throw CannotFindStorage();

	if (_defaultAddStorage->AddUnCompressToStore(st, seek) == 0)
		throw CannotAddStorage();
	*storageID = _defaultAddStorage->StorageID();
	
	return 1;
}

CachedStorageSystem::CachedStorageSystem(char *fileName, Word32 blockSize, Word32 maxMemoryBlockUse)
	: StorageSystem(fileName)
{
	this->maxMemoryBlockUse = maxMemoryBlockUse;
	memoryBlockUse = 0;
	this->blockSize = blockSize;
	if (IsFree())
		throw EmptyStorage();

	this->maxBlockCount = ((_curSeek - HeaderSize()) / blockSize)+1;
	L(LOG_WARNING, "Create %u memory blocks\n", maxBlockCount);
	memBlocks = new MemBlock* [maxBlockCount];
	for (Word32 i = 0; i < maxBlockCount; i++)
		memBlocks[i] = NULL;
}

CachedStorageSystem::~CachedStorageSystem()
{
	for (Word32 i = 0; i < maxBlockCount; i++)
	{
		if (memBlocks[i])
		{
			free(memBlocks[i]->Mem());
			delete memBlocks[i];
			memBlocks[i] = NULL;
		}
	}
	delete memBlocks;
}

int CachedStorageSystem::Load(class Storage &st, const Word32 seek, const Word32 size, class Time &curTime)
{
	Word32 startBlockNum =  (seek - HeaderSize()) / blockSize;
	Word32 endBlockNum = (seek + size - HeaderSize()) / blockSize;
	st.Start();
	Word32 resSize = size;
	Word32 resSeek = seek;
	for (Word32 i = startBlockNum; i <=	endBlockNum; i++)
	{
		if (!memBlocks[i])
			LoadMemBlock(i, curTime);
		Word32 res = memBlocks[i]->Load(st, resSeek, resSize, curTime);
		if (!res)
			throw DataOutOfRange();
		resSize -= res;
		resSeek += res;
		if (!resSize)
			break;
	}
	st.Begin();
	return 1;
}

void CachedStorageSystem::LoadMemBlock(Word32 blockNum, Time &curTime)
{
	Word32 startSeek = blockNum * blockSize + HeaderSize();
	Byte *mem = NULL;
	if (memoryBlockUse < maxMemoryBlockUse)
	{
		L(LOG_WARNING, "Create %u block from %u to %u (%u)\n", blockNum, startSeek, startSeek + blockSize, blockSize);
		mem =  (Byte*)malloc(blockSize);	
		memoryBlockUse++;
	}
	else
	{
		L(LOG_WARNING, "Reload %u block from %u to %u (%u)\n", blockNum, startSeek, startSeek + blockSize, blockSize);
		Word32 minAccTime = 0;
		Word32 minBlock = 0;
		Word32 i = 0;
		
		for (; i < maxBlockCount; i++)
			if (memBlocks[i])
			{
				minAccTime = memBlocks[i]->LastAccessTime();
				minBlock = i;
				break;
			}
		for (; i < maxBlockCount; i++)
			if (memBlocks[i] && memBlocks[i]->LastAccessTime() < minAccTime)
			{
				minAccTime = memBlocks[i]->LastAccessTime();
				minBlock = i;
			}
		L(LOG_WARNING, "Clear block %u (%u)\n", minBlock, minAccTime);
		mem = memBlocks[minBlock]->Mem();
		delete memBlocks[minBlock];
		memBlocks[minBlock] = NULL;
	}

	Word32 resSize = blockSize; // if end block
	if (blockSize + startSeek > _curSeek)
		resSize = _curSeek - startSeek;
	
	L(LOG_WARNING, "Start load data %u to block %u\n", resSize,  blockNum);
	if (!StorageSystem::Load(mem, startSeek, resSize))
		throw CannotReadData();
	L(LOG_WARNING, "End load\n");
	memBlocks[blockNum] = new MemBlock(mem, startSeek, resSize, curTime.Unix());
}
	
Word32 MemBlock::Load(Storage &st, Word32 seek, Word32 loadSize, Time &curTime)
{
	if (seek >= fromSeek && seek < fromSeek + size)
	{
		Word32 resSize = loadSize;
		if (seek + loadSize > fromSeek + size)
			resSize = (fromSeek + size) - seek;
		st.AddData(mem + (seek-fromSeek), resSize);
		lastAccess = curTime.Unix();
		return resSize;
	}
	return 0;
}


QueueCachedStorageSystem::QueueCachedStorageSystem(const char *fileName, const Word32 memSize)
	: StorageSystem(fileName), curMem(memSize)
{
	maxSeek = _curSeek;
	_curSeek = HeaderSize();
	curLoadSeek = _curSeek;
	LoadNextData();
}

void QueueCachedStorageSystem::GetData(Byte *data, Word32 size)
{
	while (size > 0)
	{
		Word32 sizeInMem = curMem.Size() - curMem.CurPos();
		if (size > sizeInMem)
		{
			curMem.GetData(data, sizeInMem);
			if (!LoadNextData())
				throw DataOutOfRange();
			size -= sizeInMem;
			data += sizeInMem;
		}
		else
		{
			curMem.GetData(data, size);
			break;
		}
	}
}

void QueueCachedStorageSystem::SkipData(Word32 size)
{
  while (size > 0)
  {
		Word32 sizeInMem = curMem.Size() - curMem.CurPos();
		if (size > sizeInMem)
		{
			curMem.Skip(sizeInMem);
			if (!LoadNextData())
				throw DataOutOfRange();
			size -= sizeInMem;
		}
		else
		{
			curMem.Skip(size);
			break;
		}
  }
}

int QueueCachedStorageSystem::LoadNextData()
{
	if (_curSeek < maxSeek)
	{
		curLoadSeek += curMem.Size();
		Word32 loadData = curMem.MaxSize()-1;
		Word32 lastLoad = maxSeek - _curSeek;
		if (loadData > lastLoad)
			loadData = lastLoad;
		if (!curMem.LoadFromFile(_file, loadData))
			throw CannotReadData();
//		L(LOG_WARNING, "[S %u] Load data from %u (%u)\n", storageID, curSeek, loadData);
		_curSeek += loadData;
		return 1;
	}
	else
		return 0;
}

	
