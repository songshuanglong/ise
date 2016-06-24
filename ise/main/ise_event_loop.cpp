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
// �ļ�����: ise_event_loop.cpp
// ��������: �¼�ѭ��ʵ��
///////////////////////////////////////////////////////////////////////////////

#include "ise/main/ise_event_loop.h"
#include "ise/main/ise_err_msgs.h"

using namespace ise;

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// class EventLoop

EventLoop::EventLoop() :
    thread_(NULL),
    loopThreadId_(0),
    lastCheckTimeoutTicks_(0)
{
    // nothing
}

EventLoop::~EventLoop()
{
    stop(false, true);
}

//-----------------------------------------------------------------------------
// ����: ���������߳�
//-----------------------------------------------------------------------------
void EventLoop::start()
{
    if (!thread_)
    {
        thread_ = new EventLoopThread(*this);
        thread_->run();
    }
}

//-----------------------------------------------------------------------------
// ����: ֹͣ�����߳�
//-----------------------------------------------------------------------------
void EventLoop::stop(bool force, bool waitFor)
{
    if (thread_ && thread_->isRunning())
    {
        if (force)
        {
            thread_->kill();
            thread_ = NULL;
            waitFor = false;
        }
        else
        {
            thread_->terminate();
            wakeupLoop();
        }

        if (waitFor)
        {
            thread_->waitFor();
            delete thread_;
            thread_ = NULL;
        }
    }
}

//-----------------------------------------------------------------------------
// ����: �жϹ����̵߳�ǰ�Ƿ���������
//-----------------------------------------------------------------------------
bool EventLoop::isRunning()
{
    return (thread_ != NULL && thread_->isRunning());
}

//-----------------------------------------------------------------------------
// ����: �жϵ�ǰ���ô˷������̺߳ʹ� eventLoop �����߳��ǲ���ͬһ���߳�
//-----------------------------------------------------------------------------
bool EventLoop::isInLoopThread()
{
    return loopThreadId_ == getCurThreadId();
}

//-----------------------------------------------------------------------------
// ����: ȷ����ǰ�������¼�ѭ���߳���
//-----------------------------------------------------------------------------
void EventLoop::assertInLoopThread()
{
    ISE_ASSERT(isInLoopThread());
}

//-----------------------------------------------------------------------------
// ����: ���¼�ѭ���߳�������ִ��ָ���ķº���
// ��ע: �̰߳�ȫ
//-----------------------------------------------------------------------------
void EventLoop::executeInLoop(const Functor& functor)
{
    if (isInLoopThread())
        functor();
    else
        delegateToLoop(functor);
}

//-----------------------------------------------------------------------------
// ����: ��ָ���ķº���ί�и��¼�ѭ���߳�ִ�С��߳�����ɵ�ǰһ���¼�ѭ������
//       ִ�б�ί�еķº�����
// ��ע: �̰߳�ȫ
//-----------------------------------------------------------------------------
void EventLoop::delegateToLoop(const Functor& functor)
{
    {
        AutoLocker locker(delegatedFunctors_.mutex);
        delegatedFunctors_.items.push_back(functor);
    }

    wakeupLoop();
}

//-----------------------------------------------------------------------------
// ����: ���һ�������� (finalizer) ���¼�ѭ���У���ÿ��ѭ��������ִ������
//-----------------------------------------------------------------------------
void EventLoop::addFinalizer(const Functor& finalizer)
{
    AutoLocker locker(finalizers_.mutex);
    finalizers_.items.push_back(finalizer);
}

//-----------------------------------------------------------------------------
// ����: ��Ӷ�ʱ�� (ָ��ʱ��ִ��)
//-----------------------------------------------------------------------------
TimerId EventLoop::executeAt(Timestamp time, const TimerCallback& callback)
{
    return addTimer(time, 0, callback);
}

//-----------------------------------------------------------------------------
// ����: ��Ӷ�ʱ�� (�� delay �����ִ��)
//-----------------------------------------------------------------------------
TimerId EventLoop::executeAfter(INT64 delay, const TimerCallback& callback)
{
    Timestamp time(Timestamp::now() + delay);
    return addTimer(time, 0, callback);
}

