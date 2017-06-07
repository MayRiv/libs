#pragma once

#include <algorithm>
namespace lb
{

	using std::swap;

	template <class D> void Group(D *data, int count)
	{
		int k;
		for (int i = 0; i < count; i+=k)
		{
			k = 1;
			for (int j = i+1; j < count; j++)
				if (data[i] == data[j])
				{
					if (j != (i+k))
					{
						D t = data[i+k]; 
						data[i+k] = data[j];
						data[j] = t;
					}
					k++;
				}
		}
	     
	}



	template <class Comparable>
	class Pointer
	{
	public:
		Pointer( Comparable *rhs = NULL ) : pointee( rhs ) { }

		bool operator<( const Pointer & rhs ) const
			{ return *pointee < *rhs.pointee; }
		operator Comparable * ( ) const
			{ return pointee; }
		Comparable * operator->() const
		{
			return pointee;
		}
	private:
		Comparable *pointee;
	};

	/**
	* Return median of left, center, and right.
	* Order these and hide the pivot.
	*/
	template <class Comparable, class Container>
	const Comparable & median3( Container & a, int left, int right )
	{
			int center = ( left + right ) / 2;
			if( a[ center ] < a[ left ] )
					swap( a[ left ], a[ center ] );
			if( a[ right ] < a[ left ] )
					swap( a[ left ], a[ right ] );
			if( a[ right ] < a[ center ] )
					swap( a[ center ], a[ right ] );

					// Place pivot at position right - 1
			swap( a[ center ], a[ right - 1 ] );
			return a[ right - 1 ];
	}
	template <class Comparable, class Container>
	void insertionSort( Container & a, int left, int right )
	{
			for( int p = left + 1; p <= right; p++ )
			{
					Comparable tmp = a[ p ];
					int j;

					for( j = p; j > left && tmp < a[ j - 1 ]; j-- )
							a[ j ] = a[ j - 1 ];
					a[ j ] = tmp;
			}
	}

	/**
	* Internal quicksort method that makes recursive calls.
	* Uses median-of-three partitioning and a cutoff of 10.
	* a is an array of Comparable items.
	* left is the left-most index of the subarray.
	* right is the right-most index of the subarray.
	*/
	template <class Comparable, class Container>
	void quicksort( Container & a, int left, int right )
	{
			if( left + 10 <= right )
			{
					Comparable pivot = median3<Comparable, Container>( a, left, right );

							// Begin partitioning
					int i = left, j = right - 1;
					for( ; ; )
					{
							while( a[ ++i ] < pivot ) { }
							while( pivot < a[ --j ] ) { }
							if( i < j )
									swap( a[ i ], a[ j ] );
							else
									break;
					}

					swap( a[ i ], a[ right - 1 ] );  // Restore pivot

					quicksort<Comparable, Container>( a, left, i - 1 );     // Sort small elements
					quicksort<Comparable, Container>( a, i + 1, right );    // Sort large elements
			}
			else  // Do an insertion sort on the subarray
					insertionSort<Comparable, Container>( a, left, right );
	}
	template <class Comparable, class Container>
	void quicksort( Container & a, const int size )
	{
			quicksort<Comparable, Container>( a, 0, size - 1 );
	}

	template <class Comparable, class Container>
	void quicksort( Container & a )
	{
			quicksort<Comparable, Container>( a, 0, a.size( ) - 1 );
	}



	const unsigned long seqseq[] = {1, 5, 19, 41, 109, 209, 505, 929, 2161, 3905, 8929, 16001, 36289, 64769, 146305, 260609, 587521, 1045505, 2354689, 4188161, 9427969, 16764929, 37730305, 67084289, 150958081, 268386305, 603906049, 1073643521};

	inline int incr(long size) {
	 /* int p1, p2, p3, s;

		p1 = p2 = p3 = 1;
		s = -1;
		do {
			if (++s % 2) {
				inc[s] = 8*p1 - 6*p2 + 1;
			} else {
				inc[s] = 9*p1 - 9*p3 + 1;
				p2 *= 2;
				p3 *= 2;
			}
		p1 *= 2;
		} while(3*inc[s] < size);  */
		int s = -1;
		do
		{
  		s++;
		} while(3*seqseq[s] < (unsigned long )size);
		return (s > 0) ? --s : 0;
	}

	template<class T>
	void shellSort(T a[], long size) {
		long inc, i, j;
		int s;

		// вычисление последовательности приращений
		s = incr(size);
		while (s >= 0) {
		// сортировка вставками с инкрементами inc[] 
		inc = seqseq[s--];

			for (i = inc; i < size; i++) {
				T temp = a[i];
				for (j = i-inc; (j >= 0) && (a[j]->More(temp)); j -= inc)
					a[j+inc] = a[j];
				a[j+inc] = temp;
			}
		}
	}

	template<class T>
	void shellSortRef(T a[], long size) {
		long inc, i, j;
		int s;

		// вычисление последовательности приращений
		s = incr(size);
		while (s >= 0) {
		// сортировка вставками с инкрементами inc[] 
		inc = seqseq[s--];

			for (i = inc; i < size; i++) {
				T temp = a[i];
				for (j = i-inc; (j >= 0) && (a[j].More(temp)); j -= inc)
					a[j+inc] = a[j];
				a[j+inc] = temp;
			}
		}
	}
	template<class T>
	void InsertSort(T *a, long size) {
			T x;
			long i, j;
			for (i = 0; i < size; i++) 
			{  // цикл проходов, i - номер прохода
  			x = a[i];
				// поиск места элемента в готовой последовательности 
    		for (j=i-1; j>=0 && a[j]->More(x); j--)
      		a[j+1] = a[j];  	// сдвигаем элемент направо, пока не дошли
				// место найдено, вставить элемент
    		a[j+1] = x;
  		}
	}
};


