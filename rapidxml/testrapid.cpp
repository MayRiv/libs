#include "rapidxml.hpp"
#include <stdio.h>

int main( void )
{
		static char TEST_XML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<request> <test>1</test><data>Fak Fak</data></request>";
	
		using namespace rapidxml;
		xml_document<> doc;    // character type defaults to char
		doc.parse<parse_no_utf8>(TEST_XML);    // 0 means default parse flags	
		printf("firstNode: %s\n", doc.first_node()->name());
		
		xml_node<> *test = doc.first_node()->first_node("test");
		xml_node<> *data = doc.first_node()->first_node("data");
		printf("%u\n",  atoi(test->value()));
		printf("%s\n", data->value());
}
