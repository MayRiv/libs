#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iconv.h>


#include "charset.hpp"
#include "seqkoi.hpp"
#include "string.hpp"
#include "icharset.hpp"
#include "tools.hpp"

extern const char *seq[];

namespace lb
{

#define UNSIG_CHAR_PTR(x) ((unsigned char *)(x))

static unsigned char to_b64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef unsigned char _cyr_charset_table[512];

/* {{{ const static _cyr_charset_table _cyr_win1251
 */
const static _cyr_charset_table _cyr_win1251 = {
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
//46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,
46,46,'\'',46,46,46,46,46,46,46,46,46,46,46,46,46,
154,174,190,46,159,189,46,46,179,191,180,157,46,46,156,183,
46,46,182,166,173,46,46,158,163,152,164,155,46,46,46,167,
225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,184,186,32,179,191,32,32,32,32,32,180,162,32,
32,32,32,168,170,32,178,175,32,32,32,32,32,165,161,169,
254,224,225,246,228,229,244,227,245,232,233,234,235,236,237,238,
239,255,240,241,242,243,230,226,252,251,231,248,253,249,247,250,
222,192,193,214,196,197,212,195,213,200,201,202,203,204,205,206,
207,223,208,209,210,211,198,194,220,219,199,216,221,217,215,218,
},
_cyr_cp866 = { 
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
35,35,35,124,124,124,124,43,43,124,124,43,43,43,43,43,
43,45,45,124,45,43,124,124,43,43,45,45,124,45,43,45,
45,45,45,43,43,43,43,43,43,43,43,35,35,124,124,35,
210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
179,163,180,164,183,167,190,174,32,149,158,32,152,159,148,154,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
205,186,213,241,243,201,32,245,187,212,211,200,190,32,247,198,
199,204,181,240,242,185,32,244,203,207,208,202,216,32,246,32,
238,160,161,230,164,165,228,163,229,168,169,170,171,172,173,174,
175,239,224,225,226,227,166,162,236,235,167,232,237,233,231,234,
158,128,129,150,132,133,148,131,149,136,137,138,139,140,141,142,
143,159,144,145,146,147,134,130,156,155,135,152,157,153,151,154,
},
_cyr_iso88595 = {
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,179,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
32,163,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,241,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,161,32,32,32,32,32,32,32,32,32,32,32,32,
238,208,209,230,212,213,228,211,229,216,217,218,219,220,221,222,
223,239,224,225,226,227,214,210,236,235,215,232,237,233,231,234,
206,176,177,198,180,181,196,179,197,184,185,186,187,188,189,190,
191,207,192,193,194,195,182,178,204,203,183,200,205,201,199,202,
},
_cyr_mac = {
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
144,145,146,147,148,149,150,151,152,153,154,155,156,179,163,209,
193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,255,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
160,161,162,222,164,165,166,167,168,169,170,171,172,173,174,175,
176,177,178,221,180,181,182,183,184,185,186,187,188,189,190,191,
254,224,225,246,228,229,244,227,245,232,233,234,235,236,237,238,
239,223,240,241,242,243,230,226,252,251,231,248,253,249,247,250,
158,128,129,150,132,133,148,131,149,136,137,138,139,140,141,142,
143,159,144,145,146,147,134,130,156,155,135,152,157,153,151,154,
};

const unsigned char koi8_tolower[256] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,                                                      97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,                                                                        129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,163,164,181,166,167,184,185,186,187,188,173,190,191,192,                                                            193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222};
char *decodeTable[256];

void InitDetectCode()
{
	memset(decodeTable, 0, sizeof(decodeTable));
	int i = 0;
	while (seq[i])
	{
		unsigned char c = seq[i][0];
		if (!decodeTable[c])
		{
			decodeTable[c] = new char[255];		
			memset(decodeTable[c], 0, 255);
		}
		unsigned char c1 = seq[i][1];
		decodeTable[c][c1] = 1;
		i++;	
	}
}

