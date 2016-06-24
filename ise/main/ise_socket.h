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
// ise_socket.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_SOCKET_H_
#define _ISE_SOCKET_H_

#include "ise/main/ise_options.h"

#ifdef ISE_WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#endif

#ifdef ISE_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <string>
#endif

#include "ise/main/ise_classes.h"
#include "ise/main/ise_thread.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ��ǰ����

class InetAddress;
class Socket;
class UdpSocket;
class BaseUdpClient;
class BaseUdpServer;
class TcpSocket;
class BaseTcpConnection;
class BaseTcpClient;
class BaseTcpServer;
class ListenerThread;
class UdpListenerThread;
class UdpListenerThreadPool;
class TcpListenerThread;

///////////////////////////////////////////////////////////////////////////////
// ��������

#ifdef ISE_WINDOWS
const int SS_SD_RECV            = 0;
const int SS_SD_SEND            = 1;
const int SS_SD_BOTH            = 2;

const int SS_EINTR              = WSAEINTR;
const int SS_EBADF              = WSAEBADF;
const int SS_EACCES             = WSAEACCES;
const int SS_EFAULT             = WSAEFAULT;
const int SS_EINVAL             = WSAEINVAL;
const int SS_EMFILE             = WSAEMFILE;
const int SS_EWOULDBLOCK        = WSAEWOULDBLOCK;
const int SS_EINPROGRESS        = WSAEINPROGRESS;
const int SS_EALREADY           = WSAEALREADY;
const int SS_ENOTSOCK           = WSAENOTSOCK;
const int SS_EDESTADDRREQ       = WSAEDESTADDRREQ;
const int SS_EMSGSIZE           = WSAEMSGSIZE;
const int SS_EPROTOTYPE         = WSAEPROTOTYPE;
const int SS_ENOPROTOOPT        = WSAENOPROTOOPT;
const int SS_EPROTONOSUPPORT    = WSAEPROTONOSUPPORT;
const int SS_ESOCKTNOSUPPORT    = WSAESOCKTNOSUPPORT;
const int SS_EOPNOTSUPP         = WSAEOPNOTSUPP;
const int SS_EPFNOSUPPORT       = WSAEPFNOSUPPORT;
const int SS_EAFNOSUPPORT       = WSAEAFNOSUPPORT;
const int SS_EADDRINUSE         = WSAEADDRINUSE;
const int SS_EADDRNOTAVAIL      = WSAEADDRNOTAVAIL;
const int SS_ENETDOWN           = WSAENETDOWN;
const int SS_ENETUNREACH        = WSAENETUNREACH;
const int SS_ENETRESET          = WSAENETRESET;
const int SS_ECONNABORTED       = WSAECONNABORTED;
const int SS_ECONNRESET         = WSAECONNRESET;
const int SS_ENOBUFS            = WSAENOBUFS;
const int SS_EISCONN            = WSAEISCONN;
const int SS_ENOTCONN           = WSAENOTCONN;
const int SS_ESHUTDOWN          = WSAESHUTDOWN;
const int SS_ETOOMANYREFS       = WSAETOOMANYREFS;
const int SS_ETIMEDOUT          = WSAETIMEDOUT;
const int SS_ECONNREFUSED       = WSAECONNREFUSED;
const int SS_ELOOP              = WSAELOOP;
const int SS_ENAMETOOLONG       = WSAENAMETOOLONG;
const int SS_EHOSTDOWN          = WSAEHOSTDOWN;
const int SS_EHOSTUNREACH       = WSAEHOSTUNREACH;
const int SS_ENOTEMPTY          = WSAENOTEMPTY;
#endif

#ifdef ISE_LINUX
const int SS_SD_RECV            = SHUT_RD;
const int SS_SD_SEND            = SHUT_WR;
const int SS_SD_BOTH            = SHUT_RDWR;

const int SS_EINTR              = EINTR;
const int SS_EBADF              = EBADF;
const int SS_EACCES             = EACCES;
const int SS_EFAULT             = EFAULT;
const int SS_EINVAL             = EINVAL;
const int SS_EMFILE             = EMFILE;
const int SS_EWOULDBLOCK        = EWOULDBLOCK;
const int SS_EINPROGRESS        = EINPROGRESS;
const int SS_EALREADY           = EALREADY;
const int SS_ENOTSOCK           = ENOTSOCK;
const int SS_EDESTADDRREQ       = EDESTADDRREQ;
const int SS_EMSGSIZE           = EMSGSIZE;
const int SS_EPROTOTYPE         = EPROTOTYPE;
const int SS_ENOPROTOOPT        = ENOPROTOOPT;
const int SS_EPROTONOSUPPORT    = EPROTONOSUPPORT;
const int SS_ESOCKTNOSUPPORT    = ESOCKTNOSUPPORT;

