#ifndef __SPEC_HTTP_HPP__ 
#define __SPEC_HTTP_HPP__

#define MAX_HTTP_HOST 64
#define MAX_HTTP_URL  27//21

#define COLOR_DEPTH_1		1
#define COLOR_DEPTH_4		2
#define COLOR_DEPTH_8		3
#define COLOR_DEPTH_16	4
#define COLOR_DEPTH_24	5
#define COLOR_DEPTH_32	6
#define COLOR_DEPTH_UNK	0


#define RESOLUTION_640 1
#define RESOLUTION_800 2
#define RESOLUTION_1024 3
#define RESOLUTION_1152 4
#define RESOLUTION_1280 5
#define RESOLUTION_1600 6
#define RESOLUTION_1400 7
#define RESOLUTION_1440 8
#define RESOLUTION_1680 9
#define RESOLUTION_1792 10
#define RESOLUTION_1800 11
#define RESOLUTION_1360 12
#define RESOLUTION_1366 13
#define RESOLUTION_1920 14
#define RESOLUTION_UNK 0

#define OUTPUT_TYPE_GIF	0
#define OUTPUT_TYPE_JS	1

#define VALUE_EMPTY 0
#define VALUE_FALSE 2
#define VALUE_TRUE  3
#define VALUE_MASK	7


// Define specific http parse
public:
	Byte Type() { return type; }
	Word32 Site() { return siteID; }
	Byte Resolution() { return resolution; }
	Byte Depth() { return depth; }
	String &FromPage() { return fromPage; }
	Word64 &UserID() { return userID; }
	String &X() { return x; }
	Byte Logo() { return logo; }
	Byte IsAdmin() { return isAdmin; }
	Byte IsHome() { return isHome; }
	Byte Output() { return output; }
	Byte Javascript(){ return js; }
	Byte Java(){ return java; }
	Byte Flash(){ return flash; }
	Byte Frame() {return frame; }
	Byte isValid() {
		if (bValidate) return  validate == siteID ^ vres ^ vdepth ^ fromPage.GetLength() ^ (Referer().GetLength()+7) ? VALUE_TRUE : VALUE_FALSE; // ??? ()
		return VALUE_EMPTY;
	}
	Word64 ExtUserID() { return _extUserID; }

	friend class Control;
protected:
	void InitData()
	{
		output = OUTPUT_TYPE_GIF;
		siteID = 0;
		userID = 0;
		type = 0; 
		vres = resolution = 0;
		vdepth = depth = 0;
		logo = 0; 
		isAdmin = 0;
		isHome = 0;
		java = 0;
		flash = 0;
		validate = 0;
		bValidate = 0;
		frame = 0;
		js = 0;
		_extUserID = 0;
	}
	Word32 siteID;
	Word64 userID;
	Byte  type;
	Byte  resolution;
	Byte  depth,vdepth;	
	Word32 validate;
	Byte   bValidate;
	Byte   js;
	Byte	output;
	Byte	frame;
	Byte	java;
	Byte	flash;
	Byte  logo;
	Byte	isHome;
	String  fromPage;
	Byte isAdmin;
	String x; 
	int isStat, vres;
	Word64 _extUserID;
