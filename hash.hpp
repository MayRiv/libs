#pragma once

#include <string.h>

namespace lb
{
	const int MAX_KEY_POWER				= 24;
	const unsigned int HASH_FREE				= 0xFFFFFFF0;
	const int MIN_HASH_KEYCOUNT		= 2;

	template <class D> class Hash
	{
	public:
		Hash(unsigned int cout);
		Hash(D *keys, const unsigned int count);
		~Hash();
		void Detach()
		{
			keys = NULL;
		}
		int Add(D k);
		D Find(D k);
		D H(D k, unsigned int i);
		D HF(D k);
		D *Data() { return keys; }
		void Flush();
		void FlushAndResize(int count);
		const unsigned int Count() const { return c; }
		int CountKeys() { return countKeys; }
		static const unsigned int easykeys[];
	protected:
		void ReAllock(unsigned int cout);
		D *keys;
		unsigned int countKeys;
		unsigned int nextCountKeys;
		unsigned int c;
	};


	const int MAX_EASY_KEYS  = 48;

	template <class D>
	const unsigned int Hash<D>::easykeys[MAX_EASY_KEYS] = 
	{
		3,       5,      11,      23,      47,      97,
		197,     797,     1597,    3203,    6421,
		12853,   25717,   51437,   102877,  205759,  411527,
		823117,  1646237, 3292489, 4320469, 6925811, 8943859,
		10212373, 12454517, 15092911, 19085123, 25770343, 30188759,
		35062843, 40160597, 45161561, 50408377,	55199569,	61082393,
		65116343, 72126787,	77119013,	81978473,	87385421,	99999989
	};

	template <class D, class K> class ListPtrHash;

	template <class D, class K> class PtrHash : public Hash<D>
	{
	public:
		PtrHash(unsigned int cout);
		PtrHash();
		int Add(D d,  K key);
		D Set(D d, K key);
		D Delete(K k);
		D Find(K k);
		D *GetData() { return Hash<D>::keys; }
		unsigned int sumi;
		void Rebuild();
		void Rebuild(int removedElementsCount);
		friend class ListPtrHash<D,K>;
	protected:
		K H(K k, unsigned int i);
		K HF(K k);
		void ReAllock(unsigned int cout);
	};
	template <class D, class K> class ListPtrHash
	{
	public:
		ListPtrHash()
		{
			from = 0;
			count = 0;
			data = NULL;
			keys = NULL;
		}
		ListPtrHash(const PtrHash<D,K> &hash)
		{
			this->keys = hash.keys;
			this->data = NULL;
			this->count = hash.countKeys;
			from = 0;
			for (int i = 0; i < count; i++)
			{
				if (keys[i])
				{
					data = keys[i];
					from = i+1;
					return;
				}
			}
		}
		ListPtrHash(const class ListPtrHash<D,K> &list);

		const ListPtrHash<D,K>	&operator = (const ListPtrHash<D,K> &list)
		{
			this->keys = list.keys;
			this->count = list.GetCountKeys();
			this->data = list.data;
			this->from = list.from;
		}
		operator  D *() const;
		D Ptr() const
		{
			return data;
		}
		D operator-> () const
		{
			return data;
		}
		void	Next()
		{
			for (int i = from; i < count; i++)
			{
				if (keys[i])
				{
					data = keys[i];
					from = i+1;
					return;
				}
			}
			data = NULL;
		}
		void Null()
		{
			keys[from-1] = NULL;
		}
		operator	bool() const
		{
			return data != NULL;
		}

	protected:
		int from;
		D *keys;
		D data;
		int count;
	};

	template <class D>
	Hash<D>::Hash(D *keys, const unsigned int count)
	{
		for (int i = 0; i < MAX_EASY_KEYS-1; i++)
			if (count < easykeys[i])
			{
				countKeys = easykeys[i];
				nextCountKeys = easykeys[i]-1;
				break;
			}
		this->keys = keys;
		c = 0;
	}

	template <class D>
	Hash<D>::Hash(unsigned int count)
		: keys(NULL)
	{
		for (int i = 0; i < MAX_EASY_KEYS-1; i++)
			if (count < easykeys[i])
			{
				countKeys = easykeys[i];
				nextCountKeys = easykeys[i]-1;
				break;
			}
		keys = new D[countKeys+1];
		memset(keys, 0, sizeof(D) * (countKeys+1));
		c = 0;
	};

	template <class D, class K>
	PtrHash<D, K>::PtrHash(unsigned int count)
		:Hash<D>(count)
	{
		
	};