int DetectCharset(char *str, int len, char from)
{
	int ball = 0;
	const unsigned char *from_table = NULL;
	
	switch (toupper(from))
	{
		case 'W':
			from_table = _cyr_win1251;
			break;
		case 'A':
		case 'D':
			from_table = _cyr_cp866;
			break;
		case 'I':
			from_table = _cyr_iso88595;
			break;
		case 'M':
			from_table = _cyr_mac;
			break;
		case 'K':
			break;
	};

	unsigned char c1;
	unsigned char c2;
	for (int i = 0; i < len-1; i++)
	{
		if (from_table)
		{
				c1 = koi8_tolower[from_table[(unsigned char)str[i]]];
				c2 = koi8_tolower[from_table[(unsigned char)str[i+1]]];
		}
		else
		{
				c1 = koi8_tolower[(unsigned char)str[i]];
				c2 = koi8_tolower[(unsigned char)str[i+1]];
		}
		 
		if (decodeTable[c1] && decodeTable[c1][c2])
			ball++;
	}
	return ball;
}
	
/*char encodeHeader(char *result, char *header, int len)
{
	char charset = 0;
	if (len < 10)
	{
		strcpy(result, header);
		return charset;
	}
	char *pheader = header;
	char *presult = result;
	char *pstart = NULL;
	int l = 0;
	while (pheader-header < len)
	{
		pstart = strstr(pheader, "=?");
		if (!pstart)  
		{
			strcpy(presult, pheader); 
			break;
		} else if (pstart && ((l = pstart-pheader) > 0))
		{
			memcpy(presult, pheader, l);
			presult += l;
		}
		pheader = pstart+2;
		charset = *pheader;
		pheader=strchr(pheader, '?');
		pheader++;
		char encode = *pheader;
		pheader++;
		if (*pheader == '?')
			pheader++;
		else
		{
			 strcpy(presult, header);
			 return charset;
		}
		char *pend = strstr(pheader, "?=");
		if (pend)
		{
			*pend = 0;
			switch (toupper(encode))
			{
				case 'B': base64_decode(presult, pheader, pend-pheader);
									break;
				case 'Q': quoted_printable_decode(presult, pheader);
									break;
			}
			pheader = pend + 2;
		}
		else
		{
			strcpy(presult, pheader);
			break;
		}
		
		l = strlen(presult);
		if (tolower(charset) != 'w')	
			convert_cyr_string(presult, len, charset, 'w');
		presult += l;
	}
	return charset;
}*/

static char hex2int(int c)
{
	if ( isdigit(c) )
	{
		return c - '0';
	}
	else if ( c >= 'A' && c <= 'F' )
	{
		return c - 'A' + 10;
	}
	else if ( c >= 'a' && c <= 'f' )
	{
		return c - 'a' + 10;
	}
	else
	{
		return -1;
	}
}

/*
*
* Decoding  Quoted-printable string.
*
*/
//char *quoted_printable_decode(char *str_out, const char *str_in)
//{
//	int i = 0, j = 0, k = 0;
//	while (str_in[i]) {
//        switch (str_in[i]) {
//        case '=':
//            if (str_in[i+1] && str_in[i+2] && isxdigit((int)str_in[i+1]) && isxdigit((int)str_in[i+2])) {
//                str_out[j++] = (hex2int((int)str_in[i+1]) << 4) + hex2int((int)str_in[i+2]);
//                i += 3;
//			} else {  /* check for soft line break according to RFC 2045*/
//                k = i + 1;
//                while (str_in[k] && (str_in[k] == ' ' || str_in[k] == '\t')) {
//                   /* Possibly, skip spaces/tabs at the end of line */
//                    k++;
//                }
//                if (!str_in[k]) {
//                    /* End of line reached */
//                    i = k;
//                } else if (str_in[k] == '\r' && str_in[k+1] == '\n') {
//                    /* CRLF */
//                    i = k + 2;
//                } else if (str_in[k] == '\r' || str_in[k] == '\n') {
//                    /* CR or LF */
//                    i = k + 1;
//                } else {
//            		str_out[j++] = str_in[i++];
//                }
//            }
//            break;
//        default:
//    		str_out[j++] = str_in[i++];
//        }
//    }
//
//    str_out[j] = '\0';
//	return str_out;
//}

