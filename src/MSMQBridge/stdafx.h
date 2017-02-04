// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma warning( disable : 4251 4800)

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

// TODO: reference additional headers your program requires here

#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <hash_map>
#include <MqOai.h>


using namespace std;


#import "mqoa.dll" named_guids
using namespace MSMQ;

#include <mq.h>

#define MAX_QUEUE_PROPERTIES 3
#define EVENT_LOG_ENTRY_COUNT 2
#define MAX_MESSAGE_PROPERTIES 4
#define EVENT_LOG_NAME L"Application"
#define APPLICATION_NAME L"MSMQBridge"
#define IS_BETWEEN (X, MAX) (X >= 0 && X <= MAX)
#define DEFAULT_LABEL L"MSMQBridge Default Message Label"