	template <class D, class K>
	PtrHash<D, K>::PtrHash()
			:Hash<D>(MIN_HASH_KEYCOUNT)
	{                                                                                                               
	}; 

	template <class D>
	Hash<D>::~Hash()
	{
		delete [] keys;	
	};


	template <class D>
	void Hash<D>::Flush()
	{
		c = 0;
		memset(keys, 0, sizeof(D) * (countKeys+1));
	};
	template <class D>
	void Hash<D>::FlushAndResize(int count)
	{
		c = 0;
		Hash<D> hash(count);
		D *oldkeys = keys;
		keys = hash.keys;
		countKeys = hash.countKeys;
		nextCountKeys = hash.nextCountKeys;
		c = hash.c;
		hash.keys = oldkeys;
	}

	template <class D>
	int Hash<D>::Add(D key)
	{
		if (!key)
			return 0;
		if (c >= countKeys-1)
				ReAllock(countKeys);
		register unsigned int i = 0;
		D hashKey = HF(key);
		D p = keys[hashKey];
		if (p && (p == key))
			return 0;
		while (p)
 		{
    		i++;
				/*
				if (i>countKeys*2)
				{
				log.fLog("Error in hash %u, %llx(%llu), %u|%u|%llx|\n", c, (Word64)key, (Word64) hashKey, (Word32)countKeys, (Word32)nextCountKeys, i, p);
				log.fLog("Count %u/%u\n", countKeys, c);
				exit(-1);
				}
				*/
				hashKey = H(key, i);
				p = keys[hashKey];
				if (p == key)
				return 0;
		}
		keys[hashKey] = key;
		c++;
		return 1;		
	};
	template <class D>
	D Hash<D>::Find(D key)
	{
		if (!key)
				return 0;
		unsigned int i = 0;
		D hashKey = HF(key);
		D p = keys[hashKey];
		if (p && (p == key))
			return p;
		while (p)
		{
			i++;
			/*
			if (i>countKeys)
			{
				log.fLog("Error in hash %u %u|%u\n", key, countKeys, nextCountKeys);
				exit(-1);
			}
			*/
			hashKey = H(key, i);
			p = keys[hashKey];
			if (p == key)
				return p;
		}
		return 0;
	}

	template <class D>
	void Hash<D>::ReAllock(unsigned int count)
	{
		Hash<D> hash(count);
		for (unsigned int i = 0; i <= countKeys; i++)
			hash.Add(keys[i]);
		D *oldkeys = keys;
		keys = hash.keys;
		countKeys = hash.countKeys;
		nextCountKeys = hash.nextCountKeys;
		c = hash.c;
		hash.keys = oldkeys;
	}

	template <class D, class K>
	D PtrHash<D, K>::Find(K k)
	{
		register int i = 0;
		register K hashKey = HF(k);
		register D p = Hash<D>::keys[hashKey];
		while (1)
		{
			if (!p)
				 return NULL;
	/*		if (p == (D)HASH_FREE)
			{
				i++;
				continue;
			}*/
			if (p->Key() == k)
				return Hash<D>::keys[hashKey];
			i++;
			hashKey = H(k, i);
			p = Hash<D>::keys[hashKey];
		}
		return Hash<D>::keys[hashKey];
													
	};

	template <class D, class K>
	void PtrHash<D, K>::ReAllock(unsigned int count)
	{
		PtrHash<D, K> hash(count);
		for (unsigned int i = 0; i <= Hash<D>::countKeys; i++)
		 if (Hash<D>::keys[i])
			 hash.Add(Hash<D>::keys[i], Hash<D>::keys[i]->Key());
		D *oldkeys = Hash<D>::keys;
		Hash<D>::keys = hash.keys;
		Hash<D>::countKeys = hash.countKeys;
		Hash<D>::nextCountKeys = hash.nextCountKeys;
		Hash<D>::c = hash.c;
		hash.keys = oldkeys;
	}

	template <class D>
	inline D Hash<D>::H(D k, unsigned int i)
	{
		return ((k % countKeys + i*((k % nextCountKeys)+1)) % countKeys);
	};

	template <class D>
	inline D Hash<D>::HF(D k)
	{
		return k % countKeys;
	};

	template <class D, class K>
	inline K PtrHash<D, K>::H(K k, unsigned int i)
	{
			return ((k % Hash<D>::countKeys + i*((k % Hash<D>::nextCountKeys)+1)) % Hash<D>::countKeys);
	};

