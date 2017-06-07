#include "xml.hpp"
#include "string.hpp"
using namespace lb;
using namespace rapidxml;

bool XmlParser::Parse(char *data)
{
	try
	{
		_doc.parse<parse_no_utf8>(data);
		return true;
	}
	catch (...)
	{
	}
	return false;
}

bool XmlParser::GetNodeMap(XmlNodeMap &nodes)
{
	xml_node<> *firstNode = _doc.first_node();
	if (firstNode == NULL)
		return false;
	nodes.Insert(std::string(firstNode->name(), firstNode->name_size()), std::string(" "));

	xml_node<> *node = firstNode->first_node();
	if (node == NULL)
		return false;

	do
	{
		if (node->type() == node_element)
		{
			xml_node<> *childNode = node->first_node();
			if (childNode)
			{
				if (childNode->next_sibling() != NULL)
				{
					RString val(childNode->value_size());
					do
					{
						if ((childNode->type() != node_cdata) && (childNode->type() != node_data))
							continue;

						if (childNode->value_size() > 0)
							val.AddStr(childNode->value(), childNode->value_size());
					}
					while ((childNode = childNode->next_sibling()) != NULL);
					if (val.Length() > 0)
						nodes.Insert(std::string(node->name(), node->name_size()), std::string(val.Data(), val.Length()));
				}
				else if (childNode->value_size() > 0)
				{
					if ((childNode->type() == node_cdata) || (childNode->type() == node_data))
						nodes.Insert(std::string(node->name(), node->name_size()), std::string(childNode->value(), childNode->value_size()));
				}
			}
			else if (node->value_size() > 0)
				nodes.Insert(std::string(node->name(), node->name_size()), std::string(node->value(), node->value_size()));
		}
	}
	while ((node = node->next_sibling()) != NULL);
	return true;
}

int XmlNodeMap::GetIntNode(const std::string &name, const int defaultValue)
{
	TNodeMap::iterator f = _nodes.find(name);
	if (f == _nodes.end())
		return defaultValue;
	else
		return atoi(f->second.c_str());
}

Word64 XmlNodeMap::GetWord64Node(const std::string &name, const Word64 defaultValue)
{
	TNodeMap::iterator f = _nodes.find(name);
	if (f == _nodes.end())
		return defaultValue;
	else
		return strtoull(f->second.c_str(),NULL, 10);
}

const char *XmlNodeMap::GetNode(const std::string &name, const char * const defaultValue)
{
	TNodeMap::iterator f = _nodes.find(name);
	if (f == _nodes.end())
		return defaultValue;
	else
		return f->second.c_str();
}

void XmlNodeMap::Insert(const std::string &name, const std::string &value)
{
	std::pair<TNodeMap::iterator, bool> res = _nodes.insert(TNodeMap::value_type(name, value));
	if (!res.second)
		res.first->second = value;
}


const char *const XmlTree::Value(xml_node<> *node, RString &buf)
{
	if (node->type() != node_element)
		return "";
	xml_node<> *childNode = node->first_node();
	if (childNode)
	{
		if (childNode->next_sibling() != NULL)
		{
			buf.Null();
			do
			{
				if ((childNode->type() != node_cdata) && (childNode->type() != node_data))
					continue;

				if (childNode->value_size() > 0)
					buf.AddStr(childNode->value(), childNode->value_size());
			}
			while ((childNode = childNode->next_sibling()) != NULL);
			if (buf.Length() > 0)
				return buf.Data();
		}
		else if (childNode->value_size() > 0)
		{
			if ((childNode->type() == node_cdata) || (childNode->type() == node_data))
				return childNode->value();
		}
	}
	else 
		return node->value();
	return "";
}