char *quoted_printable_decode(char *str_out, const char *str_in, const int len, const char ch)
{
	int i = 0, j = 0, k = 0;
	while (i < len) {
		if (str_in[i] == ch) {
			if (i + 2 < len && isxdigit((int) str_in[i+1]) && isxdigit((int) str_in[i+2])) {
				str_out[j++] = (hex2int((int) str_in[i+1]) << 4) + hex2int((int) str_in[i+2]);
				i += 3;
			} else {  /* check for soft line break according to RFC 2045*/
				k = i + 1;
				while (k < len && (str_in[k] == ' ' || str_in[k] == '\t')) {
				   /* Possibly, skip spaces/tabs at the end of line */
					k++;
				}
				if (k >= len) {
					/* End of line reached */
					i = k;
				} else if (str_in[k] == '\r' && k + 1 < len && str_in[k + 1] == '\n') {
					/* CRLF */
					i = k + 2;
				} else if (str_in[k] == '\r' || str_in[k] == '\n') {
					/* CR or LF */
					i = k + 1;
				} else {
					str_out[j++] = str_in[i++];
				}
			}
		} else {
			str_out[j++] = str_in[i++];
		}
	}

	str_out[j] = '\0';
	return str_out;
}

char *base64_decode_str(char *result, const char *in, size_t len)
{
    static unsigned char *un_b64 = 0;
    const unsigned char *cp;
    size_t  count;
    int     ch0;
    int     ch1;
    int     ch2;
    int     ch3;

#define CHARS_PER_BYTE  256
#define INVALID         0xff

    /*
     * Sanity check.
     */
	if (len % 4) {
		*result = '\0';
        return (0);
	}
	

    /*
     * Once: initialize the decoding lookup table on the fly.
     */
    if (un_b64 == 0) {
        un_b64 = (unsigned char *) malloc(CHARS_PER_BYTE);
        memset(un_b64, INVALID, CHARS_PER_BYTE);
        for (cp = to_b64; cp < to_b64 + sizeof(to_b64); cp++)
            un_b64[*cp] = cp - to_b64;
    }

    /*
     * Decode 4 -> 3.
     */
    for (cp = UNSIG_CHAR_PTR(in), count = 0; count < len; count += 4) {
        if ((ch0 = un_b64[*cp++]) == INVALID || (ch1 = un_b64[*cp++]) == INVALID) {
			*result = '\0';
            return (0);
		}
        *(result++) =  ch0 << 2 | ch1 >> 4;
        if ((ch2 = *cp++) == '=')
            break;
		if ((ch2 = un_b64[ch2]) == INVALID) {
			*result = '\0';
            return (0);
		}
        *(result++) =  ch1 << 4 | ch2 >> 2;
        if ((ch3 = *cp++) == '=')
            break;
		if ((ch3 = un_b64[ch3]) == INVALID) {
			*result = '\0';
            return (0);
		}
        *(result++) = ch2 << 6 | ch3;
    }

 	*result = '\0';
    return (result);
}

static const char base64_reverse_table[256] = {
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
	-2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
	-2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
};

char *base64_decode(char *str_out, const char *str_in, size_t length/*, bool strict*/)
{
	const unsigned char *current = (const unsigned char *) str_in;
	int ch, i = 0, j = 0, k;
	/* this sucks for threaded environments */
	unsigned char *result = (unsigned char *) str_out;
	
	/* run through the whole string, converting as we go */
	while ((ch = *current++) != '\0' && length-- > 0) {
		if (ch == '=') 
			break;

		ch = base64_reverse_table[ch];
		if (/*(!strict && ch < 0) || ch == -1*/ ch < 0) { /* a space or some other separator character, we simply skip over */
			continue;
		} else if (ch == -2) {
			*result = '\0';
			return NULL;
		}

		switch(i % 4) {
		case 0:
			result[j] = ch << 2;
			break;
		case 1:
			result[j++] |= ch >> 4;
			result[j] = (ch & 0x0f) << 4;
			break;
		case 2:
			result[j++] |= ch >>2;
			result[j] = (ch & 0x03) << 6;
			break;
		case 3:
			result[j++] |= ch;
			break;
		}
		i++;
	}

	k = j;
	/* mop things up if we ended on a boundary */
	if (ch == '=') {
		switch(i % 4) {
		case 1:
			*result = '\0';
			return NULL;
		case 2:
			k++;
		case 3:
			result[k++] = 0;
		}
	}

	result[j] = '\0';
	return (char *) result;
}

/* }}} */

/* {{{ php_convert_cyr_string
* This is the function that performs real in-place conversion of the string 
* between charsets. 
* Parameters:
*    str - string to be converted
*    from,to - one-symbol label of source and destination charset
* The following symbols are used as labels:
*    k - koi8-r
*    w - windows-1251
*    i - iso8859-5
*    a - x-cp866
*    d - x-cp866
*    m - x-mac-cyrillic
*****************************************************************************/
char *convert_cyr_string(char *str, int length, char from, char to)
{
	const unsigned char *from_table, *to_table;
	unsigned char tmp;
	int i;

	from_table = NULL;
	to_table   = NULL;
	
	switch (toupper(from))
	{
		case 'W':
			from_table = _cyr_win1251;
			break;
		case 'A':
		case 'D':
			from_table = _cyr_cp866;
			break;
		case 'I':
			from_table = _cyr_iso88595;
			break;
		case 'M':
			from_table = _cyr_mac;
			break;
		case 'K':
			break;
		default:
			printf("Unknown source charset: %c", from);
			break;
	}

	switch (toupper(to))
	{
		case 'W':
			to_table = _cyr_win1251;
			break;
		case 'A':
		case 'D':
			to_table = _cyr_cp866;
			break;
		case 'I':
			to_table = _cyr_iso88595;
			break;
		case 'M':
			to_table = _cyr_mac;
			break;
		case 'K':
			break;
		default:
			printf("Unknown destination charset: %c", to);
			break;
	}


	if (!str)
		return (char *)str;
	
	for( i = 0; i<length; i++)
	{
		tmp = (from_table == NULL)? str[i] : from_table[(unsigned char) str[i] ];
		str[i] = (to_table == NULL) ? tmp : to_table[tmp + 256];
	}
	return (char *)str;
}

char convert_cyr_char(char c, char from, char to)
{
	const unsigned char *from_table, *to_table;
	unsigned char tmp;
	from_table = NULL;
	to_table   = NULL;
	
	switch (toupper(from))
	{
		case 'W':
			from_table = _cyr_win1251;
			break;
		case 'A':
		case 'D':
			from_table = _cyr_cp866;
			break;
		case 'I':
			from_table = _cyr_iso88595;
			break;
		case 'M':
			from_table = _cyr_mac;
			break;
		case 'K':
			break;
		default:
			printf("Unknown source charset: %c", from);
			break;
	}

	switch (toupper(to))
	{
		case 'W':
			to_table = _cyr_win1251;
			break;
		case 'A':
		case 'D':
			to_table = _cyr_cp866;
			break;
		case 'I':
			to_table = _cyr_iso88595;
			break;
		case 'M':
			to_table = _cyr_mac;
			break;
		case 'K':
			break;
		default:
			printf("Unknown destination charset: %c", to);
			break;
	}
	tmp = (from_table == NULL)? c : from_table[(unsigned char) c ];
	return (to_table == NULL) ? tmp : to_table[tmp + 256];
}

bool checkXlatChars(const char *str, size_t len, const char xlat[])
{
	for (size_t i = 0; i < 256 && i < len; i++, str++) {
		if (!xlat[(unsigned char) (*str)]) {
			//printf("%20s\nwrong char '%c'(%d)\n", str, *str, (unsigned char) *str);
			return false;
		}
	}

	return true;
}

const char allowedMimeChars[256] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

