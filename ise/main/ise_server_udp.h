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
// ise_server_udp.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_SERVER_UDP_H_
#define _ISE_SERVER_UDP_H_

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

class ThreadTimeoutChecker;
class UdpPacket;
class UdpRequestQueue;
class UdpWorkerThread;
class UdpWorkerThreadPool;
class UdpRequestGroup;
class MainUdpServer;

///////////////////////////////////////////////////////////////////////////////
// class ThreadTimeoutChecker - �̳߳�ʱ�����
//
// ˵��:
// ����������� UdpWorkerThread�����й������̵߳Ĺ���ʱ�䳬ʱ��⡣
// ���������߳��յ�һ����������Ͻ��빤��״̬��һ����ԣ��������߳�Ϊ�����������������
// ʱ�䲻��̫������̫����ᵼ�·��������й������̶߳�ȱ��ʹ��Ӧ����������������½�������
// ����UDP������˵���������ˡ�ͨ������£��̹߳�����ʱ����������Ϊ��������̺��߼�����
// �������ⲿԭ�򣬱������ݿⷱæ����Դ����������ӵ�µȵȡ����̹߳�����ʱ��Ӧ֪ͨ���˳���
// ����֪ͨ�˳�������ʱ������δ�˳�����ǿ��ɱ�����������̵߳�����������ʱ�����µ��̡߳�

class ThreadTimeoutChecker : public AutoInvokable
{
public:
    explicit ThreadTimeoutChecker(Thread *thread);
    virtual ~ThreadTimeoutChecker() {}

    // ����߳��Ƿ��ѳ�ʱ������ʱ��֪ͨ���˳�
    bool check();

    // ���ó�ʱʱ�䣬��Ϊ0���ʾ�����г�ʱ���
    void setTimeoutSecs(UINT value) { timeoutSecs_ = value; }
    // �����Ƿ��ѿ�ʼ��ʱ
    bool isStarted();

protected:
    virtual void invokeInitialize() { start(); }
    virtual void invokeFinalize() { stop(); }

private:
    void start();
    void stop();

private:
    Thread *thread_;          // �������߳�
    time_t startTime_;        // ��ʼ��ʱʱ��ʱ���
    bool started_;            // �Ƿ��ѿ�ʼ��ʱ
    UINT timeoutSecs_;        // ������������Ϊ��ʱ (Ϊ0��ʾ�����г�ʱ���)
    Mutex mutex_;
};

///////////////////////////////////////////////////////////////////////////////
// class UdpPacket - UDP���ݰ���

class UdpPacket : boost::noncopyable
{
public:
    UdpPacket() :
        recvTimestamp_(0),
        peerAddr_(0, 0),
        packetSize_(0),
        packetBuffer_(NULL)
    {}
    virtual ~UdpPacket()
        { if (packetBuffer_) free(packetBuffer_); }

    void setPacketBuffer(void *packetBuffer, int packetSize);
    void* getPacketBuffer() const { return packetBuffer_; }

    const InetAddress& getPeerAddr() const { return peerAddr_; }
    int getPacketSize() const { return packetSize_; }

public:
    time_t recvTimestamp_;
    InetAddress peerAddr_;
    int packetSize_;

private:
    void *packetBuffer_;
};

///////////////////////////////////////////////////////////////////////////////
// class UdpRequestQueue - UDP���������

class UdpRequestQueue : boost::noncopyable
{
public:
    explicit UdpRequestQueue(UdpRequestGroup *ownGroup);
    virtual ~UdpRequestQueue() { clear(); }

    void addPacket(UdpPacket *packet);
    UdpPacket* extractPacket();
    void clear();
    void wakeupWaiting();

    int getCount() { return packetCount_; }

private:
    typedef std::deque<UdpPacket*> PacketList;

    UdpRequestGroup *ownGroup_;    // �������
    PacketList packetList_;        // ���ݰ��б�
    int packetCount_;              // ���������ݰ��ĸ���(Ϊ�˿��ٷ���)
    int capacity_;                 // ���е��������
    int maxWaitTime_;              // ���ݰ����ȴ�ʱ��(��)
    Condition::Mutex mutex_;
    Condition condition_;
};

///////////////////////////////////////////////////////////////////////////////
// class UdpWorkerThread - UDP�������߳���
//
// ˵��:
// 1. ȱʡ����£�UDP�������߳�������г�ʱ��⣬��ĳЩ���������ó�ʱ��⣬����:
//    UdpWorkerThread::getTimeoutChecker().setTimeoutSecs(0);
//
// ���ʽ���:
// 1. ��ʱ�߳�: ��ĳһ������빤��״̬������δ��ɵ��̡߳�
// 2. �����߳�: �ѱ�֪ͨ�˳������ò��˳����̡߳�

class UdpWorkerThread : public Thread
{
public:
    explicit UdpWorkerThread(UdpWorkerThreadPool *threadPool);
    virtual ~UdpWorkerThread();

    // ���س�ʱ�����
    ThreadTimeoutChecker& getTimeoutChecker() { return timeoutChecker_; }
    // ���ظ��߳��Ƿ����״̬(���ڵȴ�����)
    bool isIdle() { return !timeoutChecker_.isStarted(); }

protected:
    virtual void execute();
    virtual void doTerminate();
    virtual void doKill();

private:
    UdpWorkerThreadPool *ownPool_;         // �����̳߳�
    ThreadTimeoutChecker timeoutChecker_;  // ��ʱ�����
};

///////////////////////////////////////////////////////////////////////////////
// class UdpWorkerThreadPool - UDP�������̳߳���

class UdpWorkerThreadPool : boost::noncopyable
{
public:
    enum
    {
        MAX_THREAD_TERM_SECS     = 60*3,    // �̱߳�֪ͨ�˳���������(��)
        MAX_THREAD_WAIT_FOR_SECS = 2        // �̳߳����ʱ���ȴ�ʱ��(��)
    };

public:
    explicit UdpWorkerThreadPool(UdpRequestGroup *ownGroup);
    virtual ~UdpWorkerThreadPool();

    void registerThread(UdpWorkerThread *thread);
    void unregisterThread(UdpWorkerThread *thread);

    // ���ݸ��������̬�����߳�����
    void AdjustThreadCount();
    // ֪ͨ�����߳��˳�
    void terminateAllThreads();
    // �ȴ������߳��˳�
    void waitForAllThreads();

    // ȡ�õ�ǰ�߳�����
    int getThreadCount() { return threadList_.getCount(); }
    // ȡ���������
    UdpRequestGroup& getRequestGroup() { return *ownGroup_; }

private:
    void createThreads(int count);
    void terminateThreads(int count);
    void checkThreadTimeout();
    void killZombieThreads();

private:
    UdpRequestGroup *ownGroup_;           // �������
    ThreadList threadList_;               // �߳��б�
};

///////////////////////////////////////////////////////////////////////////////
// class UdpRequestGroup - UDP���������

class UdpRequestGroup : boost::noncopyable
{
public:
    UdpRequestGroup(MainUdpServer *ownMainUdpSvr, int groupIndex);
    virtual ~UdpRequestGroup() {}

    int getGroupIndex() { return groupIndex_; }
    UdpRequestQueue& getRequestQueue() { return requestQueue_; }
    UdpWorkerThreadPool& getThreadPool() { return threadPool_; }

    // ȡ������UDP������
    MainUdpServer& getMainUdpServer() { return *ownMainUdpSvr_; }

private:
    MainUdpServer *ownMainUdpSvr_;         // ����UDP������
    int groupIndex_;                       // ����(0-based)
    UdpRequestQueue requestQueue_;         // �������
    UdpWorkerThreadPool threadPool_;       // �������̳߳�
};

///////////////////////////////////////////////////////////////////////////////
// class MainUdpServer - UDP����������

class MainUdpServer : boost::noncopyable
{
public:
    MainUdpServer();
    virtual ~MainUdpServer();

    void open();
    void close();

    void setLocalPort(WORD value) { udpServer_.setLocalPort(value); }
    void setListenerThreadCount(int value) { udpServer_.setListenerThreadCount(value); }

    // ���ݸ��������̬�����������߳�����
    void adjustWorkerThreadCount();
    // ֪ͨ���й������߳��˳�
    void terminateAllWorkerThreads();
    // �ȴ����й������߳��˳�
    void waitForAllWorkerThreads();

    BaseUdpServer& getUdpServer() { return udpServer_; }

private:
    void initUdpServer();
    void initRequestGroupList();
    void clearRequestGroupList();

    void onRecvData(void *packetBuffer, int packetSize, const InetAddress& peerAddr);

private:
    BaseUdpServer udpServer_;
    std::vector<UdpRequestGroup*> requestGroupList_;    // ��������б�
    int requestGroupCount_;                             // �����������
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_SERVER_UDP_H_