const int SS_EOPNOTSUPP         = EOPNOTSUPP;
const int SS_EPFNOSUPPORT       = EPFNOSUPPORT;
const int SS_EAFNOSUPPORT       = EAFNOSUPPORT;
const int SS_EADDRINUSE         = EADDRINUSE;
const int SS_EADDRNOTAVAIL      = EADDRNOTAVAIL;
const int SS_ENETDOWN           = ENETDOWN;
const int SS_ENETUNREACH        = ENETUNREACH;
const int SS_ENETRESET          = ENETRESET;
const int SS_ECONNABORTED       = ECONNABORTED;
const int SS_ECONNRESET         = ECONNRESET;
const int SS_ENOBUFS            = ENOBUFS;
const int SS_EISCONN            = EISCONN;
const int SS_ENOTCONN           = ENOTCONN;
const int SS_ESHUTDOWN          = ESHUTDOWN;
const int SS_ETOOMANYREFS       = ETOOMANYREFS;
const int SS_ETIMEDOUT          = ETIMEDOUT;
const int SS_ECONNREFUSED       = ECONNREFUSED;
const int SS_ELOOP              = ELOOP;
const int SS_ENAMETOOLONG       = ENAMETOOLONG;
const int SS_EHOSTDOWN          = EHOSTDOWN;
const int SS_EHOSTUNREACH       = EHOSTUNREACH;
const int SS_ENOTEMPTY          = ENOTEMPTY;
#endif

///////////////////////////////////////////////////////////////////////////////
// ������Ϣ (ISE Socket Error Message)

const char* const SSEM_ERROR             = "Socket Error #%d: %s";
const char* const SSEM_SOCKETERROR       = "Socket error";
const char* const SSEM_TCPSENDTIMEOUT    = "TCP send timeout";
const char* const SSEM_TCPRECVTIMEOUT    = "TCP recv timeout";

const char* const SSEM_EINTR             = "Interrupted system call.";
const char* const SSEM_EBADF             = "Bad file number.";
const char* const SSEM_EACCES            = "Access denied.";
const char* const SSEM_EFAULT            = "Buffer fault.";
const char* const SSEM_EINVAL            = "Invalid argument.";
const char* const SSEM_EMFILE            = "Too many open files.";
const char* const SSEM_EWOULDBLOCK       = "Operation would block.";
const char* const SSEM_EINPROGRESS       = "Operation now in progress.";
const char* const SSEM_EALREADY          = "Operation already in progress.";
const char* const SSEM_ENOTSOCK          = "Socket operation on non-socket.";
const char* const SSEM_EDESTADDRREQ      = "Destination address required.";
const char* const SSEM_EMSGSIZE          = "Message too long.";
const char* const SSEM_EPROTOTYPE        = "Protocol wrong type for socket.";
const char* const SSEM_ENOPROTOOPT       = "Bad protocol option.";
const char* const SSEM_EPROTONOSUPPORT   = "Protocol not supported.";
const char* const SSEM_ESOCKTNOSUPPORT   = "Socket type not supported.";
const char* const SSEM_EOPNOTSUPP        = "Operation not supported on socket.";
const char* const SSEM_EPFNOSUPPORT      = "Protocol family not supported.";
const char* const SSEM_EAFNOSUPPORT      = "Address family not supported by protocol family.";
const char* const SSEM_EADDRINUSE        = "Address already in use.";
const char* const SSEM_EADDRNOTAVAIL     = "Cannot assign requested address.";
const char* const SSEM_ENETDOWN          = "Network is down.";
const char* const SSEM_ENETUNREACH       = "Network is unreachable.";
const char* const SSEM_ENETRESET         = "Net dropped connection or reset.";
const char* const SSEM_ECONNABORTED      = "Software caused connection abort.";
const char* const SSEM_ECONNRESET        = "Connection reset by peer.";
const char* const SSEM_ENOBUFS           = "No buffer space available.";
const char* const SSEM_EISCONN           = "Socket is already connected.";
const char* const SSEM_ENOTCONN          = "Socket is not connected.";
const char* const SSEM_ESHUTDOWN         = "Cannot send or receive after socket is closed.";
const char* const SSEM_ETOOMANYREFS      = "Too many references, cannot splice.";
const char* const SSEM_ETIMEDOUT         = "Connection timed out.";
const char* const SSEM_ECONNREFUSED      = "Connection refused.";
const char* const SSEM_ELOOP             = "Too many levels of symbolic links.";
const char* const SSEM_ENAMETOOLONG      = "File name too long.";
const char* const SSEM_EHOSTDOWN         = "Host is down.";
const char* const SSEM_EHOSTUNREACH      = "No route to host.";
const char* const SSEM_ENOTEMPTY         = "Directory not empty";

///////////////////////////////////////////////////////////////////////////////
// ���Ͷ���

#ifdef ISE_WINDOWS
typedef int socklen_t;
#endif
#ifdef ISE_LINUX
typedef int SOCKET;
#define INVALID_SOCKET ((SOCKET)(-1))
#endif

typedef struct sockaddr_in SockAddr;

// ����Э������(UDP|TCP)
enum NET_PROTO_TYPE
{
    NPT_UDP,        // UDP
    NPT_TCP         // TCP
};

// DTP Э������(TCP|UTP)
enum DTP_PROTO_TYPE
{
    DPT_TCP,        // TCP
    DPT_UTP         // UTP (UDP Transfer Protocol)
};

// �첽���ӵ�״̬
enum ASYNC_CONNECT_STATE
{
    ACS_NONE,       // ��δ��������
    ACS_CONNECTING, // ��δ������ϣ�����δ��������
    ACS_CONNECTED,  // �����ѽ����ɹ�
    ACS_FAILED      // ���ӹ����з����˴��󣬵�������ʧ��
};

///////////////////////////////////////////////////////////////////////////////
// �����

// �����ʼ��/������
void networkInitialize();
void networkFinalize();
bool isNetworkInited();
void ensureNetworkInited();

// �����ƽ̨����ĺ���
int iseSocketGetLastError();
string iseSocketGetErrorMsg(int errorCode);
string iseSocketGetLastErrMsg();
void iseCloseSocket(SOCKET handle);

// �����
string ipToString(UINT ip);
UINT stringToIp(const string& str);
InetAddress getSocketLocalAddr(SOCKET handle);
InetAddress getSocketPeerAddr(SOCKET handle);
int getFreePort(NET_PROTO_TYPE proto, int startPort, int checkTimes);
void getLocalIpList(StrList& ipList);
string getLocalIp();
string lookupHostAddr(const string& host);
void iseThrowSocketLastError();

///////////////////////////////////////////////////////////////////////////////
// class InetAddress - IPv4��ַ��

#pragma pack(1)     // 1�ֽڶ���

// ��ַ��Ϣ
class InetAddress
{
public:
    UINT ip;        // IP   (�����ֽ�˳��)
    WORD port;      // �˿� (�����ֽ�˳��)
public:
    InetAddress() : ip(0), port(0) {}
    InetAddress(UINT _ip, WORD _port) : ip(_ip), port(_port) {}
    InetAddress(const string& _ip, WORD _port)
    {
        ip = stringToIp(_ip);
        port = _port;
    }

    InetAddress(const SockAddr& sockAddr)
    {
        ip = ntohl(sockAddr.sin_addr.s_addr);
        port = ntohs(sockAddr.sin_port);
    }

    bool operator == (const InetAddress& rhs) const
        { return (ip == rhs.ip && port == rhs.port); }
    bool operator != (const InetAddress& rhs) const
        { return !((*this) == rhs); }

    SockAddr getSockAddr() const
    {
        SockAddr result;
        memset(&result, 0, sizeof(result));
        result.sin_family = AF_INET;
        result.sin_addr.s_addr = htonl(ip);
        result.sin_port = htons(port);
        return result;
    }

    void clear() { ip = 0; port = 0; }
    bool isEmpty() const { return (ip == 0) && (port == 0); }
    string getDisplayStr() const;
};

#pragma pack()

///////////////////////////////////////////////////////////////////////////////
// class Socket - �׽�����

class Socket : boost::noncopyable
{
public:
    friend class BaseTcpServer;

public:
    Socket();
    virtual ~Socket();

    virtual void open();
    virtual void close();

    bool isActive() const { return isActive_; }
    SOCKET getHandle() const { return handle_; }
    InetAddress getLocalAddr() const;
    InetAddress getPeerAddr() const;
    bool isBlockMode() const { return isBlockMode_; }
    void setBlockMode(bool value);
    void setHandle(SOCKET value);

protected:
    void setActive(bool value);
    void setDomain(int value);
    void setType(int value);
    void setProtocol(int value);

