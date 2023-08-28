// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifndef _SECURE_ATL
#define _SECURE_ATL 1 //Use the Secure C Runtime in ATL
#endif

#define NO_CENUMERATESERIAL_USING_CREATEFILE 1

#define NO_CENUMERATESERIAL_USING_QUERYDOSDEVICE 1
#define NO_CENUMERATESERIAL_USING_GETDEFAULTCOMMCONFIG 1
#define NO_CENUMERATESERIAL_USING_SETUPAPI2 1
#define NO_CENUMERATESERIAL_USING_ENUMPORTS 1
#define NO_CENUMERATESERIAL_USING_WMI 1
#define NO_CENUMERATESERIAL_USING_COMDB 1
#define NO_CENUMERATESERIAL_USING_REGISTRY 1


// Windows Header Files:
#include <windows.h>

#include "atlbase.h"
#include "atlstr.h"

#include <process.h>
#include <stdio.h>

#include <setupapi.h>

#include <stdint.h>


// TODO: reference additional headers your program requires here
