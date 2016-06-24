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
// �ļ�����: ise_application.cpp
// ��������: ������Ӧ������Ԫ
///////////////////////////////////////////////////////////////////////////////

#include "ise/main/ise_application.h"
#include "ise/main/ise_err_msgs.h"

using namespace ise;

///////////////////////////////////////////////////////////////////////////////
// �ⲿ��������

IseBusiness* createIseBusinessObject();

///////////////////////////////////////////////////////////////////////////////
// ������

#ifdef ISE_WINDOWS

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        if (iseApp().parseArguments(__argc, __argv))
        {
            AutoFinalizer finalizer(boost::bind(&IseApplication::finalize, &iseApp()));

            iseApp().initialize();
            iseApp().run();
        }
    }
    catch (Exception& e)
    {
        logger().writeException(e);
    }

    return 0;
}

#endif
#ifdef ISE_LINUX

int main(int argc, char *argv[])
{
    try
    {
        if (iseApp().parseArguments(argc, argv))
        {
            AutoFinalizer finalizer(boost::bind(&IseApplication::finalize, &iseApp()));

            iseApp().initialize();
            iseApp().run();
        }
    }
    catch (Exception& e)
    {
        std::cout << e.makeLogStr() << std::endl << std::endl;
        logger().writeException(e);
    }

    return 0;
}

#endif

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ȫ�ֱ�������

#ifdef ISE_LINUX
// ���ڽ����˳�ʱ����ת
static sigjmp_buf procExitJmpBuf;
#endif

// �����ڴ治�������³����˳�
static char *reservedMemoryForExit;

///////////////////////////////////////////////////////////////////////////////
// �źŴ�����

#ifdef ISE_LINUX
//-----------------------------------------------------------------------------
// ����: �����˳����� �źŴ�����
//-----------------------------------------------------------------------------
void exitProgramSignalHandler(int sigNo)
{
    static bool isInHandler = false;
    if (isInHandler) return;
    isInHandler = true;

    // ֹͣ���߳�ѭ��
    iseApp().setTerminated(true);

    logger().writeFmt(SEM_SIG_TERM, sigNo);

    siglongjmp(procExitJmpBuf, 1);
}

//-----------------------------------------------------------------------------
// ����: �����Ƿ����� �źŴ�����
//-----------------------------------------------------------------------------
void fatalErrorSignalHandler(int sigNo)
{
    static bool isInHandler = false;
    if (isInHandler) return;
    isInHandler = true;

    // ֹͣ���߳�ѭ��
    iseApp().setTerminated(true);

    logger().writeFmt(SEM_SIG_FATAL_ERROR, sigNo);
    abort();
}
#endif

//-----------------------------------------------------------------------------
// ����: �û��źŴ�����
//-----------------------------------------------------------------------------
void userSignalHandler(int sigNo)
{
    const CallbackList<UserSignalHandlerCallback>& callbackList = iseApp().onUserSignal_;

    for (int i = 0; i < callbackList.getCount(); i++)
    {
        const UserSignalHandlerCallback& callback = callbackList.getItem(i);
        if (callback)
            callback(sigNo);
    }
}

//-----------------------------------------------------------------------------
// ����: �ڴ治���������
// ��ע:
//   ��δ��װ��������(set_new_handler)���� new ����ʧ��ʱ�׳� bad_alloc �쳣��
//   ����װ����������new �����������׳��쳣�����ǵ��ô�����������
//-----------------------------------------------------------------------------
void outOfMemoryHandler()
{
    static bool isInHandler = false;
    if (isInHandler) return;
    isInHandler = true;

    // �ͷű����ڴ棬��������˳��������ٴγ����ڴ治��
    delete[] reservedMemoryForExit;
    reservedMemoryForExit = NULL;

    logger().writeStr(SEM_OUT_OF_MEMORY);
    abort();
}

///////////////////////////////////////////////////////////////////////////////
// class IseOptions

