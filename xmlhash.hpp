#pragma once

#include <xmlwrapp/xmlwrapp.h>
#include <map>
#include <string>
#include <stdlib.h>
#include "icharset.hpp"
#include "string.hpp"
#include "log.hpp"

namespace lb
{
	class XmlHash : public xml::event_parser
	{
	public:
		XmlHash()
		{
		}
		typedef std::map<std::string, std::string> TNodeHash;
		const char *GetNode(const std::string &node)
		{
			TNodeHash::iterator p = nodes.find(node);
			if (p != nodes.end())
    		return p->second.c_str();
			else
    		return NULL;
		}
		const int GetIntNode(const std::string &node)
		{
			TNodeHash::iterator p = nodes.find(node);
			if (p != nodes.end())
    		return atoi(p->second.c_str());
			else
    		return 0;
		}

		const Word64 GetWord64Node(const std::string &node)
		{
			TNodeHash::iterator p = nodes.find(node);
			if (p != nodes.end())
    		return strtoull(p->second.c_str(),NULL, 10);
			else
    		return 0;
		}
		const Word64 GetWord64Node16(const std::string &node)
		{
			TNodeHash::iterator p = nodes.find(node);
			if (p != nodes.end())
    		return strtoull(p->second.c_str(),NULL, 16);
			else
    		return 0;
		}

		void SetEncoding(int code)
		{
			outputEncoding = code;
		}
		void Clear()
		{
			nodes.clear();
			curNode = "";
			fullText.Null();
			parse_finish();
		}
	private:
		XmlHash(const XmlHash&);
		bool start_element (const std::string &name, const attrs_type&)
		{ 
			if (curNode.size() > 0 && fullText.Length() > 0)
				end_element (curNode);
  		curNode = name;
  		return true; 
		}
	  
		bool end_element (const std::string &name)
		{ 
			if (fullText.Length() > 0)
			{
				char *strtext = (char*)fullText.Data();
				int len = fullText.Length();
	#ifndef __NOT_UTF8_LIB_XML
				RString outBuf(fullText.Length()+2);
				len = IConvert(strtext, fullText.Length(), outBuf.GetBuf(fullText.Length()), fullText.Length(), CONTENT_CHARSET_UTF8, outputEncoding);
				if (!len)
				{
					L(LOG_WARNING, "UTF8 decode error\n");
					return false;
				}
				outBuf.SetLength(len);
				strtext = outBuf;
	#else
				fullText.AddEndZero();
	#endif
				std::string outText(strtext, len);
				std::pair<TNodeHash::iterator, bool> res = nodes.insert(TNodeHash::value_type(curNode, outText));
				if (!res.second)
					res.first->second = outText;
				fullText.Null();
			}
  		curNode = "";
  		return true;
		}
	  
		bool text (const std::string& text)
		{ 
			if (curNode.size() > 0)
				fullText.Add(text.c_str(), text.size());
  		return true; 
		}
		virtual bool cdata (const std::string &contents)
		{
			if (curNode.size() > 0)
				fullText.Add(contents.c_str(), contents.size());
			return true;
		}
		virtual bool comment (const std::string &contents)
		{
			return true;
		}
		virtual bool processing_instruction (const std::string &target, const std::string &data)
		{
			return true;
		}
		virtual bool warning (const std::string &message)
		{
			return true;
		}

		std::string curNode;
		TNodeHash nodes;
		RString fullText;
		int outputEncoding;
	};
};

