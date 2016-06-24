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
// ise_application.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_APPLICATION_H_
#define _ISE_APPLICATION_H_

#include "ise/main/ise_options.h"
#include "ise/main/ise_classes.h"
#include "ise/main/ise_thread.h"
#include "ise/main/ise_sys_utils.h"
#include "ise/main/ise_socket.h"
#include "ise/main/ise_exceptions.h"
#include "ise/main/ise_server_udp.h"
#include "ise/main/ise_server_tcp.h"
#include "ise/main/ise_server_assistor.h"
#include "ise/main/ise_sys_threads.h"
#include "ise/main/ise_timer.h"

#ifdef ISE_WINDOWS
#include <stdarg.h>
#include <windows.h>
#endif

#ifdef ISE_LINUX
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#endif

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ��ǰ����

class IseBusiness;
class IseOptions;
class IseMainServer;
class IseApplication;

///////////////////////////////////////////////////////////////////////////////
// ���Ͷ���

// ����������(�ɶ�ѡ��ѡ)
enum SERVER_TYPE
{
    ST_UDP = 0x0001,     // UDP������
    ST_TCP = 0x0002      // TCP������
};

// �û��źŴ���ص�
typedef boost::function<void (int signalNumber)> UserSignalHandlerCallback;

///////////////////////////////////////////////////////////////////////////////
// interfaces

class UdpCallbacks
{
public:
    virtual ~UdpCallbacks() {}

    // UDP���ݰ�����
    virtual void classifyUdpPacket(void *packetBuffer, int packetSize, int& groupIndex) = 0;
    // �յ���UDP���ݰ�
    virtual void onRecvedUdpPacket(UdpWorkerThread& workerThread, int groupIndex, UdpPacket& packet) = 0;
};

class TcpCallbacks
{
public:
    virtual ~TcpCallbacks() {}

    // ������һ���µ�TCP����
    virtual void onTcpConnected(const TcpConnectionPtr& connection) = 0;
    // �Ͽ���һ��TCP����
    virtual void onTcpDisconnected(const TcpConnectionPtr& connection) = 0;
    // TCP�����ϵ�һ���������������
    virtual void onTcpRecvComplete(const TcpConnectionPtr& connection, void *packetBuffer,
        int packetSize, const Context& context) = 0;
    // TCP�����ϵ�һ���������������
    virtual void onTcpSendComplete(const TcpConnectionPtr& connection, const Context& context) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// class IseBusiness - ISEҵ�����

class IseBusiness :
    boost::noncopyable,
    public UdpCallbacks,
    public TcpCallbacks
{
public:
    IseBusiness() {}
    virtual ~IseBusiness() {}

    // ��ʼ�� (ʧ�����׳��쳣)
    virtual void initialize() {}
    // ������ (���۳�ʼ���Ƿ����쳣������ʱ����ִ��)
    virtual void finalize() {}

    // ���������в�������������ȷ�򷵻� false
    virtual bool parseArguments(int argc, char *argv[]) { return true; }
    // ���س���ĵ�ǰ�汾��
    virtual string getAppVersion() { return "0.0.0.0"; }
    // ���س���İ�����Ϣ
    virtual string getAppHelp() { return ""; }

    // ��ʼ��ISE������Ϣ
    virtual void initIseOptions(IseOptions& options) {}

    // ��ʼ��֮ǰ
    virtual void beforeInit() {}
    // ��ʼ���ɹ�֮��
    virtual void afterInit() {}
    // ��ʼ��ʧ�� (��Ӧ�׳��쳣)
    virtual void onInitFailed(Exception& e) {}

public:  /* interface UdpCallbacks */
    // UDP���ݰ�����
    virtual void classifyUdpPacket(void *packetBuffer, int packetSize, int& groupIndex) { groupIndex = 0; }
    // �յ���UDP���ݰ�
    virtual void onRecvedUdpPacket(UdpWorkerThread& workerThread, int groupIndex, UdpPacket& packet) {}

public:  /* interface TcpCallbacks */
    // ������һ���µ�TCP����
    virtual void onTcpConnected(const TcpConnectionPtr& connection) {}
    // �Ͽ���һ��TCP����
    virtual void onTcpDisconnected(const TcpConnectionPtr& connection) {}
    // TCP�����ϵ�һ���������������
    virtual void onTcpRecvComplete(const TcpConnectionPtr& connection, void *packetBuffer,
        int packetSize, const Context& context) {}
    // TCP�����ϵ�һ���������������
    virtual void onTcpSendComplete(const TcpConnectionPtr& connection, const Context& context) {}

public:
    // ���������߳�ִ��(assistorIndex: 0-based)
    virtual void assistorThreadExecute(AssistorThread& assistorThread, int assistorIndex) {}
    // ϵͳ�ػ��߳�ִ�� (secondCount: 0-based)
    virtual void daemonThreadExecute(Thread& thread, int secondCount) {}
};

///////////////////////////////////////////////////////////////////////////////
// class IseOptions - ISE������

class IseOptions : boost::noncopyable
{
public:
    // ��������������ȱʡֵ
    enum
    {
        DEF_SERVER_TYPE                 = 0,             // ������Ĭ������
        DEF_ASSISTOR_THREAD_COUNT       = 0,             // �����̵߳ĸ���
    };

    // UDP����������ȱʡֵ
    enum
    {
        DEF_UDP_SERVER_PORT             = 8000,          // UDP����Ĭ�϶˿�
        DEF_UDP_LISTENER_THREAD_COUNT   = 1,             // �����̵߳�����
        DEF_UDP_REQ_GROUP_COUNT         = 1,             // �������������ȱʡֵ
        DEF_UDP_REQ_QUEUE_CAPACITY      = 5000,          // ������е�ȱʡ����(���ܷ��¶������ݰ�)
        DEF_UDP_WORKER_THREADS_MIN      = 1,             // ÿ������й������̵߳�ȱʡ���ٸ���
        DEF_UDP_WORKER_THREADS_MAX      = 8,             // ÿ������й������̵߳�ȱʡ������
        DEF_UDP_REQ_MAX_WAIT_TIME       = 10,            // �����ڶ����е���ȴ�ʱ��ȱʡֵ(��)
        DEF_UDP_WORKER_THREAD_TIMEOUT   = 60,            // �������̵߳Ĺ�����ʱʱ��ȱʡֵ(��)
        DEF_UDP_QUEUE_ALERT_LINE        = 500,           // ���������ݰ�����������ȱʡֵ�����������������������߳�
        DEF_UDP_ADJUST_THREAD_INTERVAL  = 5,             // ��̨���� "�������߳�����" ��ʱ����ȱʡֵ(��)
    };

    // TCP����������ȱʡֵ
    enum
    {
        DEF_TCP_SERVER_PORT             = 8000,          // TCP����Ĭ�϶˿�
        DEF_TCP_SERVER_COUNT            = 1,             // TCP������������ȱʡֵ
        DEF_TCP_SERVER_EVENT_LOOP_COUNT = 1,             // TCP���������¼�ѭ��������ȱʡֵ
        DEF_TCP_CLIENT_EVENT_LOOP_COUNT = 1,             // ����ȫ��TCP�ͻ��˵��¼�ѭ���ĸ�����ȱʡֵ
        DEF_TCP_MAX_RECV_BUFFER_SIZE    = 1024*1024*1,   // TCP���ջ��������ֽ���
    };

    // UDP������������
    struct UdpRequestGroupOption
    {
        int requestQueueCapacity;      // ������е�����(�������ɶ��ٸ����ݰ�)
        int minWorkerThreads;          // �������̵߳����ٸ���
        int maxWorkerThreads;          // �������̵߳�������

        UdpRequestGroupOption()
        {
            requestQueueCapacity = DEF_UDP_REQ_QUEUE_CAPACITY;
            minWorkerThreads = DEF_UDP_WORKER_THREADS_MIN;
            maxWorkerThreads = DEF_UDP_WORKER_THREADS_MAX;
        }
    };
    typedef std::vector<UdpRequestGroupOption> UdpRequestGroupOptions;

    // TCP������������
    struct TcpServerOption
    {
        int serverPort;                // TCP����˿ں�
        int eventLoopCount;            // �¼�ѭ������

        TcpServerOption()
        {
            serverPort = DEF_TCP_SERVER_PORT;
            eventLoopCount = DEF_TCP_SERVER_EVENT_LOOP_COUNT;
        }
    };
    typedef std::vector<TcpServerOption> TcpServerOptions;

public:
    IseOptions();

    // ϵͳ��������/��ȡ-------------------------------------------------------

    void setLogFileName(const string& value, bool logNewFileDaily = false)
        { logFileName_ = value; logNewFileDaily_ = logNewFileDaily; }
    void setIsDaemon(bool value) { isDaemon_ = value; }
    void setAllowMultiInstance(bool value) { allowMultiInstance_ = value; }

    string getLogFileName() { return logFileName_; }
    bool getLogNewFileDaily() { return logNewFileDaily_; }
    bool getIsDaemon() { return isDaemon_; }
    bool getAllowMultiInstance() { return allowMultiInstance_; }

    // ��������������----------------------------------------------------------

    // ���÷���������(ST_UDP | ST_TCP)
    void setServerType(UINT serverType);
    // ���ø����̵߳�����
    void setAssistorThreadCount(int count);

    // ����UDP����˿ں�
    void setUdpServerPort(int port);
    // ����UDP�����̵߳�����
    void setUdpListenerThreadCount(int count);
    // ����UDP������������
    void setUdpRequestGroupCount(int count);
    // ����UDP������е�������� (�������ɶ��ٸ����ݰ�)
    void setUdpRequestQueueCapacity(int groupIndex, int capacity);
    // ����UDP�����ڶ����е���ȴ�ʱ�䣬��ʱ���账��(��)
    void setUdpRequestMaxWaitTime(int seconds);
    // ����UDP������������ݰ�����������
    void setUdpRequestQueueAlertLine(int count);
    // ����UDP�������̸߳�����������
    void setUdpWorkerThreadCount(int groupIndex, int minThreads, int maxThreads);
    // ����UDP�������̵߳Ĺ�����ʱʱ��(��)����Ϊ0��ʾ�����г�ʱ���
    void setUdpWorkerThreadTimeout(int seconds);
    // ���ú�̨����UDP�������߳�������ʱ����(��)
    void setUdpAdjustThreadInterval(int seconds);

    // ����TCP������������
    void setTcpServerCount(int count);
    // ����TCP����˿ں�
    void setTcpServerPort(int serverIndex, int port);
    void setTcpServerPort(int port) { setTcpServerPort(0, port); }
    // ����ÿ��TCP���������¼�ѭ���ĸ���
    void setTcpServerEventLoopCount(int serverIndex, int eventLoopCount);
    void setTcpServerEventLoopCount(int eventLoopCount) { setTcpServerEventLoopCount(0, eventLoopCount); }
    // ��������ȫ��TCP�ͻ��˵��¼�ѭ���ĸ���
    void setTcpClientEventLoopCount(int eventLoopCount);
    // ����TCP���ջ������޽�������ʱ������ֽ���
    void setTcpMaxRecvBufferSize(int bytes);

    // ���������û�ȡ----------------------------------------------------------

    UINT getServerType() { return serverType_; }
    int getAssistorThreadCount() { return assistorThreadCount_; }

    int getUdpServerPort() { return udpServerPort_; }
    int getUdpListenerThreadCount() { return udpListenerThreadCount_; }
    int getUdpRequestGroupCount() { return udpRequestGroupCount_; }
    int getUdpRequestQueueCapacity(int groupIndex);
    void getUdpWorkerThreadCount(int groupIndex, int& minThreads, int& maxThreads);
    int getUdpRequestMaxWaitTime() { return udpRequestMaxWaitTime_; }
    int getUdpRequestQueueAlertLine() { return udpRequestQueueAlertLine_; }
    int getUdpWorkerThreadTimeout() { return udpWorkerThreadTimeout_; }
    int getUdpAdjustThreadInterval() { return udpAdjustThreadInterval_; }

    int getTcpServerCount() { return tcpServerCount_; }
    int getTcpServerPort(int serverIndex);
    int getTcpServerEventLoopCount(int serverIndex);
    int getTcpClientEventLoopCount() { return tcpClientEventLoopCount_; }
    int getTcpMaxRecvBufferSize() { return tcpMaxRecvBufferSize_; }

private:
    /* ------------ ϵͳ����: ------------------ */

    string logFileName_;         // ��־�ļ��� (��·��)
    bool logNewFileDaily_;       // �Ƿ�ÿ����һ���������ļ��洢��־
    bool isDaemon_;              // �Ƿ��̨�ػ�����(daemon)
    bool allowMultiInstance_;    // �Ƿ�����������ʵ��ͬʱ����

    /* ------------ ��������������: ------------ */

    // ���������� (ST_UDP | ST_TCP)
    UINT serverType_;
    // �����̵߳ĸ���
    int assistorThreadCount_;

    /* ------------ UDP����������: ------------ */

    // UDP����˿�
    int udpServerPort_;
    // �����̵߳�����
    int udpListenerThreadCount_;
    // ������������
    int udpRequestGroupCount_;
    // ÿ������ڵ�����
    UdpRequestGroupOptions udpRequestGroupOpts_;
    // ���ݰ��ڶ����е���ȴ�ʱ�䣬��ʱ���账��(��)
    int udpRequestMaxWaitTime_;
    // �������̵߳Ĺ�����ʱʱ��(��)����Ϊ0��ʾ�����г�ʱ���
    int udpWorkerThreadTimeout_;
    // ������������ݰ����������ߣ����������������������߳�
    int udpRequestQueueAlertLine_;
    // ��̨����UDP�������߳�������ʱ����(��)
    int udpAdjustThreadInterval_;

    /* ------------ TCP����������: ------------ */

    // TCP������������ (һ��TCP������ռ��һ��TCP�˿�)
    int tcpServerCount_;
    // ÿ��TCP������������
    TcpServerOptions tcpServerOpts_;
    // ����ȫ��TCP�ͻ��˵��¼�ѭ���ĸ���
    int tcpClientEventLoopCount_;
    // TCP���ջ������޽�������ʱ������ֽ���
    int tcpMaxRecvBufferSize_;
};

///////////////////////////////////////////////////////////////////////////////
// class IseMainServer - ����������

class IseMainServer : boost::noncopyable
{
public:
    IseMainServer();
    virtual ~IseMainServer();

    void initialize();
    void finalize();
    void run();

    MainUdpServer& getMainUdpServer();
    MainTcpServer& getMainTcpServer();
    AssistorServer& getAssistorServer();
    TimerManager& getTimerManager();
    TcpConnector& getTcpConnector();
private:
    void runBackground();
private:
    MainUdpServer *udpServer_;            // UDP������
    MainTcpServer *tcpServer_;            // TCP������
    AssistorServer *assistorServer_;      // ����������
    TimerManager *timerManager_;          // ��ʱ��������
    TcpConnector *tcpConnector_;          // TCP������
    SysThreadMgr *sysThreadMgr_;          // ϵͳ�̹߳�����
};

///////////////////////////////////////////////////////////////////////////////
// class IseApplication - ISEӦ�ó�����
//
// ˵��:
// 1. ����������������������ܣ�ȫ�ֵ�������(Application)�ڳ�������ʱ����������
// 2. һ����˵��������������ⲿ��������(kill)���˳��ġ���ISE�У������յ�kill�˳������
//    (��ʱ��ǰִ�е�һ���� iseApp().run() ��)���ᴥ�� exitProgramSignalHandler
//    �źŴ��������������� longjmp ����ʹִ�е�ģ��� run() ���˳����̶�ִ�� finalize��
// 3. �������������ķǷ��������󣬻��ȴ��� fatalErrorSignalHandler �źŴ�������
//    Ȼ��ͬ����������������˳���˳���
// 4. �������ڲ��������˳������Ƽ�ʹ��exit���������� iseApp().setTeraminted(true).
//    ���������ó���������������˳���˳���

class IseApplication : boost::noncopyable
{
public:
    virtual ~IseApplication();
    static IseApplication& instance();

    bool parseArguments(int argc, char *argv[]);
    void initialize();
    void finalize();
    void run();

    IseOptions& iseOptions() { return iseOptions_; }
    IseBusiness& iseBusiness() { return *iseBusiness_; }
    IseMainServer& mainServer() { return *mainServer_; }
    BaseUdpServer& udpServer() { return mainServer_->getMainUdpServer().getUdpServer(); }
    BaseTcpServer& tcpServer(int index) { return mainServer_->getMainTcpServer().getTcpServer(index); }
    TimerManager& timerManager() { return mainServer_->getTimerManager(); }
    TcpConnector& tcpConnector() { return mainServer_->getTcpConnector(); }

    void setTerminated(bool value) { terminated_ = value; }
    bool isTerminated() { return terminated_; }

    // ȡ�ÿ�ִ���ļ���ȫ��(������·��)
    string getExeName() { return exeName_; }
    // ȡ�ÿ�ִ���ļ����ڵ�·��
    string getExePath();
    // ȡ�������в�������(�׸�����Ϊ����·���ļ���)
    int getArgCount() { return argList_.getCount(); }
    // ȡ�������в����ַ��� (index: 0-based)
    string getArgString(int index);
    // ȡ�ó�������ʱ��ʱ��
    time_t getAppStartTime() { return appStartTime_; }

    // ע���û��źŴ�����
    void registerUserSignalHandler(const UserSignalHandlerCallback& callback);

private:
    IseApplication();

private:
    bool processStandardArgs(bool isInitializing);
    void checkMultiInstance();
    void applyIseOptions();
    void createMainServer();
    void deleteMainServer();
    void createIseBusiness();
    void deleteIseBusiness();
    void initExeName();
    void initDaemon();
    void initSignals();
    void initNewOperHandler();
    void openTerminal();
    void closeTerminal();
    void doFinalize();

private:
    IseOptions iseOptions_;              // ISE����
    IseBusiness *iseBusiness_;           // ҵ�����
    IseMainServer *mainServer_;          // ��������
    StrList argList_;                    // �����в��� (������������ argv[0])
    string exeName_;                     // ��ִ���ļ���ȫ��(������·��)
    time_t appStartTime_;                // ��������ʱ��ʱ��
    bool initialized_;                   // �Ƿ�ɹ���ʼ��
    bool terminated_;                    // �Ƿ�Ӧ�˳��ı�־
    CallbackList<UserSignalHandlerCallback> onUserSignal_;  // �û��źŴ���ص�

    friend void userSignalHandler(int sigNo);
};

///////////////////////////////////////////////////////////////////////////////
// ȫ�ֺ���

inline IseApplication& iseApp() { return IseApplication::instance(); }

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_APPLICATION_H_