IseOptions::IseOptions()
{
    setLogFileName(getAppSubPath("log") + changeFileExt(getAppExeName(false), ".log"), false);
    setIsDaemon(true);
    setAllowMultiInstance(false);

    setServerType(DEF_SERVER_TYPE);
    setAssistorThreadCount(DEF_ASSISTOR_THREAD_COUNT);

    setUdpServerPort(DEF_UDP_SERVER_PORT);
    setUdpListenerThreadCount(DEF_UDP_LISTENER_THREAD_COUNT);
    setUdpRequestGroupCount(DEF_UDP_REQ_GROUP_COUNT);
    for (int i = 0; i < DEF_UDP_REQ_GROUP_COUNT; i++)
    {
        setUdpRequestQueueCapacity(i, DEF_UDP_REQ_QUEUE_CAPACITY);
        setUdpWorkerThreadCount(i, DEF_UDP_WORKER_THREADS_MIN, DEF_UDP_WORKER_THREADS_MAX);
    }
    setUdpRequestMaxWaitTime(DEF_UDP_REQ_MAX_WAIT_TIME);
    setUdpWorkerThreadTimeout(DEF_UDP_WORKER_THREAD_TIMEOUT);
    setUdpRequestQueueAlertLine(DEF_UDP_QUEUE_ALERT_LINE);
    setUdpAdjustThreadInterval(DEF_UDP_ADJUST_THREAD_INTERVAL);

    setTcpServerCount(DEF_TCP_SERVER_COUNT);
    for (int i = 0; i < DEF_TCP_SERVER_COUNT; i++)
        setTcpServerPort(i, DEF_TCP_SERVER_PORT);
    for (int i = 0; i < DEF_TCP_SERVER_COUNT; i++)
        setTcpServerEventLoopCount(i, DEF_TCP_SERVER_EVENT_LOOP_COUNT);
    setTcpClientEventLoopCount(DEF_TCP_CLIENT_EVENT_LOOP_COUNT);
    setTcpMaxRecvBufferSize(DEF_TCP_MAX_RECV_BUFFER_SIZE);
}

//-----------------------------------------------------------------------------
// ����: ���÷���������(UDP|TCP)
// ����:
//   serverType - ������������(�ɶ�ѡ��ѡ)
// ʾ��:
//   setServerType(ST_UDP | ST_TCP);
//-----------------------------------------------------------------------------
void IseOptions::setServerType(UINT serverType)
{
    serverType_ = serverType;
}

//-----------------------------------------------------------------------------
// ����: ���ø����̵߳�����
//-----------------------------------------------------------------------------
void IseOptions::setAssistorThreadCount(int count)
{
    count = ise::max(count, 0);
    assistorThreadCount_ = count;
}

//-----------------------------------------------------------------------------
// ����: ����UDP����˿ں�
//-----------------------------------------------------------------------------
void IseOptions::setUdpServerPort(int port)
{
    udpServerPort_ = port;
}

//-----------------------------------------------------------------------------
// ����: ����UDP�����̵߳�����
//-----------------------------------------------------------------------------
void IseOptions::setUdpListenerThreadCount(int count)
{
    count = ise::max(count, 1);
    udpListenerThreadCount_ = count;
}

//-----------------------------------------------------------------------------
// ����: ����UDP���ݰ����������
//-----------------------------------------------------------------------------
void IseOptions::setUdpRequestGroupCount(int count)
{
    if (count <= 0) count = DEF_UDP_REQ_GROUP_COUNT;

    udpRequestGroupCount_ = count;
    udpRequestGroupOpts_.resize(count);
}

//-----------------------------------------------------------------------------
// ����: ����UDP������е�������� (�������ɶ��ٸ����ݰ�)
// ����:
//   groupIndex - ���� (0-based)
//   capacity   - ����
//-----------------------------------------------------------------------------
void IseOptions::setUdpRequestQueueCapacity(int groupIndex, int capacity)
{
    if (groupIndex < 0 || groupIndex >= udpRequestGroupCount_) return;

    if (capacity <= 0) capacity = DEF_UDP_REQ_QUEUE_CAPACITY;

    udpRequestGroupOpts_[groupIndex].requestQueueCapacity = capacity;
}

//-----------------------------------------------------------------------------
// ����: ����UDP�����ڶ����е���Ч�ȴ�ʱ�䣬��ʱ���账��
// ����:
//   nMSecs - �ȴ�����
//-----------------------------------------------------------------------------
void IseOptions::setUdpRequestMaxWaitTime(int seconds)
{
    if (seconds <= 0) seconds = DEF_UDP_REQ_MAX_WAIT_TIME;
    udpRequestMaxWaitTime_ = seconds;
}

//-----------------------------------------------------------------------------
// ����: ����������������ݰ�����������
//-----------------------------------------------------------------------------
void IseOptions::setUdpRequestQueueAlertLine(int count)
{
    count = ise::max(count, 1);
    udpRequestQueueAlertLine_ = count;
}

