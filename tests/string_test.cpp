
#include "string.hpp"

#define BOOST_TEST_MODULE testString

#include <boost/test/unit_test.hpp>

using namespace lb;

BOOST_AUTO_TEST_SUITE( test1_string )

BOOST_AUTO_TEST_CASE(testRStringAddStr)
{
	char testBuf[] = "12345678901";
	RString buf(10);
	buf.AddStr(testBuf, 9);
	BOOST_CHECK(strlen(buf.Data()) == (unsigned int)buf.Length());
	BOOST_CHECK(buf.MaxSize() > 11);
}

BOOST_AUTO_TEST_CASE(testUnescapeChars)
{
	BOOST_TEST_MESSAGE("Test UnescapeChars method");
	char testString1[] = {0xD1,0x85,0xD0,0xB0,0xD1,0x80,0XD1,0x8C,0xD0,0xBA,0xD0,0xBE,0xD0,0xB2,0x0};
	
	char testString2[] = {0x04,0x40,0x04,0x43,0x04,0x41,0x04,0x41,0x04,0x3A,0x04,0x38,0x04,0x39,0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,0x04,0x42,0x0};
	
	char testString3[] = {0x04,0x40,0x04,0x43,0x04,0x41,0x04,0x41,0x04,0x3A,0x04,0x38,' ','w','a','s','d',' ',0x04,0x39,0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,0x04,0x42,0x0};
	
	char testString4[] = {0x04,0x40,0x04,0x43,0x04,0x41,0x04,0x41,0x04,0x04,0x38,0x04,0x39,0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,0x04,0x42,0x0};

	char testString5[] = {'%','u',0x04,0x43,0x04,0x04,0x41,'%','u','0',0x04,0x38,0x4,'3',0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,0x4,0x42,0x0};

	char testString6[] = {'%','u',0x04,0x43,0x04,0x04,0x41,'%','u','0',0x04,0x38,0x4,'3',0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,'%','u',0x0};
	char testString7[] = {'%','u',0x04,0x43,0x04,0x04,0x41,'%','u','0',0x04,0x38,0x4,'3',0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,'%','u','0',0x0};
	char testString8[] = {'%','u',0x04,0x43,0x04,0x04,0x41,'%','u','0',0x04,0x38,0x4,'3',0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,0x04,0x0};
	char testString9[] = {'%','u',0x04,0x43,0x04,0x04,0x41,'%','u','0',0x04,0x38,0x4,'3',0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,0x04,'4',0x0};

	char testString10[] = {0xD1,0x85,0xD0,0x04,0x40,0x04,0x43,0x04,0x41,0x04,0x41,0x04,0x3A,0x04,0x38,0x04,0x39,0x04,0x42,0x04,0x35,0x04,0x3A,0x04,0x41,0x04,0x42,0xBE,0xD0,0xB2,0x0};

	char testString11[] = {'a','b','c',0xD1,0x85,0x4,0x40,'%','8',0x40,'%','a','V','b','b',0x4,0x40,' ','a','a','%','4',0x0};

	String str1("%D1%85%D0%B0%D1%80%D1%8C%D0%BA%D0%BE%D0%B2");
	
	String str2("%u0440%u0443%u0441%u0441%u043A%u0438%u0439%u0442%u0435%u043A%u0441%u0442");
	
	String str3("%u0440%u0443%u0441%u0441%u043A%u0438 wasd %u0439%u0442%u0435%u043A%u0441%u0442");

	String str4("%u0440%u0443%u0441%u0441%u04%u0438%u0439%u0442%u0435%u043A%u0441%u0442");

	String str5("%u%u0443%u04%u0441%u0%u0438%u043%u0442%u0435%u043A%u0441%u0442");

	String str6("%u%u0443%u04%u0441%u0%u0438%u043%u0442%u0435%u043A%u0441%u");
	String str7("%u%u0443%u04%u0441%u0%u0438%u043%u0442%u0435%u043A%u0441%u0");
	String str8("%u%u0443%u04%u0441%u0%u0438%u043%u0442%u0435%u043A%u0441%u04");
	String str9("%u%u0443%u04%u0441%u0%u0438%u043%u0442%u0435%u043A%u0441%u044");

	String str10("%D1%85%D0%u0440%u0443%u0441%u0441%u043A%u0438%u0439%u0442%u0435%u043A%u0441%u0442%BE%D0%B2");

	String str11("abc%D1%85%u0440%8%u40%aVbb%u0440+aa%4");


	str1.UnescapeChars();
	BOOST_CHECK(strlen(testString1) == str1.GetLength());
	BOOST_CHECK(1 == str1.Cmp(testString1));

	str2.UnescapeChars();
	BOOST_CHECK(strlen(testString2) == str2.GetLength());
	BOOST_CHECK(1 == str2.Cmp(testString2));

	str3.UnescapeChars();
	BOOST_CHECK(strlen(testString3) == str3.GetLength());
	BOOST_CHECK(1 == str3.Cmp(testString3));

	str4.UnescapeChars();
	BOOST_CHECK(strlen(testString4) == str4.GetLength());
	BOOST_CHECK(1 == str4.Cmp(testString4));

	str5.UnescapeChars();
	BOOST_CHECK(strlen(testString5) == str5.GetLength());
	BOOST_CHECK(1 == str5.Cmp(testString5));

	str6.UnescapeChars();
	BOOST_CHECK(strlen(testString6) == str6.GetLength());
	BOOST_CHECK(1 == str6.Cmp(testString6));

	str7.UnescapeChars();
	BOOST_CHECK(strlen(testString7) == str7.GetLength());
	BOOST_CHECK(1 == str7.Cmp(testString7));

	str8.UnescapeChars();
	BOOST_CHECK(strlen(testString8) == str8.GetLength());
	BOOST_CHECK(1 == str8.Cmp(testString8));

	str9.UnescapeChars();
	BOOST_CHECK(strlen(testString9) == str9.GetLength());
	BOOST_CHECK(1 == str9.Cmp(testString9));

	str10.UnescapeChars();
	BOOST_CHECK(strlen(testString10) == str10.GetLength());
	BOOST_CHECK(1 == str10.Cmp(testString10));


	str11.UnescapeChars();
	BOOST_CHECK(strlen(testString11) == str11.GetLength());
	BOOST_CHECK(1 == str11.Cmp(testString11));
	
	String cyrtest("%D0%B7%D0%B0%D0%BF%D1%87%D0%B0%D1%81%D1%82%D0%B8,honda,%D1%85%D0%B0%D1%80%D1%8C%D0%BA%D0%BE%D0%B2");
	cyrtest.UnescapeChars();
	String decoded("запчасти,honda,харьков");
	BOOST_CHECK(decoded.GetLength() == cyrtest.GetLength());
	BOOST_CHECK(1 == cyrtest.Cmp(decoded));
	

}


BOOST_AUTO_TEST_SUITE_END()


	
