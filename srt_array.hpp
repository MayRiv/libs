#pragma once

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <iostream>

namespace lb
{
	const int minDelta  = 16;

	typedef long int size_a;

	const int SRT_NOTFOUND = -1;
	template <class D> class SortedArrayIter;

	template <class D>
	class SortedArray
	{
		friend class SortedArrayIter<D>;
		public:
			SortedArray(size_a size)
			{
				maxSize = size;
				count = 0;
				elements = new D[size];
				delta = maxSize/10;
				if( delta < minDelta)
					delta = minDelta;
			};
			~SortedArray(){if(elements) delete elements;};
			size_a Add(D key);
			size_a Delete(D key);
			size_a Find(D key);
			size_a Count(){return count;};
			size_a ArraySize(){return maxSize;}
			void Print();
			D& operator []( int loc)
			{
				return elements[loc];
			}
		protected:
			D *elements;
			size_a count;
			size_a maxSize;
			size_a delta;
	};

	template <class D> class SortedArrayIter
	{
	public:
		SortedArrayIter()
		{
			from = 0;
			count = 0;
		}
		SortedArrayIter(const SortedArray<D> &array)
		{
			this->elements = array.elements;
			this->count = array.count;
			from = 0;
		}
		//ListPtrHash(const class ListPtrHash<D,K> &list);

		const SortedArray<D>	&operator = (const SortedArray<D> &array)
		{
			this->elements = array.elements;
			this->count = array.count;
			this->from = array.from;
		}
		operator  D *() const;
		D Value() const
		{
			return elements[from];
		}
		D operator-> () const
		{
			return elements[from];
		}
		void	Next()
		{
			from++;
		}

		operator	bool() const
		{
			return from < count;
		}

	protected:
		int from;
		D *elements;
		int count;
	};


	template <class D>
	size_a SortedArray<D>::Add(D key)
	{
		if(maxSize == count)
		{
			size_a tempMax = maxSize + delta;
			D *temp = new D[tempMax];
			//printf("Realocating... to %d\n", tempMax);
			memcpy(temp, elements, sizeof(D)*tempMax);
			delete elements;
			elements = temp;
			maxSize = tempMax;
		}
		if(count==0 || key > elements[count-1])
		{
			elements[count] = key;
			count++;
			return count-1;
		}
		else
		{
			register size_a lower = 0;
			register size_a upper = count - 1;
			register size_a middle;
			register D middleKey;
			while( lower <= upper )
			{
				middle = (lower+upper)/2;
				middleKey = elements[middle];
				if( middleKey < key )
					lower = middle + 1;
				else if( middleKey > key )
					upper = middle - 1;
				else
					return middle;
			}
			
	//		printf("nonASC order: key %ld lower el[%ld]=%ld\tmiddle el[%ld]=%ld\tupper el[%ld]=%ld\n", key, lower, 	elements[lower], middle, middleKey, upper, elements[upper]);

			if(upper<lower)
				lower = upper;
			lower++;
			memmove(&elements[lower+1], &elements[lower], (count - (lower))*sizeof(D));
			elements[lower] = key;
			count++;
			return count - 1;
		}
		return -1;
	}

	template <class D>
	size_a SortedArray<D>::Find(D key)
	{
		if( count != 0 )
		{
			register size_a lower = 0;
			register size_a upper = count - 1;
			while( lower <= upper )
			{
					register size_a middle = (lower+upper)/2;
					if( elements[middle] == key)
						return middle;
					if( elements[middle] < key)
						lower = middle + 1;
					else
						upper = middle - 1;
			}
		}
		return -1;
	}

	template <class D>
	size_a SortedArray<D>::Delete(D key)
	{
		size_a index = Find(key);
		if( index != -1 )
		{
			memmove(&elements[index], &elements[index+1], (count - (index + 1))*sizeof(D));
			count--;
		}
		return index;
	}

	template <class D>
	void SortedArray<D>::Print()
	{
		std::cout << "Total " << count << " elements (max " << maxSize << "*"<< sizeof(D)<<" bytes)\n";
		for(size_a i = 0; i < count; ++i)
		{
			std::cout<< elements[i] << ' ';
		}
		std::cout << "\n";
	}
};