	template <class D, class K>
	inline K PtrHash<D, K>::HF(K k)
	{
		return k % Hash<D>::countKeys;
	};

	template <class D, class K>
	inline D PtrHash<D, K>::Delete(K k)
	{
		register unsigned int i = 0;
		register K hashKey = HF(k);
		register D p = Hash<D>::keys[hashKey];
		if (!p/* || p == (D)HASH_FREE*/)
			return NULL;
		while (p->Key() != k)
		{
			i++;
			hashKey = H(k, i);
			p = Hash<D>::keys[hashKey];
			if (!p /*|| p == (D)HASH_FREE*/)
				return NULL;
			if (p->Key() == k)
				break;
		}
		D data=Hash<D>::keys[hashKey];
		Hash<D>::keys[hashKey] = NULL;
		Rebuild();
	 /*
		PtrHash<D,K> hash(countKeys-1);
		int key;
		for (i = 0; i <= countKeys; i++)
			if (keys[i] && ((key=keys[i]->Key())!=k))
				hash.Add(keys[i], key);
	//  log.fLog("Delete %u, %u, %u\n", c, hash.Count(), hash.c);
	  
		keys = hash.keys;
		countKeys = hash.countKeys;
		nextCountKeys = hash.nextCountKeys;
		c = hash.c;
		hash.keys = NULL;
		*/          
		/*keys[hashKey] = (D)HASH_FREE;*/
		return data;
	}

	template <class D, class K>
	inline void PtrHash<D, K>::Rebuild()
	{
		PtrHash<D,K> hash(Hash<D>::countKeys-1);
		for (unsigned int i = 0; i < Hash<D>::countKeys; i++)
			if (Hash<D>::keys[i])
				hash.Add(Hash<D>::keys[i], Hash<D>::keys[i]->Key());
		D *oldkeys = Hash<D>::keys;
		Hash<D>::keys = hash.keys;
		Hash<D>::countKeys = hash.countKeys;
		Hash<D>::nextCountKeys = hash.nextCountKeys;
		Hash<D>::c = hash.c;
		hash.keys = oldkeys;
	}

	template <class D, class K>
	inline void PtrHash<D, K>::Rebuild(int removedElementsCount)
	{
		int newCountKeys = Hash<D>::c;
		if (newCountKeys > removedElementsCount)
			newCountKeys -= removedElementsCount;
		else
			newCountKeys = MIN_HASH_KEYCOUNT;

		PtrHash<D,K> hash(newCountKeys);
		for (unsigned int i = 0; i < Hash<D>::countKeys; i++)
			if (Hash<D>::keys[i])
				hash.Add(Hash<D>::keys[i], Hash<D>::keys[i]->Key());
		D *oldkeys = Hash<D>::keys;
		Hash<D>::keys = hash.keys;
		Hash<D>::countKeys = hash.countKeys;
		Hash<D>::nextCountKeys = hash.nextCountKeys;
		Hash<D>::c = hash.c;
		hash.keys = oldkeys;
	}

	template <class D, class K>
	inline D PtrHash<D, K>::Set(D d, K key)
	{
		register unsigned int i = 0;
		register K hashKey = HF(key);
		register D p = Hash<D>::keys[hashKey];
		if (!p)
			return NULL;
		while (p->Key() != key)
		{
			i++;
			hashKey = H(key, i);
			p = Hash<D>::keys[hashKey];
			if (!p)
				return NULL;
			if (p->Key() == key)
				break;
		}
		D data=Hash<D>::keys[hashKey];
		Hash<D>::keys[hashKey] = d;
		return data;
	}

	template <class D, class K> 
	inline int PtrHash<D, K>::Add(D d, K key)
	{
		if (Hash<D>::c >= Hash<D>::countKeys-1)
			ReAllock(Hash<D>::countKeys);
		register int i = 0;
		register K hashKey = HF(key);
		register D p = Hash<D>::keys[hashKey];
		if (p && /*p != (D)HASH_FREE &&*/ (p->Key() == key))
			return 0;
		while (p/* && p != (D)HASH_FREE*/)
		{
			i++;
			hashKey = H(key, i);
			p = Hash<D>::keys[hashKey];
			if (!p/* || p == (D)HASH_FREE*/)
				break;
			if (p->Key() == key)
				return 0;
	//		if (i >= countKeys)
	//			printf("DDDDDDD");
		};
		Hash<D>::keys[hashKey] = d;
		Hash<D>::c++;	
	//	sumi += (i+1);
		return 1;
	};
};

