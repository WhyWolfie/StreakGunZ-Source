// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef _STDAFX_H
#define _STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT 0x0501

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS

#pragma once

//#include "targetver.h"

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define NO_WARN_MBCS_MFC_DEPRECATION


#include <sdkddkver.h>

// ¿©±â´Ù include
#include <winsock2.h>
#include <crtdbg.h>
#include <mbstring.h>
#include <tchar.h>

#include <list>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <numeric>
#include <future>
#include <thread>
#include <functional>

#include "d3dx9math.h"

#include "MDebug.h"
#include "MZFileSystem.h"
#include "fileinfo.h"
#include "MXml.h"
#include "RTypes.h"
#include "RMesh.h"
#endif