    void bind(WORD port);

private:
    void doSetBlockMode(SOCKET handle, bool value);
    void doClose();

protected:
    bool isActive_;     // �׽����Ƿ�׼������
    SOCKET handle_;     // �׽��־��
    int domain_;        // �׽��ֵ�Э����� (PF_UNIX, PF_INET, PF_INET6, PF_IPX, ...)
    int type_;          // �׽������ͣ�����ָ�� (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, SOCK_RDM, SOCK_SEQPACKET)
    int protocol_;      // �׽�������Э�飬��Ϊ0 (IPPROTO_IP, IPPROTO_UDP, IPPROTO_TCP, ...)
    bool isBlockMode_;  // �Ƿ�Ϊ����ģʽ (ȱʡΪ����ģʽ)
};

///////////////////////////////////////////////////////////////////////////////
// class UdpSocket - UDP �׽�����

class UdpSocket : public Socket
{
public:
    UdpSocket()
    {
        type_ = SOCK_DGRAM;
        protocol_ = IPPROTO_UDP;
        isBlockMode_ = true;
    }

    int recvBuffer(void *buffer, int size);
    int recvBuffer(void *buffer, int size, InetAddress& peerAddr);
    int sendBuffer(void *buffer, int size, const InetAddress& peerAddr, int sendTimes = 1);

    virtual void open();
};

///////////////////////////////////////////////////////////////////////////////
// class UdpClient - UDP Client ����

class BaseUdpClient : public UdpSocket
{
public:
    BaseUdpClient() { open(); }
};

///////////////////////////////////////////////////////////////////////////////
// class UdpServer - UDP Server ����

class BaseUdpServer :
    public UdpSocket,
    public ObjectContext
{
public:
    friend class UdpListenerThread;

    typedef boost::function<void (void *packetBuffer, int packetSize,
        const InetAddress& peerAddr)> UdpSvrRecvDataCallback;

public:
    BaseUdpServer();
    virtual ~BaseUdpServer();

    virtual void open();
    virtual void close();

    WORD getLocalPort() const { return localPort_; }
    void setLocalPort(WORD value);

    int getListenerThreadCount() const;
    void setListenerThreadCount(int value);

    void setRecvDataCallback(const UdpSvrRecvDataCallback& callback);

protected:
    virtual void startListenerThreads();
    virtual void stopListenerThreads();

private:
    void dataReceived(void *packetBuffer, int packetSize, const InetAddress& peerAddr);

private:
    WORD localPort_;
    UdpListenerThreadPool *listenerThreadPool_;
    UdpSvrRecvDataCallback onRecvData_;
};

///////////////////////////////////////////////////////////////////////////////
// class TcpSocket - TCP �׽�����

class TcpSocket : public Socket
{
public:
    TcpSocket()
    {
        type_ = SOCK_STREAM;
        protocol_ = IPPROTO_TCP;
        isBlockMode_ = false;
    }

    void shutdown(bool closeSend = true, bool closeRecv = true);
};

///////////////////////////////////////////////////////////////////////////////
// class BaseTcpConnection - TCP Connection ����

class BaseTcpConnection :
    boost::noncopyable,
    public ObjectContext
{
public:
    BaseTcpConnection();
    BaseTcpConnection(SOCKET socketHandle);
    virtual ~BaseTcpConnection() {}

    virtual bool isConnected() const;
    void disconnect();
    void shutdown(bool closeSend = true, bool closeRecv = true);

    void setNoDelay(bool value);
    void setKeepAlive(bool value);

    TcpSocket& getSocket() { return socket_; }
    const TcpSocket& getSocket() const { return socket_; }
    const InetAddress& getLocalAddr() const;
    const InetAddress& getPeerAddr() const;

protected:
    virtual void doDisconnect();

protected:
    int sendBuffer(void *buffer, int size, bool syncMode = false, int timeoutMSecs = -1);
    int recvBuffer(void *buffer, int size, bool syncMode = false, int timeoutMSecs = -1);

private:
    int doSyncSendBuffer(void *buffer, int size, int timeoutMSecs = -1);
    int doSyncRecvBuffer(void *buffer, int size, int timeoutMSecs = -1);
    int doAsyncSendBuffer(void *buffer, int size);
    int doAsyncRecvBuffer(void *buffer, int size);

protected:
    TcpSocket socket_;
private:
    bool isDisconnected_;
    mutable InetAddress localAddr_;
    mutable InetAddress peerAddr_;
};

///////////////////////////////////////////////////////////////////////////////
// class BaseTcpClient - TCP Client ����

class BaseTcpClient : boost::noncopyable
{
public:
    BaseTcpClient();
    virtual ~BaseTcpClient();