//stat.php
	int CheckFile(char *tbuf, int len)
	{
		if (len > 8) 
			if (!memcmp(tbuf-8, "stat.php", 8))
			{
				isStat = 1; // If query /stat.php params in 'param=value' format
			}
		return 1;
	}
	int SetParam(char param, char *paramValueBuf, int i)
	{
		int res = 0;
		if (isStat && *(paramValueBuf) == '=')
		{
			paramValueBuf++;
			i--;
		}
		switch (param) 
		{
			case 'o': output = atoi(paramValueBuf) ? OUTPUT_TYPE_JS : OUTPUT_TYPE_GIF;
					break;
			case 'v': validate = atoi(paramValueBuf);
					bValidate = 1;
					js=1;
					break;
			case 'm': flash = atoi(paramValueBuf);
					if(flash>20) flash=0;
					js=1;
					break;
			case 'y': java  = atoi(paramValueBuf);
					if(java) java=1;
					js=1;
					break;
			case 'w': frame  = atoi(paramValueBuf);
					frame = frame & VALUE_MASK;
					js=1;
					break;
			case 's': siteID = atoi(paramValueBuf);
					break;
			case 'a': //quarantine = atoi(paramValueBuf);
					break;
			case 't': type = atoi(paramValueBuf);
			        if (type==100) type = 13;
					break;
			case 'f':
					if (i > 9)
					{
					  if (!memcmp("http%3A//", paramValueBuf, 9))
					  {
						paramValueBuf += 9;
						i -= 9;
					  }
					  else if ((i >13) &&!memcmp("http%3A%2F%2F", paramValueBuf, 13))
					  {
						paramValueBuf += 13;
						i -= 13;
					  }
                                          else if ((i > 10) &&!memcmp("https%3A//", paramValueBuf, 10))
					  {
						paramValueBuf += 10;
						i -= 10;
					  }
					  else if ((i >14) &&!memcmp("https%3A%2F%2F", paramValueBuf, 14))
					  {
						paramValueBuf += 14;
						i -= 14;
					  }
					}
					fromPage.SetUnescapeChars(paramValueBuf, i);
					js=1;
					break;
			case 'r':	vres = res = atoi(paramValueBuf);
					switch (res)
					{
						case 640:  resolution = RESOLUTION_640;break;
						case 800:  resolution = RESOLUTION_800;break;
						case 1024: resolution = RESOLUTION_1024;break;
						case 1152: resolution = RESOLUTION_1152;break;
						case 1280: resolution = RESOLUTION_1280;break;
						case 1400: resolution = RESOLUTION_1400;break;
						case 1600: resolution = RESOLUTION_1600;break;
						case 1440: resolution = RESOLUTION_1440;break;
						case 1680: resolution = RESOLUTION_1680;break;
						case 1792: resolution = RESOLUTION_1792;break;
						case 1800: resolution = RESOLUTION_1800;break;
						case 1360: resolution = RESOLUTION_1360;break;
						case 1366: resolution = RESOLUTION_1366;break;
						case 1920: resolution = RESOLUTION_1920;break;
						default: resolution = RESOLUTION_UNK;
					}
					js=1;
					break;
			case 'd': vdepth = depth = atoi(paramValueBuf);
					switch (depth)
					{
						case 1: depth = COLOR_DEPTH_1; break;
						case 4: depth = COLOR_DEPTH_4; break;
						case 8: depth = COLOR_DEPTH_8; break;
						case 16: depth = COLOR_DEPTH_16; break;
						case 24: depth = COLOR_DEPTH_24; break;
						case 32: depth = COLOR_DEPTH_32; break;
						default: depth = COLOR_DEPTH_UNK; break;
					};
					js=1;
					break;
			case 'l': logo = atoi(paramValueBuf);
					break;
			case 'c': isCookies = 1;
					js=1;
					break;
			case 'p': 
					if (!strcmp(paramValueBuf, "RolBackAFK"))
						isAdmin = 1;
					break;
			case 'x': if (!x)
					x.Set(paramValueBuf, i);
					break;
			case 'h': isHome = 1;
					js=1;
					break;
			case 'n':
					js=1;
					break;
			case 'z':
					_extUserID = strtoull(paramValueBuf, NULL, 16);
					break;
			default:
			{
				isError = HTTP_ERROR_UNKOWN_PARAM;
				return 0;
				//throw HttpError(HTTP_ERROR_UNKOWN_PARAM);
			}
		};
		return 1;
	};
	void ParseCookie(char *cookie)
	{
		char *pcookie = cookie;
		pcookie=strstr(cookie, "U=");
		if (pcookie)
		{
			pcookie+=2;
			userID=strtoul(pcookie, NULL, 16);
		}
	}

#endif

