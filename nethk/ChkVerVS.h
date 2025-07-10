#if (_MSC_VER < 1300)	// Visual studio 6 이하라면

#define		_tcscat_s(a, b)						strcat(a, b)
#define		_vsntprintf_s(a, b, c, d, e)		_vsntprintf(a, b, d, e)
#define		_vstprintf_s(a, b, c)				_vstprintf(a, b, c)
#define		strcpy_s(a,b,c)						strcpy(a,c)
#define		wcscpy_s(a,b)						wcscpy(a,b)
#define		_tcscpy_s(a,b,c)					strcpy(a,c)
#define		_tcslwr_s(a)						_tcslwr(a)
#define		_tcsncpy_s(a,b,c,d)					strncpy(a,c,d)
#define		memcpy_s(a,b,c,d)					memcpy(a,c,b)
#define		_strtime_s(a)						_strtime(a);
#define		sscanf_s(a,b,c)						sscanf(a,b,c)

#endif