//-----------------------------------------------------------------------------
// ����: ��Ӷ�ʱ�� (ÿ interval ����ѭ��ִ��)
//-----------------------------------------------------------------------------
TimerId EventLoop::executeEvery(INT64 interval, const TimerCallback& callback)
{
    Timestamp time(Timestamp::now() + interval);
    return addTimer(time, interval, callback);
}

//-----------------------------------------------------------------------------
// ����: ȡ����ʱ��
//-----------------------------------------------------------------------------
void EventLoop::cancelTimer(TimerId timerId)
{
    executeInLoop(boost::bind(&TimerQueue::cancelTimer, &timerQueue_, timerId));
}

//-----------------------------------------------------------------------------
// ����: ִ���¼�ѭ��
//-----------------------------------------------------------------------------
void EventLoop::runLoop(Thread *thread)
{
    bool isTerminated = false;
    while (!isTerminated)
    {
        try
        {
	        if (thread->isTerminated())
            {
                isTerminated = true;
                wakeupLoop();
            }

            doLoopWork(thread);
            executeDelegatedFunctors();
            executeFinalizer();
        }
        catch (Exception& e)
        {
            logger().writeException(e);
        }
        catch (...)
        {}
    }
}

//-----------------------------------------------------------------------------
// ����: ִ�б�ί�еķº���
//-----------------------------------------------------------------------------
void EventLoop::executeDelegatedFunctors()
{
    Functors functors;
    {
        AutoLocker locker(delegatedFunctors_.mutex);
        functors.swap(delegatedFunctors_.items);
    }

    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();
}

//-----------------------------------------------------------------------------
// ����: ִ������������
//-----------------------------------------------------------------------------
void EventLoop::executeFinalizer()
{
    Functors finalizers;
    {
        AutoLocker locker(finalizers_.mutex);
        finalizers.swap(finalizers_.items);
    }

    for (size_t i = 0; i < finalizers.size(); ++i)
        finalizers[i]();
}

