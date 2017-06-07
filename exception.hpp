#pragma once

#include <exception>
#include <string>

namespace lb
{
	class Exception
	{
	public:
		Exception(int type) { this->type = type;};
		int GetType() { return type; };
		virtual const char *GetStr() = 0;
		virtual ~Exception() {};
	protected:
		int type;
			
	};
	class WhatException : public std::exception
	{
	public:
		WhatException(const std::string &what)
			: _what(what)
		{ 
		};
		virtual ~WhatException() throw()
		{
		};
		virtual const char* what() const throw()
		{
			return _what.c_str();
		};
	protected:
		std::string _what;
	};
};