//-----------------------------------------------------------------------------
// ����: ����UDP�������̸߳�����������
// ����:
//   groupIndex - ���� (0-based)
//   minThreads - �̸߳���������
//   maxThreads - �̸߳���������
//-----------------------------------------------------------------------------
void IseOptions::setUdpWorkerThreadCount(int groupIndex, int minThreads, int maxThreads)
{
    if (groupIndex < 0 || groupIndex >= udpRequestGroupCount_) return;

    if (minThreads < 1) minThreads = 1;
    if (maxThreads < minThreads) maxThreads = minThreads;

    udpRequestGroupOpts_[groupIndex].minWorkerThreads = minThreads;
    udpRequestGroupOpts_[groupIndex].maxWorkerThreads = maxThreads;
}

//-----------------------------------------------------------------------------
// ����: ����UDP�������̵߳Ĺ�����ʱʱ��(��)����Ϊ0��ʾ�����г�ʱ���
//-----------------------------------------------------------------------------
void IseOptions::setUdpWorkerThreadTimeout(int seconds)
{
    seconds = ise::max(seconds, 0);
    udpWorkerThreadTimeout_ = seconds;
}

//-----------------------------------------------------------------------------
// ����: ���ú�̨ά��UDP�������߳�������ʱ����(��)
//-----------------------------------------------------------------------------
void IseOptions::setUdpAdjustThreadInterval(int seconds)
{
    if (seconds <= 0) seconds = DEF_UDP_ADJUST_THREAD_INTERVAL;
    udpAdjustThreadInterval_ = seconds;
}

//-----------------------------------------------------------------------------
// ����: ����TCP���ݰ����������
//-----------------------------------------------------------------------------
void IseOptions::setTcpServerCount(int count)
{
    count = ise::max(count, 0);

    tcpServerCount_ = count;
    tcpServerOpts_.resize(count);
}

//-----------------------------------------------------------------------------
// ����: ����TCP����˿ں�
// ����:
//   serverIndex - TCP��������� (0-based)
//   port        - �˿ں�
//-----------------------------------------------------------------------------
void IseOptions::setTcpServerPort(int serverIndex, int port)
{
    if (serverIndex < 0 || serverIndex >= tcpServerCount_) return;

    tcpServerOpts_[serverIndex].serverPort = port;
}

//-----------------------------------------------------------------------------
// ����: ����ÿ��TCP���������¼�ѭ���ĸ���
// ����:
//   serverIndex    - TCP��������� (0-based)
//   eventLoopCount - �¼�ѭ������
//-----------------------------------------------------------------------------
void IseOptions::setTcpServerEventLoopCount(int serverIndex, int eventLoopCount)
{
    if (serverIndex < 0 || serverIndex >= tcpServerCount_) return;

    eventLoopCount = ise::max(eventLoopCount, 1);
    tcpServerOpts_[serverIndex].eventLoopCount = eventLoopCount;
}

//-----------------------------------------------------------------------------
// ����: ��������ȫ��TCP�ͻ��˵��¼�ѭ���ĸ���
//-----------------------------------------------------------------------------
void IseOptions::setTcpClientEventLoopCount(int eventLoopCount)
{
    eventLoopCount = ise::max(eventLoopCount, 1);
    tcpClientEventLoopCount_ = eventLoopCount;
}

//-----------------------------------------------------------------------------
// ����: ����TCP���ջ������޽�������ʱ������ֽ���
//-----------------------------------------------------------------------------
void IseOptions::setTcpMaxRecvBufferSize(int bytes)
{
    bytes = ise::max(bytes, 0);
    tcpMaxRecvBufferSize_ = bytes;
}

//-----------------------------------------------------------------------------
// ����: ȡ��UDP������е�������� (�������ɶ��ٸ����ݰ�)
// ����:
//   groupIndex - ���� (0-based)
//-----------------------------------------------------------------------------
int IseOptions::getUdpRequestQueueCapacity(int groupIndex)
{
    if (groupIndex < 0 || groupIndex >= udpRequestGroupCount_) return -1;

    return udpRequestGroupOpts_[groupIndex].requestQueueCapacity;
}

//-----------------------------------------------------------------------------
// ����: ȡ��UDP�������̸߳�����������
// ����:
//   groupIndex - ���� (0-based)
//   minThreads - ����̸߳���������
//   maxThreads - ����̸߳���������
//-----------------------------------------------------------------------------
void IseOptions::getUdpWorkerThreadCount(int groupIndex, int& minThreads, int& maxThreads)
{
    if (groupIndex < 0 || groupIndex >= udpRequestGroupCount_) return;

    minThreads = udpRequestGroupOpts_[groupIndex].minWorkerThreads;
    maxThreads = udpRequestGroupOpts_[groupIndex].maxWorkerThreads;
}

