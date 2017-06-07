#pragma once

#include "rapidxml/rapidxml.hpp"
#include <map>
#include <string>
#include "string.hpp"

namespace lb
{
	using namespace rapidxml;	
	class XmlNodeMap
	{
	public:
		XmlNodeMap()
		{
		}
		void Insert(const std::string &name, const std::string &value);
		int GetIntNode(const std::string &name, const int defaultValue = 0);
		Word64 GetWord64Node(const std::string& name, const Word64 defaultValue = 0);
		const char *GetNode(const std::string &name, const char * const defaultValue = "");
	private:
		typedef std::map<std::string, std::string> TNodeMap;
		TNodeMap _nodes;
	};

	class XmlParser
	{
	public:
		XmlParser()
		{
		}

		bool Parse(char *data);
		bool GetNodeMap(XmlNodeMap &nodes);
	private:
		xml_document<> _doc;
	};

	class XmlTree
	{
	public:
		static const char *const Value(xml_node<> *node, RString &buf);
	};

};
