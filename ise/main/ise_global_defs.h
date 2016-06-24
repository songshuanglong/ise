/****************************************************************************\
*                                                                            *
*  ISE (Iris Server Engine) Project                                          *
*  http://github.com/haoxingeng/ise                                          *
*                                                                            *
*  Copyright 2013 HaoXinGeng (haoxingeng@gmail.com)                          *
*  All rights reserved.                                                      *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
\****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// �ļ�����: ise_global_defs.h
// ��������: ȫ�ֶ���
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_GLOBAL_DEFS_H_
#define _ISE_GLOBAL_DEFS_H_

#include "ise/main/ise_options.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef ISE_COMPILER_VC
#include <basetsd.h>
#else
#include <stdint.h>
#endif

// boost headers
#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/utility.hpp"
#include "boost/any.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"

#include "ise/main/ise_stldefs.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ���Ͷ���

#ifdef ISE_COMPILER_VC
typedef INT8      int8_t;
typedef INT16     int16_t;
typedef INT32     int32_t;
typedef INT64     int64_t;
typedef UINT8     uint8_t;
typedef UINT16    uint16_t;
typedef UINT32    uint32_t;
typedef UINT64    uint64_t;
#endif

#ifdef ISE_LINUX
typedef int8_t    INT8;
typedef int16_t   INT16;
typedef int32_t   INT32;
typedef int64_t   INT64;

typedef uint8_t   UINT8;
typedef uint16_t  UINT16;
typedef uint32_t  UINT32;
typedef uint64_t  UINT64;

typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef uint32_t  DWORD;
typedef uint32_t  UINT;

typedef int       HANDLE;
typedef void*     PVOID;
typedef UINT8*    PBYTE;
typedef UINT16*   PWORD;
typedef UINT32*   PDWORD;
#endif

#ifdef ISE_WINDOWS
typedef UINT32 THREAD_ID;
#endif
#ifdef ISE_LINUX
typedef pid_t THREAD_ID;
#endif

typedef void* POINTER;

typedef std::vector<string> StringArray;
typedef std::vector<int> IntegerArray;
typedef std::vector<bool> BooleanArray;
typedef std::set<int> IntegerSet;

typedef boost::any Context;
typedef boost::function<void(void)> Functor;

typedef boost::function<void(void)> TimerCallback;
typedef INT64 TimerId;

///////////////////////////////////////////////////////////////////////////////
// ��������

#ifdef ISE_WINDOWS
const char PATH_DELIM               = '\\';
const char DRIVER_DELIM             = ':';
const char FILE_EXT_DELIM           = '.';
const char* const S_CRLF            = "\r\n";
#endif
#ifdef ISE_LINUX
const char PATH_DELIM               = '/';
const char FILE_EXT_DELIM           = '.';
const char* const S_CRLF            = "\n";
#endif

// �ļ�����
const unsigned int FA_READ_ONLY     = 0x00000001;
const unsigned int FA_HIDDEN        = 0x00000002;
const unsigned int FA_SYS_FILE      = 0x00000004;
const unsigned int FA_VOLUME_ID     = 0x00000008;    // Windows Only
const unsigned int FA_DIRECTORY     = 0x00000010;
const unsigned int FA_ARCHIVE       = 0x00000020;
const unsigned int FA_SYM_LINK      = 0x00000040;    // Linux Only
const unsigned int FA_ANY_FILE      = 0x0000007F;

// ʱ�����
const int MILLISECS_PER_SECOND   = 1000;
const int SECONDS_PER_MINUTE     = 60;
const int MINUTES_PER_HOUR       = 60;
const int HOURS_PER_DAY          = 24;
const int SECONDS_PER_HOUR       = 3600;
const int SECONDS_PER_DAY        = 86400;

// ��Χֵ
#define MAXCHAR     0x7f
#define MAXSHORT    0x7fff
#define MAXINT      0x7fffffff
#define MAXBYTE     0xff
#define MAXWORD     0xffff
#define MAXDWORD    0xffffffff

// Strings ��س�������
#define DEFAULT_DELIMITER            ','
#define DEFAULT_QUOTE_CHAR           '\"'
#define DEFAULT_NAME_VALUE_SEP       '='
#define DEFAULT_LINE_BREAK           S_CRLF

#ifdef ISE_LINUX
#define INVALID_HANDLE_VALUE        ((HANDLE)-1)
#endif

#define TRUE_STR                    "true"
#define FALSE_STR                   "false"

const int TIMEOUT_INFINITE = -1;

const Context EMPTY_CONTEXT = Context();

///////////////////////////////////////////////////////////////////////////////
// �궨��

#define SAFE_DELETE(x)          { delete x; x = NULL; }
#define CATCH_ALL_EXCEPTION(x)  try { x; } catch(...) {}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_GLOBAL_H_