    // ����ʽ����
    void connect(const string& ip, int port);
    // �첽(������ʽ)���� (���� enum ASYNC_CONNECT_STATE)
    int asyncConnect(const string& ip, int port, int timeoutMSecs = -1);
    // ����첽���ӵ�״̬ (���� enum ASYNC_CONNECT_STATE)
    int checkAsyncConnectState(int timeoutMSecs = -1);

    bool isConnected() { return connection_ && connection_->isConnected(); }
    void disconnect();
    BaseTcpConnection& getConnection();

protected:
    virtual BaseTcpConnection* createConnection() { return new BaseTcpConnection(); }
private:
    void ensureConnCreated();
    TcpSocket& getSocket();
protected:
    BaseTcpConnection *connection_;
};

///////////////////////////////////////////////////////////////////////////////
// class BaseTcpServer - TCP Server ����

class BaseTcpServer :
    boost::noncopyable,
    public ObjectContext
{
public:
    friend class TcpListenerThread;

    typedef boost::function<void (BaseTcpServer *tcpServer, SOCKET socketHandle,
        BaseTcpConnection*& connection)> TcpSvrCreateConnCallback;
    typedef boost::function<void (BaseTcpServer *tcpServer,
        BaseTcpConnection *connection)> TcpSvrAcceptConnCallback;

public:
    enum { LISTEN_QUEUE_SIZE = 30 };   // TCP�������г���

public:
    BaseTcpServer();
    virtual ~BaseTcpServer();

    virtual void open();
    virtual void close();

    bool isActive() const { return socket_.isActive(); }
    void setActive(bool value);

    WORD getLocalPort() const { return localPort_; }
    void setLocalPort(WORD value);

    const TcpSocket& getSocket() const { return socket_; }

    void setCreateConnCallback(const TcpSvrCreateConnCallback& callback);
    void setAcceptConnCallback(const TcpSvrAcceptConnCallback& callback);

protected:
    virtual void startListenerThread();
    virtual void stopListenerThread();

    virtual BaseTcpConnection* createConnection(SOCKET socketHandle);
    virtual void acceptConnection(BaseTcpConnection *connection);

private:
    TcpSocket socket_;
    WORD localPort_;
    TcpListenerThread *listenerThread_;
    TcpSvrCreateConnCallback onCreateConn_;
    TcpSvrAcceptConnCallback onAcceptConn_;
};

///////////////////////////////////////////////////////////////////////////////
// class ListenerThread - �����߳���

class ListenerThread : public Thread
{
public:
    ListenerThread()
    {
#ifdef ISE_WINDOWS
        setPriority(THREAD_PRI_HIGHEST);
#endif
#ifdef ISE_LINUX
        setPolicy(THREAD_POL_RR);
        setPriority(THREAD_PRI_HIGH);
#endif
    }
    virtual ~ListenerThread() {}

protected:
    virtual void execute() {}
};

///////////////////////////////////////////////////////////////////////////////
// class UdpListenerThread - UDP�����������߳���

class UdpListenerThread : public ListenerThread
{
public:
    explicit UdpListenerThread(UdpListenerThreadPool *threadPool, int index);
    virtual ~UdpListenerThread();
protected:
    virtual void execute();
private:
    UdpListenerThreadPool *threadPool_;  // �����̳߳�
    BaseUdpServer *udpServer_;           // ����UDP������
    int index_;                          // �߳��ڳ��е�������(0-based)
};

///////////////////////////////////////////////////////////////////////////////
// class UdpListenerThreadPool - UDP�����������̳߳���

class UdpListenerThreadPool : boost::noncopyable
{
public:
    explicit UdpListenerThreadPool(BaseUdpServer *udpServer);
    virtual ~UdpListenerThreadPool();

    void registerThread(UdpListenerThread *thread);
    void unregisterThread(UdpListenerThread *thread);

    void startThreads();
    void stopThreads();

    int getMaxThreadCount() const { return maxThreadCount_; }
    void setMaxThreadCount(int value) { maxThreadCount_ = value; }

    // ��������UDP������
    BaseUdpServer& getUdpServer() { return *udpServer_; }

private:
    BaseUdpServer *udpServer_;            // ����UDP������
    ThreadList threadList_;               // �߳��б�
    int maxThreadCount_;                  // ��������߳�����
};

///////////////////////////////////////////////////////////////////////////////
// class TcpListenerThread - TCP�����������߳���

class TcpListenerThread : public ListenerThread
{
public:
    explicit TcpListenerThread(BaseTcpServer *tcpServer);
protected:
    virtual void execute();
private:
    BaseTcpServer *tcpServer_;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_SOCKET_H_
