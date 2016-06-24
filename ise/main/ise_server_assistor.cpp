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
// �ļ�����: ise_server_assistor.cpp
// ��������: ������������ʵ��
///////////////////////////////////////////////////////////////////////////////

#include "ise/main/ise_server_assistor.h"
#include "ise/main/ise_err_msgs.h"
#include "ise/main/ise_application.h"

using namespace ise;

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// class AssistorThread

AssistorThread::AssistorThread(AssistorThreadPool *threadPool, int assistorIndex) :
    ownPool_(threadPool),
    assistorIndex_(assistorIndex)
{
    setAutoDelete(true);
    ownPool_->registerThread(this);
}

AssistorThread::~AssistorThread()
{
    ownPool_->unregisterThread(this);
}

//-----------------------------------------------------------------------------
// ����: �߳�ִ�к���
//-----------------------------------------------------------------------------
void AssistorThread::execute()
{
    ownPool_->getAssistorServer().onAssistorThreadExecute(*this, assistorIndex_);
}

//-----------------------------------------------------------------------------
// ����: ִ�� kill() ǰ�ĸ��Ӳ���
//-----------------------------------------------------------------------------
void AssistorThread::doKill()
{
    // nothing
}

///////////////////////////////////////////////////////////////////////////////
// class AssistorThreadPool

AssistorThreadPool::AssistorThreadPool(AssistorServer *ownAssistorServer) :
    ownAssistorSvr_(ownAssistorServer)
{
    // nothing
}

AssistorThreadPool::~AssistorThreadPool()
{
    // nothing
}

//-----------------------------------------------------------------------------
// ����: ע���߳�
//-----------------------------------------------------------------------------
void AssistorThreadPool::registerThread(AssistorThread *thread)
{
    threadList_.add(thread);
}

//-----------------------------------------------------------------------------
// ����: ע���߳�
//-----------------------------------------------------------------------------
void AssistorThreadPool::unregisterThread(AssistorThread *thread)
{
    threadList_.remove(thread);
}

//-----------------------------------------------------------------------------
// ����: ֪ͨ�����߳��˳�
//-----------------------------------------------------------------------------
void AssistorThreadPool::terminateAllThreads()
{
    threadList_.terminateAllThreads();
}

//-----------------------------------------------------------------------------
// ����: �ȴ������߳��˳�
//-----------------------------------------------------------------------------
void AssistorThreadPool::waitForAllThreads()
{
    if (threadList_.getCount() > 0)
        logger().writeFmt(SEM_WAIT_FOR_THREADS, "assistor");

    threadList_.waitForAllThreads(TIMEOUT_INFINITE);
}

//-----------------------------------------------------------------------------
// ����: ���ָ���̵߳�˯��
//-----------------------------------------------------------------------------
void AssistorThreadPool::interruptThreadSleep(int assistorIndex)
{
    AutoLocker locker(threadList_.getMutex());

    for (int i = 0; i < threadList_.getCount(); i++)
    {
        AssistorThread *thread = (AssistorThread*)threadList_[i];
        if (thread->getIndex() == assistorIndex)
        {
            thread->interruptSleep();
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// class AssistorServer

AssistorServer::AssistorServer() :
    isActive_(false),
    threadPool_(this)
{
    // nothing
}

AssistorServer::~AssistorServer()
{
    // nothing
}

//-----------------------------------------------------------------------------
// ����: ����������
//-----------------------------------------------------------------------------
void AssistorServer::open()
{
    if (!isActive_)
    {
        int count = iseApp().iseOptions().getAssistorThreadCount();

        for (int i = 0; i < count; i++)
        {
            AssistorThread *thread;
            thread = new AssistorThread(&threadPool_, i);
            thread->run();
        }

        isActive_ = true;
    }
}

//-----------------------------------------------------------------------------
// ����: �رշ�����
//-----------------------------------------------------------------------------
void AssistorServer::close()
{
    if (isActive_)
    {
        waitForAllAssistorThreads();
        isActive_ = false;
    }
}

//-----------------------------------------------------------------------------
// ����: ���������߳�ִ�к���
// ����:
//   assistorIndex - �����߳����(0-based)
//-----------------------------------------------------------------------------
void AssistorServer::onAssistorThreadExecute(AssistorThread& assistorThread, int assistorIndex)
{
    iseApp().iseBusiness().assistorThreadExecute(assistorThread, assistorIndex);
}

//-----------------------------------------------------------------------------
// ����: ֪ͨ���и����߳��˳�
//-----------------------------------------------------------------------------
void AssistorServer::terminateAllAssistorThreads()
{
    threadPool_.terminateAllThreads();
}

//-----------------------------------------------------------------------------
// ����: �ȴ����и����߳��˳�
//-----------------------------------------------------------------------------
void AssistorServer::waitForAllAssistorThreads()
{
    threadPool_.waitForAllThreads();
}

//-----------------------------------------------------------------------------
// ����: ���ָ�������̵߳�˯��
//-----------------------------------------------------------------------------
void AssistorServer::interruptAssistorThreadSleep(int assistorIndex)
{
    threadPool_.interruptThreadSleep(assistorIndex);
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise
