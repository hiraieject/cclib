/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __YOSTEMPLATE_H__
#define __YOSTEMPLATE_H__

#ifdef __linux
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
//#include <math.h>
#include <dirent.h>
#endif

#ifdef _MSC_VER
// Windows header files
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
// C RunTime header files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#define ASSERT assert
#endif

#include <queue>
#include <vector>
#include <list>
#include <algorithm>
#include <string>  
#include <cctype>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <string>
#include <locale>
//#include <codecvt>
using namespace std;


#ifdef __linux
#define DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt, __FILE__,__FUNCTION__,__LINE__,## args); }
#define ERRPR(fmt, args...) \
	{ printf("[%s:%s():%d] ##### ERROR!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#define WARNPR(fmt, args...)											\
	{ printf("[%s:%s():%d] ##### WARNING!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#endif

#include "udpcomm.h"

#endif // __YOSTEMPLATE_H__