size_t decode_mime_header(const char *from, class RString &to, size_t len, const char *additionCharset, const char *escapeChars)
{
	if ((int) len > to.MaxSize() - 1)
		len = to.MaxSize() - 1;

	if (additionCharset != NULL && !checkXlatChars(from, len, allowedMimeChars)) {
		memcpy(to, from, len);
		to.SetLength(len);
		//IConvert(to, len, additionCharset, "CP1251//TRANSLIT");
		IConvertToCP1251Translit(to, len, additionCharset);
                to.SetLength(strlen(to.Data()));

		return to.Length();
	}

	const char *in = from;
//	char *out = to;
	to.Null();
	const char *const end = from + len;
	const char *p;
	size_t tmpLen;

	while (in < end) {
		tmpLen = end - in;
		p = strnstr(in, "=?", tmpLen);
		if (!p) {
			to.Add(in, tmpLen);
			//memcpy(out, in, tmpLen);
			//out += tmpLen;
			break;
		}

		tmpLen = p - in;
		if (tmpLen) {
			for (const char *text = in; text < p; text++) {
				const char ch = *text;
				if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' && ch != '\0' && ch != '\x0B') {
					to.Add(in, tmpLen);
					//memcpy(out, in, tmpLen);
					//out += tmpLen;
					break;
				}
			}

			in += tmpLen;
		}

		const char *character = in + 2; // "=?"
		p = (const char *) memchr(character, '?', end - character);
		if (!p) {
			tmpLen = end - in;
			to.Add(in, tmpLen);
			//memcpy(out, in, tmpLen);
			//out += tmpLen;
			break;
		}
		const char *encoding = p + 1; // "?"
		if ((*encoding != 'B' && *encoding != 'b' && *encoding != 'Q' && *encoding != 'q') || *(encoding + 1) != '?') {
			to.Add(in, 2);
			//out[0] = in[0];
			//out[1] = in[1];
			in += 2;
			//out += 2;
			continue;
		}
		const char *text = encoding + 2; // "b?" || "q?"
		p = strnstr(text, "?=", end - text);
		if (!p) {
			tmpLen = end - in;
			to.Add(in, tmpLen);
			//memcpy(out, in, tmpLen);
			//out += tmpLen;
			break;
		}

		tmpLen = p - text;
		if (tmpLen <= 0) { // skip convert if size = 0
			in = p + 2; // "?="
			continue;
		}

		char *out = to.GetBuf(tmpLen);
		if (*encoding == 'b' || *encoding == 'B') {
			base64_decode(out, text, tmpLen);
		} else {
			for (size_t i = 0; i < tmpLen; i++)
				out[i] = text[i] == '_' ? ' ' : text[i];
			quoted_printable_decode(out, out, tmpLen);
		}
		tmpLen = strlen(out);

		class String charset(character, encoding - 1 - character);
		//IConvert(out, tmpLen, charset.c_str(), "CP1251//TRANSLIT");
		IConvertToCP1251Translit(out, tmpLen, charset.c_str());
		tmpLen = strlen(out);

		to.SetLength(out + tmpLen - to.Data());

		char *esc;
		if (escapeChars != NULL && (esc = strpbrk(out, escapeChars)) != NULL) {
			tmpLen -= esc - out;
			out = esc;
			char *tmp = esc;
			RString buf(tmpLen * 2);
			buf.Add('\\');
			buf.Add(*tmp++);
			while ((esc = strpbrk(tmp, escapeChars))) {
				buf.Add(tmp, esc - tmp);
				buf.Add('\\');
				buf.Add(*esc);
				tmp = esc + 1;
			}
			buf.AddStr(tmp);

			to.SetLength(out - to.Data());
			to.AddStr(buf.Data());
			//tmpLen = buf.Length();
			//memcpy(out, buf.Data(), tmpLen);
		}

		//to.SetLength(out + tmpLen - to.Data());
		//out += tmpLen;

		in = p + 2; // "?="
	}