//-----------------------------------------------------------------------------
// ����: ���¼�ѭ������ȴ�ǰ������ȴ���ʱʱ�� (����)
//-----------------------------------------------------------------------------
int EventLoop::calcLoopWaitTimeout()
{
    int result = TIMEOUT_INFINITE;
    Timestamp expiration;

    if (timerQueue_.getNearestExpiration(expiration))
    {
        Timestamp now(Timestamp::now());
        if (expiration <= now)
            result = 0;
        else
            result = ise::max((int)(expiration - now), 1);
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: �¼�ѭ���ȴ���Ϻ󣬴���ʱ���¼�
//-----------------------------------------------------------------------------
void EventLoop::processExpiredTimers()
{
    timerQueue_.processExpiredTimers(Timestamp::now());
}

//-----------------------------------------------------------------------------
// ����: ��Ӷ�ʱ�� (�̰߳�ȫ)
//-----------------------------------------------------------------------------
TimerId EventLoop::addTimer(Timestamp expiration, INT64 interval, const TimerCallback& callback)
{
    Timer *timer = new Timer(expiration, interval, callback);

    // �˴�������� delegateToLoop������������ executeInLoop����Ϊǰ���ܱ�֤ wakeupLoop��
    // �Ӷ��������¼����¼�ѭ���ĵȴ���ʱʱ�䡣
    delegateToLoop(boost::bind(&TimerQueue::addTimer, &timerQueue_, timer));

    return timer->timerId();
}

///////////////////////////////////////////////////////////////////////////////
// class EventLoopThread

EventLoopThread::EventLoopThread(EventLoop& eventLoop) :
    eventLoop_(eventLoop)
{
    setAutoDelete(false);
}

//-----------------------------------------------------------------------------

void EventLoopThread::execute()
{
    eventLoop_.loopThreadId_ = getThreadId();
    eventLoop_.runLoop(this);
}

//-----------------------------------------------------------------------------

void EventLoopThread::afterExecute()
{
    eventLoop_.loopThreadId_ = 0;
}

///////////////////////////////////////////////////////////////////////////////
// class EventLoopList

EventLoopList::EventLoopList(int loopCount) :
    items_(false, true),
    wantLoopCount_(loopCount)
{
    // nothing
}

EventLoopList::~EventLoopList()
{
    // nothing
}

//-----------------------------------------------------------------------------
// ����: ����ȫ�� eventLoop �Ĺ����߳�
//-----------------------------------------------------------------------------
void EventLoopList::start()
{
    if (getCount() == 0)
        setCount(wantLoopCount_);

    for (int i = 0; i < items_.getCount(); i++)
        items_[i]->start();
}

//-----------------------------------------------------------------------------
// ����: ȫ�� eventLoop ֹͣ����
//-----------------------------------------------------------------------------
void EventLoopList::stop()
{
    const int MAX_WAIT_FOR_SECS = 10;   // (��)
    const double SLEEP_INTERVAL = 0.5;  // (��)

    // ֹ֪ͨͣ
    for (int i = 0; i < items_.getCount(); i++)
        items_[i]->stop(false, false);

    // �ȴ�ֹͣ
    double waitSecs = 0;
    while (waitSecs < MAX_WAIT_FOR_SECS)
    {
        int runningCount = 0;
        for (int i = 0; i < items_.getCount(); i++)
            if (items_[i]->isRunning()) runningCount++;

        if (runningCount == 0) break;

        sleepSeconds(SLEEP_INTERVAL, true);
        waitSecs += SLEEP_INTERVAL;
    }

    // ǿ��ֹͣ
    for (int i = 0; i < items_.getCount(); i++)
        items_[i]->stop(true, true);
}

//-----------------------------------------------------------------------------
// ����: �����¼�ѭ���߳�ID���Ҷ�Ӧ���¼�ѭ�����Ҳ�������NULL
//-----------------------------------------------------------------------------
EventLoop* EventLoopList::findEventLoop(THREAD_ID loopThreadId)
{
    for (int i = 0; i < (int)items_.getCount(); ++i)
    {
        EventLoop *eventLoop = items_[i];
        if (eventLoop->getLoopThreadId() == loopThreadId)
            return eventLoop;
    }

    return NULL;
}

//-----------------------------------------------------------------------------

EventLoop* EventLoopList::createEventLoop()
{
    return new OsEventLoop();
}

//-----------------------------------------------------------------------------
// ����: ���� eventLoop �ĸ���
//-----------------------------------------------------------------------------
void EventLoopList::setCount(int count)
{
    count = ensureRange(count, 1, (int)MAX_LOOP_COUNT);

    for (int i = 0; i < count; i++)
        items_.add(createEventLoop());
}

///////////////////////////////////////////////////////////////////////////////
// class OsEventLoop

OsEventLoop::OsEventLoop()
{
#ifdef ISE_WINDOWS
    iocpObject_ = new IocpObject(this);
#endif
#ifdef ISE_LINUX
    epollObject_ = new EpollObject(this);
#endif
}

OsEventLoop::~OsEventLoop()
{
    stop(false, true);

#ifdef ISE_WINDOWS
    delete iocpObject_;
#endif
#ifdef ISE_LINUX
    delete epollObject_;
#endif
}

//-----------------------------------------------------------------------------
// ����: ִ�е����¼�ѭ���еĹ���
//-----------------------------------------------------------------------------
void OsEventLoop::doLoopWork(Thread *thread)
{
#ifdef ISE_WINDOWS
    iocpObject_->work();
#endif
#ifdef ISE_LINUX
    epollObject_->poll();
#endif
}

//-----------------------------------------------------------------------------
// ����: �����¼�ѭ���е���������
//-----------------------------------------------------------------------------
void OsEventLoop::wakeupLoop()
{
#ifdef ISE_WINDOWS
    iocpObject_->wakeup();
#endif
#ifdef ISE_LINUX
    epollObject_->wakeup();
#endif
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise
