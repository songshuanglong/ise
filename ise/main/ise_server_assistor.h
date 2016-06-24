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
// ise_server_assistor.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_SERVER_ASSISTOR_H_
#define _ISE_SERVER_ASSISTOR_H_

#include "ise/main/ise_options.h"
#include "ise/main/ise_classes.h"
#include "ise/main/ise_thread.h"
#include "ise/main/ise_sys_utils.h"
#include "ise/main/ise_socket.h"
#include "ise/main/ise_exceptions.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ��ǰ����

class AssistorThread;
class AssistorThreadPool;
class AssistorServer;

///////////////////////////////////////////////////////////////////////////////
// class AssistorThread - �����߳���
//
// ˵��:
// 1. ���ڷ���˳��򣬳����������֮�⣬һ�㻹��Ҫ���ɺ�̨�ػ��̣߳����ں�̨ά��������
//    �����������ݻ��ա����ݿ������������ȵȡ���������߳�ͳ��Ϊ assistor thread.

class AssistorThread : public Thread
{
public:
    AssistorThread(AssistorThreadPool *threadPool, int assistorIndex);
    virtual ~AssistorThread();

    int getIndex() const { return assistorIndex_; }

protected:
    virtual void execute();
    virtual void doKill();

private:
    AssistorThreadPool *ownPool_;        // �����̳߳�
    int assistorIndex_;                  // �����������(0-based)
};

///////////////////////////////////////////////////////////////////////////////
// class AssistorThreadPool - �����̳߳���

class AssistorThreadPool : boost::noncopyable
{
public:
    explicit AssistorThreadPool(AssistorServer *ownAssistorServer);
    virtual ~AssistorThreadPool();

    void registerThread(AssistorThread *thread);
    void unregisterThread(AssistorThread *thread);

    // ֪ͨ�����߳��˳�
    void terminateAllThreads();
    // �ȴ������߳��˳�
    void waitForAllThreads();
    // ���ָ���̵߳�˯��
    void interruptThreadSleep(int assistorIndex);

    // ȡ�õ�ǰ�߳�����
    int getThreadCount() { return threadList_.getCount(); }
    // ȡ����������������
    AssistorServer& getAssistorServer() { return *ownAssistorSvr_; }

private:
    AssistorServer *ownAssistorSvr_;     // ��������������
    ThreadList threadList_;              // �߳��б�
};

///////////////////////////////////////////////////////////////////////////////
// class AssistorServer - ����������

class AssistorServer : boost::noncopyable
{
public:
    explicit AssistorServer();
    virtual ~AssistorServer();

    // ����������
    void open();
    // �رշ�����
    void close();

    // ���������߳�ִ�к���
    void onAssistorThreadExecute(AssistorThread& assistorThread, int assistorIndex);

    // ֪ͨ���и����߳��˳�
    void terminateAllAssistorThreads();
    // �ȴ����и����߳��˳�
    void waitForAllAssistorThreads();
    // ���ָ�������̵߳�˯��
    void interruptAssistorThreadSleep(int assistorIndex);

private:
    bool isActive_;                       // �������Ƿ�����
    AssistorThreadPool threadPool_;       // �����̳߳�
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_SERVER_ASSISTOR_H_