//-----------------------------------------------------------------------------
// ����: ȡ��TCP����˿ں�
// ����:
//   serverIndex - TCP����������� (0-based)
//-----------------------------------------------------------------------------
int IseOptions::getTcpServerPort(int serverIndex)
{
    if (serverIndex < 0 || serverIndex >= tcpServerCount_) return -1;

    return tcpServerOpts_[serverIndex].serverPort;
}

//-----------------------------------------------------------------------------
// ����: ȡ��TCP���������¼�ѭ���ĸ���
// ����:
//   serverIndex - TCP����������� (0-based)
//-----------------------------------------------------------------------------
int IseOptions::getTcpServerEventLoopCount(int serverIndex)
{
    if (serverIndex < 0 || serverIndex >= tcpServerCount_) return -1;

    return tcpServerOpts_[serverIndex].eventLoopCount;
}

///////////////////////////////////////////////////////////////////////////////
// class IseMainServer

IseMainServer::IseMainServer() :
    udpServer_(NULL),
    tcpServer_(NULL),
    assistorServer_(NULL),
    timerManager_(NULL),
    tcpConnector_(NULL),
    sysThreadMgr_(NULL)
{
    // nothing
}

IseMainServer::~IseMainServer()
{
    // nothing
}

//-----------------------------------------------------------------------------
// ����: ��������ʼ�� (����ʼ��ʧ�����׳��쳣)
// ��ע: �� iseApp().initialize() ����
//-----------------------------------------------------------------------------
void IseMainServer::initialize()
{
    // ��ʱ��������
    timerManager_ = new TimerManager();

    // ��ʼ�� UDP ������
    if (iseApp().iseOptions().getServerType() & ST_UDP)
    {
        udpServer_ = new MainUdpServer();
        udpServer_->setLocalPort(static_cast<WORD>(iseApp().iseOptions().getUdpServerPort()));
        udpServer_->setListenerThreadCount(iseApp().iseOptions().getUdpListenerThreadCount());
        udpServer_->open();
    }

    // ��ʼ�� TCP ������
    if (iseApp().iseOptions().getServerType() & ST_TCP)
    {
        tcpServer_ = new MainTcpServer();
        tcpServer_->open();
    }

    // ��ʼ������������
    assistorServer_ = new AssistorServer();
    assistorServer_->open();

    // TCP������
    tcpConnector_ = new TcpConnector();

    // ��ʼ��ϵͳ�̹߳�����
    sysThreadMgr_ = new SysThreadMgr();
    sysThreadMgr_->initialize();
}

//-----------------------------------------------------------------------------
// ����: ������������
// ��ע: �� iseApp().finalize() ���ã��� IseMainServer �����������в��ص���
//-----------------------------------------------------------------------------
void IseMainServer::finalize()
{
    if (assistorServer_)
    {
        assistorServer_->close();
        delete assistorServer_;
        assistorServer_ = NULL;
    }

    if (udpServer_)
    {
        udpServer_->close();
        delete udpServer_;
        udpServer_ = NULL;
    }

    if (tcpServer_)
    {
        tcpServer_->close();
        delete tcpServer_;
        tcpServer_ = NULL;
    }

    if (tcpConnector_)
    {
        delete tcpConnector_;
        tcpConnector_ = NULL;
    }

    if (sysThreadMgr_)
    {
        sysThreadMgr_->finalize();
        delete sysThreadMgr_;
        sysThreadMgr_ = NULL;
    }

    if (timerManager_)
    {
        delete timerManager_;
        timerManager_ = NULL;
    }
}

//-----------------------------------------------------------------------------
// ����: ��ʼ���з�����
// ��ע: �� iseApp().run() ����
//-----------------------------------------------------------------------------
void IseMainServer::run()
{
    runBackground();
}

//-----------------------------------------------------------------------------

MainUdpServer& IseMainServer::getMainUdpServer()
{
    ISE_ASSERT(udpServer_ != NULL);
    return *udpServer_;
}

//-----------------------------------------------------------------------------

MainTcpServer& IseMainServer::getMainTcpServer()
{
    ISE_ASSERT(tcpServer_ != NULL);
    return *tcpServer_;
}

//-----------------------------------------------------------------------------

AssistorServer& IseMainServer::getAssistorServer()
{
    ISE_ASSERT(assistorServer_ != NULL);
    return *assistorServer_;
}

//-----------------------------------------------------------------------------

