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
// ise_svr_mod_msgs.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_SVR_MOD_MSGS_H_
#define _ISE_SVR_MOD_MSGS_H_

#include "ise/main/ise_options.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ��Ϣ���붨�� (Server Module Message Code)

const UINT SMMC_BASE = 100;

///////////////////////////////////////////////////////////////////////////////
// ��Ϣ�ṹ����

// ��Ϣ����
struct BaseSvrModMessage
{
public:
    UINT messageCode;      // ��Ϣ����
    bool isHandled;        // �Ƿ����˴���Ϣ
public:
    BaseSvrModMessage() : messageCode(0), isHandled(false) {}
    virtual ~BaseSvrModMessage() {}
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_SVR_MOD_MSGS_H_
