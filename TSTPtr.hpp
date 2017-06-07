#pragma once


#include <vector>

typedef struct TstNode* TNodePtr;

struct TstNode 
{
	TstNode( char c ) : splitChar(c), left(0), right(0), mid(0)
	{
	}
	char splitChar;
	TNodePtr left;
	TNodePtr right;
	union {
		TNodePtr mid;
		int index;
	};
};

template <class Object>
class TSTPtr
{
public:
	TSTPtr();
	TstNode *add(const char* key, Object *value);
	TstNode *addKey( const char* key, int &existingItemIndex);
	Object *getValue( const char * key )
	{
		int diff, sc = *key;
		TNodePtr p = _root;

		while (p) 
		{
			if ((diff = sc - p->splitChar) == 0) 
			{
				if (sc == 0) // found the key
					return _items[p->index];
				sc = *++key;
				p = p->mid;
			} 
			else if (diff < 0)
				p = p->left;
			else 
				p = p->right;
		}
		// if index -1, that means the search has run off the end of the tree, the key not found
		return NULL;
	}
	typedef std::vector<Object*> TItemVector;

	bool partialPrefixSearch(const char * key, TItemVector &res);
	const size_t size() const
	{
		return _items.size();
	}
private:
	void _partialMatchSearch( TNodePtr root, const char * key, TItemVector &res);
	TNodePtr _root;
	TItemVector _items;
};

template <class Object>
TSTPtr<Object>::TSTPtr()
	: _root(NULL)
{
}

template <class Object>
bool TSTPtr<Object>::partialPrefixSearch( const char * key, TItemVector &res)
{
	_partialMatchSearch( _root, key, res);
	return !res.empty();
}

template <class Object>
void TSTPtr<Object>::_partialMatchSearch(TNodePtr tree, const char *key, TItemVector &res)
{
	if (!tree) return;

	// partial match left
	if (*key == '\0' || *key < tree->splitChar)
	{
		_partialMatchSearch( tree->left, key, res );
	}
	// partial match middle
	if (*key == '\0' || *key == tree->splitChar)
	{
		if ( tree->splitChar )
		{
			if ( *key == '\0' )
			{
				_partialMatchSearch( tree->mid, key, res );
			}
			else
			{
				_partialMatchSearch( tree->mid, key + 1, res );	// search next pattern char
			}
		}
	}
	if ( (*key == 0) && tree->splitChar == 0 )
	{
		res.push_back(_items[ tree->	index ]);
	}

	if (*key == '\0' || *key > tree->splitChar)
	{
		_partialMatchSearch( tree->right, key, res );
	}
}

template <class Object>
TstNode *TSTPtr<Object>::add( const char* key, Object *value)
{
	int existingItemIndex  = -1;
	TNodePtr p = addKey( key, existingItemIndex );
	if ( p )
	{
		if (existingItemIndex == -1)
		{	
			_items.push_back( value );
			p->index = _items.size() - 1;
		}
		else
		{
			_items[ existingItemIndex ] = value;
			p->index = existingItemIndex;

		}
	}
	return p;
}

template <class Object>
TstNode* TSTPtr<Object>::addKey( const char* key, int &existingItemIndex)
{
	TNodePtr p = _root;
	TNodePtr parent = 0;
	if( key == 0 || *key == 0)
		return NULL;

	while (p) 
	{
		parent = p;
		if ( *key < p->splitChar )
		{
			p = p->left;
		}
		else if ( *key == p->splitChar )  
		{
			if ( *key == 0 )
			{
				existingItemIndex = p->index;
				break;
			}
			p = p->mid;
			++key;
		} 
		else
		{
			p = p->right;
		}
	}


	if( !p )
	{
		existingItemIndex = -1;
		p = new TstNode( *key );
		if ( parent )
		{
			int diff = *key - parent->splitChar;
			diff == 0 ? parent->mid = p : diff < 0 ? parent->left = p : parent->right = p;
		}
		if ( ! _root )
		{
			_root = p;
		}
		while ( p->splitChar )
		{
			++key;
			p->mid = new TstNode( *key );
			p = p->mid;
		}
	}
	return p;
}
