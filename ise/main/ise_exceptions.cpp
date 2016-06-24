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
// �ļ�����: ise_exceptions.cpp
// ��������: �쳣��
///////////////////////////////////////////////////////////////////////////////

#include "ise/main/ise_exceptions.h"
#include "ise/main/ise_sys_utils.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////

const char* const EXCEPTION_LOG_PREFIX = "ERROR: ";

///////////////////////////////////////////////////////////////////////////////
// class Exception

//-----------------------------------------------------------------------------
// ����: ���� std::exception ���ش�����Ϣ
//-----------------------------------------------------------------------------
const char* Exception::what() const throw()
{
    what_ = getErrorMessage();
    return what_.c_str();
}

//-----------------------------------------------------------------------------
// ����: �������� Log ���ַ���
//-----------------------------------------------------------------------------
string Exception::makeLogStr() const
{
    return string(EXCEPTION_LOG_PREFIX) + getErrorMessage();
}

///////////////////////////////////////////////////////////////////////////////
// class SimpleException

SimpleException::SimpleException(const char *errorMsg,
    const char *srcFileName, int srcLineNumber)
{
    if (errorMsg)
        errorMsg_ = errorMsg;
    if (srcFileName)
        srcFileName_ = srcFileName;
    srcLineNumber_ = srcLineNumber;
}

//-----------------------------------------------------------------------------

string SimpleException::makeLogStr() const
{
    string result(getErrorMessage());

    if (!srcFileName_.empty() && srcLineNumber_ >= 0)
        result = result + " (" + srcFileName_ + ":" + intToStr(srcLineNumber_) + ")";

    result = string(EXCEPTION_LOG_PREFIX) + result;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// class FileException

FileException::FileException(const char *fileName, int errorCode, const char *errorMsg) :
    fileName_(fileName),
    errorCode_(errorCode)
{
    if (errorMsg == NULL)
        errorMsg_ = sysErrorMessage(errorCode);
    else
        errorMsg_ = errorMsg;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise
