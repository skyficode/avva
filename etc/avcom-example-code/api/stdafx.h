// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once





#if (defined(_WIN32))
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <ws2tcpip.h>

#include "targetver.h"
#include <stdio.h>
#include "stdafx.h"
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

#include <string.h>
//#include <stdio.h>
//#include <errno.h>
#endif

// TODO: reference additional headers your program requires here
