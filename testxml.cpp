#include "xml.hpp"
#include <stdio.h>

using namespace lb;

int main( void )
{
		static char TEST_XML[] = "<request> <test>1</test><data><fak>MegaFak</fak>[16:31:53] Vitalii: <![CDATA[test1 <![CDATA[test1]]]]><![CDATA[> test2]]></data></request>";

		XmlNodeMap nm;
		{
			XmlParser parser;
			if (!parser.Parse(TEST_XML))
			{
				printf("Error XML parse\n");
				return -1;
			}
			if (!parser.GetNodeMap(nm))
			{
				printf("Can't get node map\n");
				return -1;
			}

		}
		
	
		printf("%u\n",  nm.GetIntNode("test"));
		printf("%s\n", nm.GetNode("data"));
}