TimerManager& IseMainServer::getTimerManager()
{
    ISE_ASSERT(timerManager_ != NULL);
    return *timerManager_;
}

//-----------------------------------------------------------------------------

TcpConnector& IseMainServer::getTcpConnector()
{
    ISE_ASSERT(tcpConnector_ != NULL);
    return *tcpConnector_;
}

//-----------------------------------------------------------------------------
// ����: ��������ʼ���к����߳̽��к�̨�ػ�����
//-----------------------------------------------------------------------------
void IseMainServer::runBackground()
{
    int adjustThreadInterval = iseApp().iseOptions().getUdpAdjustThreadInterval();
    int secondCount = 0;

    while (!iseApp().isTerminated())
    try
    {
        try
        {
            // ÿ�� adjustThreadInterval ��ִ��һ��
            if ((secondCount % adjustThreadInterval) == 0)
            {
#ifdef ISE_LINUX
                // ��ʱ�����˳��ź�
                SignalMasker sigMasker(true);
                sigMasker.setSignals(1, SIGTERM);
                sigMasker.block();
#endif

                // ά���������̵߳�����
                if (udpServer_) udpServer_->adjustWorkerThreadCount();
            }
        }
        catch (...)
        {}

        secondCount++;
        sleepSeconds(1, true);  // 1��
    }
    catch (...)
    {}
}

///////////////////////////////////////////////////////////////////////////////
// class IseApplication

IseApplication::IseApplication() :
    iseBusiness_(NULL),
    mainServer_(NULL),
    appStartTime_(time(NULL)),
    initialized_(false),
    terminated_(false)
{
    createIseBusiness();
}

IseApplication::~IseApplication()
{
    finalize();
}

//-----------------------------------------------------------------------------

IseApplication& IseApplication::instance()
{
    static IseApplication obj;
    return obj;
}

//-----------------------------------------------------------------------------
// ����: ���������в���
// ����:
//   true  - ��������ִ��
//   false - ����Ӧ�˳� (���������в�������ȷ)
//-----------------------------------------------------------------------------
bool IseApplication::parseArguments(int argc, char *argv[])
{
    // �ȼ�¼�����в���
    argList_.clear();
    for (int i = 1; i < argc; i++)
        argList_.add(argv[i]);

    // �����׼�����в���
    if (processStandardArgs(false)) return false;

    // ���� IseBusiness �������
    return iseBusiness_->parseArguments(argc, argv);
}

//-----------------------------------------------------------------------------
// ����: Ӧ�ó����ʼ�� (����ʼ��ʧ�����׳��쳣)
//-----------------------------------------------------------------------------
void IseApplication::initialize()
{
    try
    {
#ifdef ISE_LINUX
        // �ڳ�ʼ���׶�Ҫ�����˳��ź�
        SignalMasker sigMasker(true);
        sigMasker.setSignals(1, SIGTERM);
        sigMasker.block();
#endif

        networkInitialize();
        initExeName();
        iseBusiness_->beforeInit();
        iseBusiness_->initIseOptions(iseOptions_);
        processStandardArgs(true);
        checkMultiInstance();
        if (iseOptions_.getIsDaemon()) initDaemon();
        initSignals();
        initNewOperHandler();
        applyIseOptions();
        createMainServer();
        iseBusiness_->initialize();
        mainServer_->initialize();
        iseBusiness_->afterInit();
        if (iseOptions_.getIsDaemon()) closeTerminal();
        initialized_ = true;
    }
    catch (Exception& e)
    {
        openTerminal();
        iseBusiness_->onInitFailed(e);
        doFinalize();
        throw;
    }
}

//-----------------------------------------------------------------------------
// ����: Ӧ�ó��������
//-----------------------------------------------------------------------------
void IseApplication::finalize()
{
    if (initialized_)
    {
        openTerminal();
        doFinalize();
        initialized_ = false;
    }
}

//-----------------------------------------------------------------------------
// ����: ��ʼ����Ӧ�ó���
//-----------------------------------------------------------------------------
void IseApplication::run()
{
#ifdef ISE_LINUX
    // ���̱���ֹʱ����ת���˴�����������
    if (sigsetjmp(procExitJmpBuf, 0)) return;
#endif

    if (mainServer_)
        mainServer_->run();
}

//-----------------------------------------------------------------------------
// ����: ȡ�ÿ�ִ���ļ����ڵ�·��
//-----------------------------------------------------------------------------
string IseApplication::getExePath()
{
    return extractFilePath(exeName_);
}

