#pragma once

namespace lb
{

	template <class T>
	class SmartPtr
	{
	public:
		SmartPtr() 
			: _ptr(NULL)
		{
		};
		SmartPtr(T *t)
			: _ptr(NULL)
		{
			 _New(t);
		};
	  
		SmartPtr(const SmartPtr<T> &cl)
			: _ptr(NULL)
		{
			 _Inc(cl._ptr);
		};
	  
		~SmartPtr()
		{
			 _Delete();
		};
	  
		SmartPtr & operator = (T *t)
		{
			if (t != NULL)
				_New(t);
			else
				_Delete();
			return *this;
		};
	  
		SmartPtr & operator = (const SmartPtr<T> & cl)
		{
				_Inc(cl._ptr);
				return *this;
		};
	  
		T& operator *()
		{
			 return *_ptr->ptr;
	     
		};
	  
		T *operator ->() const
		{
			 return _ptr->ptr;         
		};
	  
		T *get() const
		{
			if (_ptr)
				return _ptr->ptr;         
			else
				return NULL;
		};
	private:
		struct Ptr
		{
			 Ptr(T *ptr)
				 : ptr(ptr), count(1)
			 {
			 }
			 T *ptr;
			 int count;
		};
		Ptr *_ptr;
	  
		void _Delete()
		{
			 if (_ptr != NULL) 
			 {
					_ptr->count--;
					if (_ptr->count == 0) 
					{
						 delete _ptr->ptr;
						 delete _ptr;
					}
					_ptr = NULL;
			 }
		};
	  
		void _New(T *ptr)
		{ 
			 _Delete();
			 _ptr = new Ptr(ptr);
		};
	  
		void _Inc(Ptr *t)
		{
			 _Delete();
			 _ptr = t;
			 if (_ptr != NULL) 
				 _ptr->count++;
		}
	};
};