//	to.SetLength(out - to.Data());

	return to.Length();
}

	void transliterate(const char *in, int inlen, char *out, int outlen)
	{
/*		static const char *_trCompl[256] = {
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,"Ye",	0,	0,	0,	0,"Yi",	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,"ye",	0,	0,	0,	0,"yi",	
			0,	0,	0,	0,	0,	0,	0,	0,	0,"Y",	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,"Yu","Ya",	
			0,	0,	0,	0,	0,	0,	0,	0,	0,"y",	0,	0,	0,	0,	0,	0,	
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,"yu","ya",	
		};*/
/*
		struct _Ttr2char {
			char key[3];
			char *val;
		};

		static const _Ttr2char _tr2char[19] = {
			{"��", "'O"}, {"��", "'o"},
			{"ܯ", "'I"}, {"��", "'i"},
			{"��", "ZH"}, {"��", "Zh"}, {"��", "zh"},
			{"��", "CH"}, {"��", "Ch"}, {"��", "ch"},
			{"��", "ZGH"}, {"��", "Zgh"}, {"��", "zgh"},
			{"��", "KGH"}, {"��", "Kgh"}, {"��", "kgh"},
			{"��", "SGH"}, {"��", "Sgh"}, {"��", "sgh"},
		};
*/
		static const char *_tr[256] = {
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0,   0,   0,  "",   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
		   "",   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,  "",   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
			0,   0,   0,   0,   0, "G",   0,   0, "E",   0,"Ye",   0,   0,   0,   0, "Yi",   
			0,   0, "I", "i", "g",   0,   0,   0, "e",   0,"ye",   0,   0,   0,   0, "yi",   
		  "A", "B", "V", "H", "D", "E","Zh", "Z", "Y", "Y", "K", "L", "M", "N", "O", "P",   
		  "R", "S", "T", "U", "F","Kh","Ts","Ch","Sh","Sch", "", "I",  "", "E","Yu","Ya",   
		  "a", "b", "v", "h", "d", "e","zh", "z", "y", "y", "k", "l", "m", "n", "o", "p",   
		  "r", "s", "t", "u", "f","kh","ts","ch","sh","sch", "", "i",  "", "e","yu","ya",   
		};
/*
		int i = 0, j = 0;
		const char *p = 0;
		for (; i < inlen && j < outlen - 3; i++) {
			p = _tr[(unsigned char) in[i]];
			if (p) {
				strcat(out, p);
				j += strlen(p);
			} else {
				strncat(out, in + i, 1);
				j++;
			}
		}
*/
		const char *inEnd = in + inlen;
		const char *outEnd = out + outlen - 1;
		const char *p;
		char *curOut;
		while (in < inEnd && out < outEnd) {
			p = _tr[(unsigned char) *in];
			if (p) {
				curOut = out;
				while (*p && out < outEnd)
					*out++ = *p++;
				if (*p) {
					out = curOut;
					break;
				}
			} else {
				*out++ = *in;
			}
			in++;
		}
		*out = '\0';
	}

	void decode_html_entities(char *buf, const char *charset)
	{
		// Removing / Replacing Windows Illegals Characters
		static const unsigned int tr_128_159[] = {
			8364, 160, 8218, 402, 8222, 8230, 8224, 8225, 
			710, 8240, 352, 8249, 338, 160, 381, 160, 
			160, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 
			732, 8482, 353, 8250, 339, 160, 382, 376,
		};

		const char *in = buf;
		char *out = buf;
		const char *begin;
		const char *end;
		size_t len = 0;
		iconv_t iconv_cd = (iconv_t) -1;

		while ((begin = strchr(in, '&')) && (end = strchr(begin + 1, ';'))) {
			len = begin - in;
			memcpy(out, in, len);
			in += len;
			out += len;
			begin++;

			if (*begin == '#') {
				begin++;
				int base = 10;
				if (*begin == 'x' || *begin == 'X') {
					begin++;
					base = 16;
				}
				char *endptr = 0;
				unsigned int num = strtoul(begin, &endptr, base);
				if (endptr != end) {
					len = begin - in;
					memcpy(out, in, len);
					in += len;
					out += len;
				} else if (!num || num >= 2097152) {
					in = end + 1;
				} else if (num < 128) {
					*out++ = num;
					in = end + 1;
				} else {
					// code2utf
					if (num < 160)
						num = tr_128_159[num - 128];

					len = 0;
					if (num < 2048) {
						out[0] = (num >> 6) + 192;
						out[1] = (num & 63) + 128;
						len = 2;
					} else if (num < 65536) {
						out[0] = (num >> 12) + 224;
						out[1] = ((num >> 6) & 63) + 128;
						out[2] = (num & 63) + 128;
						len = 3;
					} else if (num < 2097152) {
						out[0] = (num >> 18) + 240;
						out[1] = ((num >> 12) & 63) + 128;
						out[2] = ((num >> 6) & 63) + 128;
						out[3] = (num & 63) + 128;
						len = 4;
					}

					if (charset && iconv_cd == (iconv_t) -1) {
						if ((iconv_cd = iconv_open(charset, "UTF-8")) == (iconv_t) -1) {
							printf("Cannot open iconv from UTF-8 to %s\n", charset);
							charset = 0;
						}
					}
					if (iconv_cd != (iconv_t) -1) {
#ifdef BSD
						const char *inbuf = out;
#else
						char *inbuf = out;
#endif
						char *outbuf = out;
						size_t inlen = len;
						size_t outlen = len;

						//size_t res = 
						iconv(iconv_cd, &inbuf, &inlen, &outbuf, &outlen);
						out = outbuf;
					} else {
						out += len;
					}

					in = end + 1;
				}
			} else if ((len = end + 1 - in) >= 4 && len <= 8) { // check min&max enity len
				static const THtmlEntitiesTable *entitiesTable = get_html_entities_table();
				RString ent;
				ent.AddStr(in, end + 1 - in);
				ent.ToLower();
				THtmlEntitiesTable::const_iterator f = entitiesTable->find(ent.Data());
				if (f != entitiesTable->end()) {
					*out++ = f->second;
					in = end + 1;
				} else {
					*out++ = *in++;
				}
			} else {
				*out++ = *in++;
			}
		}
		
		if (out != in)
			strcpy(out, in);

		if (iconv_cd != (iconv_t) -1)
			iconv_close(iconv_cd);
	}

	const THtmlEntitiesTable *get_html_entities_table()
	{
		THtmlEntitiesTable *table = new THtmlEntitiesTable();
		table->insert(THtmlEntitiesTable::value_type("&nbsp;", 160));
		table->insert(THtmlEntitiesTable::value_type("&iexcl;", 161));
		table->insert(THtmlEntitiesTable::value_type("&cent;", 162));
		table->insert(THtmlEntitiesTable::value_type("&pound;", 163));
		table->insert(THtmlEntitiesTable::value_type("&curren;", 164));
		table->insert(THtmlEntitiesTable::value_type("&yen;", 165));
		table->insert(THtmlEntitiesTable::value_type("&brvbar;", 166));
		table->insert(THtmlEntitiesTable::value_type("&sect;", 167));
		table->insert(THtmlEntitiesTable::value_type("&uml;", 168));
		table->insert(THtmlEntitiesTable::value_type("&copy;", 169));
		table->insert(THtmlEntitiesTable::value_type("&ordf;", 170));
		table->insert(THtmlEntitiesTable::value_type("&laquo;", 171));
		table->insert(THtmlEntitiesTable::value_type("&not;", 172));
		table->insert(THtmlEntitiesTable::value_type("&shy;", 173));
		table->insert(THtmlEntitiesTable::value_type("&reg;", 174));
		table->insert(THtmlEntitiesTable::value_type("&macr;", 175));
		table->insert(THtmlEntitiesTable::value_type("&deg;", 176));
		table->insert(THtmlEntitiesTable::value_type("&plusmn;", 177));
		table->insert(THtmlEntitiesTable::value_type("&sup2;", 178));
		table->insert(THtmlEntitiesTable::value_type("&sup3;", 179));
		table->insert(THtmlEntitiesTable::value_type("&acute;", 180));
		table->insert(THtmlEntitiesTable::value_type("&micro;", 181));
		table->insert(THtmlEntitiesTable::value_type("&para;", 182));
		table->insert(THtmlEntitiesTable::value_type("&middot;", 183));
		table->insert(THtmlEntitiesTable::value_type("&cedil;", 184));
		table->insert(THtmlEntitiesTable::value_type("&sup1;", 185));
		table->insert(THtmlEntitiesTable::value_type("&ordm;", 186));
		table->insert(THtmlEntitiesTable::value_type("&raquo;", 187));
		table->insert(THtmlEntitiesTable::value_type("&frac14;", 188));
		table->insert(THtmlEntitiesTable::value_type("&frac12;", 189));
		table->insert(THtmlEntitiesTable::value_type("&frac34;", 190));
		table->insert(THtmlEntitiesTable::value_type("&iquest;", 191));
		table->insert(THtmlEntitiesTable::value_type("&agrave;", 192));
		table->insert(THtmlEntitiesTable::value_type("&aacute;", 193));
		table->insert(THtmlEntitiesTable::value_type("&acirc;", 194));
		table->insert(THtmlEntitiesTable::value_type("&atilde;", 195));
		table->insert(THtmlEntitiesTable::value_type("&auml;", 196));
		table->insert(THtmlEntitiesTable::value_type("&aring;", 197));
		table->insert(THtmlEntitiesTable::value_type("&aelig;", 198));
		table->insert(THtmlEntitiesTable::value_type("&ccedil;", 199));
		table->insert(THtmlEntitiesTable::value_type("&egrave;", 200));
		table->insert(THtmlEntitiesTable::value_type("&eacute;", 201));
		table->insert(THtmlEntitiesTable::value_type("&ecirc;", 202));
		table->insert(THtmlEntitiesTable::value_type("&euml;", 203));
		table->insert(THtmlEntitiesTable::value_type("&igrave;", 204));
		table->insert(THtmlEntitiesTable::value_type("&iacute;", 205));
		table->insert(THtmlEntitiesTable::value_type("&icirc;", 206));
		table->insert(THtmlEntitiesTable::value_type("&iuml;", 207));
		table->insert(THtmlEntitiesTable::value_type("&eth;", 208));
		table->insert(THtmlEntitiesTable::value_type("&ntilde;", 209));
		table->insert(THtmlEntitiesTable::value_type("&ograve;", 210));
		table->insert(THtmlEntitiesTable::value_type("&oacute;", 211));
		table->insert(THtmlEntitiesTable::value_type("&ocirc;", 212));
		table->insert(THtmlEntitiesTable::value_type("&otilde;", 213));
		table->insert(THtmlEntitiesTable::value_type("&ouml;", 214));
		table->insert(THtmlEntitiesTable::value_type("&times;", 215));
		table->insert(THtmlEntitiesTable::value_type("&oslash;", 216));
		table->insert(THtmlEntitiesTable::value_type("&ugrave;", 217));
		table->insert(THtmlEntitiesTable::value_type("&uacute;", 218));
		table->insert(THtmlEntitiesTable::value_type("&ucirc;", 219));
		table->insert(THtmlEntitiesTable::value_type("&uuml;", 220));
		table->insert(THtmlEntitiesTable::value_type("&yacute;", 221));
		table->insert(THtmlEntitiesTable::value_type("&thorn;", 222));
		table->insert(THtmlEntitiesTable::value_type("&szlig;", 223));
		table->insert(THtmlEntitiesTable::value_type("&agrave;", 224));
		table->insert(THtmlEntitiesTable::value_type("&aacute;", 225));
		table->insert(THtmlEntitiesTable::value_type("&acirc;", 226));
		table->insert(THtmlEntitiesTable::value_type("&atilde;", 227));
		table->insert(THtmlEntitiesTable::value_type("&auml;", 228));
		table->insert(THtmlEntitiesTable::value_type("&aring;", 229));
		table->insert(THtmlEntitiesTable::value_type("&aelig;", 230));
		table->insert(THtmlEntitiesTable::value_type("&ccedil;", 231));
		table->insert(THtmlEntitiesTable::value_type("&egrave;", 232));
		table->insert(THtmlEntitiesTable::value_type("&eacute;", 233));
		table->insert(THtmlEntitiesTable::value_type("&ecirc;", 234));
		table->insert(THtmlEntitiesTable::value_type("&euml;", 235));
		table->insert(THtmlEntitiesTable::value_type("&igrave;", 236));
		table->insert(THtmlEntitiesTable::value_type("&iacute;", 237));
		table->insert(THtmlEntitiesTable::value_type("&icirc;", 238));
		table->insert(THtmlEntitiesTable::value_type("&iuml;", 239));
		table->insert(THtmlEntitiesTable::value_type("&eth;", 240));
		table->insert(THtmlEntitiesTable::value_type("&ntilde;", 241));
		table->insert(THtmlEntitiesTable::value_type("&ograve;", 242));
		table->insert(THtmlEntitiesTable::value_type("&oacute;", 243));
		table->insert(THtmlEntitiesTable::value_type("&ocirc;", 244));
		table->insert(THtmlEntitiesTable::value_type("&otilde;", 245));
		table->insert(THtmlEntitiesTable::value_type("&ouml;", 246));
		table->insert(THtmlEntitiesTable::value_type("&divide;", 247));
		table->insert(THtmlEntitiesTable::value_type("&oslash;", 248));
		table->insert(THtmlEntitiesTable::value_type("&ugrave;", 249));
		table->insert(THtmlEntitiesTable::value_type("&uacute;", 250));
		table->insert(THtmlEntitiesTable::value_type("&ucirc;", 251));
		table->insert(THtmlEntitiesTable::value_type("&uuml;", 252));
		table->insert(THtmlEntitiesTable::value_type("&yacute;", 253));
		table->insert(THtmlEntitiesTable::value_type("&thorn;", 254));
		table->insert(THtmlEntitiesTable::value_type("&yuml;", 255));
		table->insert(THtmlEntitiesTable::value_type("&quot;", 34));
		table->insert(THtmlEntitiesTable::value_type("&lt;", 60));
		table->insert(THtmlEntitiesTable::value_type("&gt;", 62));
		table->insert(THtmlEntitiesTable::value_type("&amp;", 38));

		return table;
	}


};