//-----------------------------------------------------------------------------
// ����: ȡ�������в����ַ��� (index: 0-based)
//-----------------------------------------------------------------------------
string IseApplication::getArgString(int index)
{
    if (index >= 0 && index < (int)argList_.getCount())
        return argList_[index];
    else
        return "";
}

//-----------------------------------------------------------------------------
// ����: ע���û��źŴ�����
//-----------------------------------------------------------------------------
void IseApplication::registerUserSignalHandler(const UserSignalHandlerCallback& callback)
{
    onUserSignal_.registerCallback(callback);
}

//-----------------------------------------------------------------------------
// ����: �����׼�����в���
// ����:
//   isInitializing - Ӧ�ó��� (IseApplication) �Ƿ��ڳ�ʼ���ڼ�
// ����:
//   true  - ��ǰ�����в����Ǳ�׼����
//   false - �����෴
//-----------------------------------------------------------------------------
bool IseApplication::processStandardArgs(bool isInitializing)
{
    if (!isInitializing)
    {
        if (getArgCount() == 1)
        {
            string arg = getArgString(0);
            if (arg == "--version")
            {
                string version = iseBusiness_->getAppVersion();
                printf("%s\n", version.c_str());
                return true;
            }
            if (arg == "--help")
            {
                string help = iseBusiness_->getAppHelp();
                printf("%s\n", help.c_str());
                return true;
            }
        }
    }
    else
    {
        if (argList_.exists("--nodaemon"))
        {
            iseOptions_.setIsDaemon(false);
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
// ����: ����Ƿ������˶������ʵ��
//-----------------------------------------------------------------------------
void IseApplication::checkMultiInstance()
{
    if (iseOptions_.getAllowMultiInstance()) return;

#ifdef ISE_WINDOWS
    CreateMutexA(NULL, false, getExeName().c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        iseThrowException(SEM_ALREADY_RUNNING);
#endif
#ifdef ISE_LINUX
    umask(0);
    int fd = open(getExeName().c_str(), O_RDONLY, 0666);
    if (fd >= 0 && flock(fd, LOCK_EX | LOCK_NB) != 0)
        iseThrowException(SEM_ALREADY_RUNNING);
#endif
}

//-----------------------------------------------------------------------------
// ����: Ӧ�� ISE ����
//-----------------------------------------------------------------------------
void IseApplication::applyIseOptions()
{
    logger().setFileName(iseOptions_.getLogFileName(), iseOptions_.getLogNewFileDaily());
}

//-----------------------------------------------------------------------------
// ����: ������������
//-----------------------------------------------------------------------------
void IseApplication::createMainServer()
{
    if (!mainServer_)
        mainServer_ = new IseMainServer();
}

//-----------------------------------------------------------------------------
// ����: �ͷ���������
//-----------------------------------------------------------------------------
void IseApplication::deleteMainServer()
{
    delete mainServer_;
    mainServer_ = NULL;
}

//-----------------------------------------------------------------------------
// ����: ���� IseBusiness ����
//-----------------------------------------------------------------------------
void IseApplication::createIseBusiness()
{
    if (!iseBusiness_)
        iseBusiness_ = createIseBusinessObject();

    if (!iseBusiness_)
        iseThrowException(SEM_BUSINESS_OBJ_EXPECTED);
}

//-----------------------------------------------------------------------------
// ����: �ͷ� IseBusiness ����
//-----------------------------------------------------------------------------
void IseApplication::deleteIseBusiness()
{
    delete iseBusiness_;
    iseBusiness_ = NULL;
}

//-----------------------------------------------------------------------------
// ����: ȡ�������ļ���ȫ��������ʼ�� exeName_
//-----------------------------------------------------------------------------
void IseApplication::initExeName()
{
    exeName_ = getAppExeName();
}

//-----------------------------------------------------------------------------
// ����: �ػ�ģʽ��ʼ��
//-----------------------------------------------------------------------------
void IseApplication::initDaemon()
{
#ifdef ISE_WINDOWS
#endif
#ifdef ISE_LINUX
    int r;

    r = fork();
    if (r < 0)
        iseThrowException(SEM_INIT_DAEMON_ERROR);
    else if (r != 0)
        exit(0);

    // ��һ�ӽ��̺�̨����ִ��

    // ��һ�ӽ��̳�Ϊ�µĻỰ�鳤�ͽ����鳤
    r = setsid();
    if (r < 0) exit(1);

    // ���� SIGHUP �ź� (ע: �������ն˶Ͽ�ʱ�������������������ر�telnet)
    signal(SIGHUP, SIG_IGN);

    // ��һ�ӽ����˳�
    r = fork();
    if (r < 0) exit(1);
    else if (r != 0) exit(0);

    // �ڶ��ӽ��̼���ִ�У��������ǻỰ�鳤

    // �ı䵱ǰ����Ŀ¼ (core dump ���������Ŀ¼��)
    // chdir("/");

    // �����ļ�������ģ
    umask(0);
#endif
}

//-----------------------------------------------------------------------------
// ����: ��ʼ���ź� (�źŵİ�װ�����Ե�)
//
//  �ź�����    ֵ                         �ź�˵��
// ---------  ----  -----------------------------------------------------------
// # SIGHUP    1    ���ź����û��ն�����(�����������)����ʱ������ͨ�������ն˵Ŀ��ƽ���
//                  ����ʱ��֪ͨͬһ session �ڵĸ�����ҵ����ʱ����������ն˲��ٹ�����
// # SIGINT    2    ������ֹ(interrupt)�źţ����û�����INTR�ַ�(ͨ����Ctrl-C)ʱ������
// # SIGQUIT   3    �� SIGINT ���ƣ�����QUIT�ַ� (ͨ���� Ctrl-\) �����ơ����������յ�
//                  ���ź��˳�ʱ�����core�ļ��������������������һ����������źš�
// # SIGILL    4    ִ���˷Ƿ�ָ�ͨ������Ϊ��ִ���ļ�������ִ��󣬻�����ͼִ�����ݶΡ�
//                  ��ջ���ʱҲ�п��ܲ�������źš�
// # SIGTRAP   5    �ɶϵ�ָ������� trap ָ��������� debugger ʹ�á�
// # SIGABRT   6    �����Լ����ִ��󲢵��� abort ʱ������
// # SIGIOT    6    ��PDP-11����iotָ������������������Ϻ� SIGABRT һ����
// # SIGBUS    7    �Ƿ���ַ�������ڴ��ַ����(alignment)����eg: ����һ���ĸ��ֳ���
//                  �����������ַ���� 4 �ı�����
// # SIGFPE    8    �ڷ��������������������ʱ������������������������󣬻������������
//                  ��Ϊ 0 ���������е������Ĵ���
// # SIGKILL   9    ��������������������С����źŲ��ܱ�����������ͺ��ԡ�
// # SIGUSR1   10   �����û�ʹ�á�
// # SIGSEGV   11   ��ͼ����δ������Լ����ڴ棬����ͼ��û��дȨ�޵��ڴ��ַд���ݡ�
// # SIGUSR2   12   �����û�ʹ�á�
// # SIGPIPE   13   �ܵ�����(broken pipe)��дһ��û�ж��˿ڵĹܵ���
// # SIGALRM   14   ʱ�Ӷ�ʱ�źţ��������ʵ�ʵ�ʱ���ʱ��ʱ�䡣alarm ����ʹ�ø��źš�
// # SIGTERM   15   �������(terminate)�źţ��� SIGKILL ��ͬ���Ǹ��źſ��Ա������ʹ���
//                  ͨ������Ҫ������Լ������˳���shell ���� kill ȱʡ��������źš�
// # SIGSTKFLT 16   Э��������ջ����(stack fault)��
// # SIGCHLD   17   �ӽ��̽���ʱ�������̻��յ�����źš�
// # SIGCONT   18   ��һ��ֹͣ(stopped)�Ľ��̼���ִ�С����źŲ��ܱ�������������һ��
//                  handler ���ó������� stopped ״̬��Ϊ����ִ��ʱ����ض��Ĺ���������
//                  ������ʾ��ʾ����
// # SIGSTOP   19   ֹͣ(stopped)���̵�ִ�С�ע������terminate�Լ�interrupt������:
//                  �ý��̻�δ������ֻ����ִͣ�С����źŲ��ܱ��������������ԡ�
// # SIGTSTP   20   ֹͣ���̵����У������źſ��Ա�����ͺ��ԡ��û�����SUSP�ַ�ʱ(ͨ����^Z)
//                  ��������źš�
// # SIGTTIN   21   ����̨��ҵҪ���û��ն˶�����ʱ������ҵ�е����н��̻��յ����źš�ȱʡʱ
//                  ��Щ���̻�ִֹͣ�С�
// # SIGTTOU   22   ������SIGTTIN������д�ն�(���޸��ն�ģʽ)ʱ�յ���
// # SIGURG    23   �� "����" ���ݻ����(out-of-band) ���ݵ��� socket ʱ������
// # SIGXCPU   24   ����CPUʱ����Դ���ơ�������ƿ�����getrlimit/setrlimit����ȡ�͸ı䡣
// # SIGXFSZ   25   �����ļ���С��Դ���ơ�
// # SIGVTALRM 26   ����ʱ���źš������� SIGALRM�����Ǽ�����Ǹý���ռ�õ�CPUʱ�䡣
// # SIGPROF   27   ������SIGALRM/SIGVTALRM���������ý����õ�CPUʱ���Լ�ϵͳ���õ�ʱ�䡣
// # SIGWINCH  28   �ն��Ӵ��ĸı�ʱ������
// # SIGIO     29   �ļ�������׼�����������Կ�ʼ��������/���������
// # SIGPWR    30   Power failure.
// # SIGSYS    31   �Ƿ���ϵͳ���á�
//-----------------------------------------------------------------------------
void IseApplication::initSignals()
{
#ifdef ISE_WINDOWS
#endif
#ifdef ISE_LINUX
    UINT i;

    // ����ĳЩ�ź�
    int ignoreSignals[] = {SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGTSTP, SIGTTIN,
        SIGTTOU, SIGXCPU, SIGCHLD, SIGPWR, SIGALRM, SIGVTALRM, SIGIO};
    for (i = 0; i < sizeof(ignoreSignals)/sizeof(int); i++)
        signal(ignoreSignals[i], SIG_IGN);

    // ��װ�����Ƿ������źŴ�����
    int fatalSignals[] = {SIGILL, SIGBUS, SIGFPE, SIGSEGV, SIGSTKFLT, SIGPROF, SIGSYS};
    for (i = 0; i < sizeof(fatalSignals)/sizeof(int); i++)
        signal(fatalSignals[i], fatalErrorSignalHandler);

    // ��װ�����˳��źŴ�����
    int exitSignals[] = {SIGTERM/*, SIGABRT*/};
    for (i = 0; i < sizeof(exitSignals)/sizeof(int); i++)
        signal(exitSignals[i], exitProgramSignalHandler);

    // ��װ�û��źŴ�����
    int userSignals[] = {SIGUSR1, SIGUSR2};
    for (i = 0; i < sizeof(userSignals)/sizeof(int); i++)
        signal(userSignals[i], userSignalHandler);
#endif
}

//-----------------------------------------------------------------------------
// ����: ��ʼ�� new �������Ĵ�������
//-----------------------------------------------------------------------------
void IseApplication::initNewOperHandler()
{
    const int RESERVED_MEM_SIZE = 1024*1024*2;     // 2M

    std::set_new_handler(outOfMemoryHandler);

    // �����ڴ治�������³����˳�
    reservedMemoryForExit = new char[RESERVED_MEM_SIZE];
}

//-----------------------------------------------------------------------------

#ifdef ISE_LINUX
static int s_oldStdInFd = -1;
static int s_oldStdOutFd = -1;
static int s_oldStdErrFd = -1;
#endif

//-----------------------------------------------------------------------------
// ����: ���ն�
//-----------------------------------------------------------------------------
void IseApplication::openTerminal()
{
#ifdef ISE_LINUX
    if (s_oldStdInFd != -1)
        dup2(s_oldStdInFd, 0);

    if (s_oldStdOutFd != -1)
        dup2(s_oldStdOutFd, 1);

    if (s_oldStdErrFd != -1)
        dup2(s_oldStdErrFd, 2);
#endif
}

//-----------------------------------------------------------------------------
// ����: �ر��ն�
//-----------------------------------------------------------------------------
void IseApplication::closeTerminal()
{
#ifdef ISE_LINUX
    s_oldStdInFd = dup(0);
    s_oldStdOutFd = dup(1);
    s_oldStdErrFd = dup(2);

    int fd = open("/dev/null", O_RDWR);
    if (fd != -1)
    {
        dup2(fd, 0);  // stdin
        dup2(fd, 1);  // stdout
        dup2(fd, 2);  // stderr

        if (fd > 2)
            close(fd);
    }
#endif
}

//-----------------------------------------------------------------------------
// ����: Ӧ�ó�������� (����� initialized_ ��־)
//-----------------------------------------------------------------------------
void IseApplication::doFinalize()
{
    try { if (mainServer_) mainServer_->finalize(); } catch (...) {}
    try { iseBusiness_->finalize(); } catch (...) {}
    try { deleteMainServer(); } catch (...) {}
    try { deleteIseBusiness(); } catch (...) {}
    try { networkFinalize(); } catch (...) {}
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise
