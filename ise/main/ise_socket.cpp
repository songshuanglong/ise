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
// �ļ�����: ise_socket.cpp
// ��������: ����������
///////////////////////////////////////////////////////////////////////////////

#include "ise/main/ise_socket.h"
#include "ise/main/ise_sys_utils.h"
#include "ise/main/ise_exceptions.h"

#ifdef ISE_COMPILER_VC
#pragma comment(lib, "ws2_32.lib")
#endif

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// �����

static int s_networkInitCount = 0;

//-----------------------------------------------------------------------------
// ����: �����ʼ�� (��ʧ�����׳��쳣)
//-----------------------------------------------------------------------------
void networkInitialize()
{
    s_networkInitCount++;
    if (s_networkInitCount > 1) return;

#ifdef ISE_WINDOWS
    WSAData wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        s_networkInitCount--;
        iseThrowSocketLastError();
    }
#endif
}

//-----------------------------------------------------------------------------
// ����: ���������
//-----------------------------------------------------------------------------
void networkFinalize()
{
    if (s_networkInitCount > 0)
        s_networkInitCount--;
    if (s_networkInitCount != 0) return;

#ifdef ISE_WINDOWS
    WSACleanup();
#endif
}

//-----------------------------------------------------------------------------

bool isNetworkInited()
{
    return (s_networkInitCount > 0);
}

//-----------------------------------------------------------------------------

void ensureNetworkInited()
{
    if (!isNetworkInited())
        networkInitialize();
}

//-----------------------------------------------------------------------------
// ����: ȡ�����Ĵ������
//-----------------------------------------------------------------------------
int iseSocketGetLastError()
{
#ifdef ISE_WINDOWS
    return WSAGetLastError();
#endif
#ifdef ISE_LINUX
    return errno;
#endif
}

//-----------------------------------------------------------------------------
// ����: ���ش�����Ϣ
//-----------------------------------------------------------------------------
string iseSocketGetErrorMsg(int errorCode)
{
    string result;
    const char *p = "";

    switch (errorCode)
    {
    case SS_EINTR:              p = SSEM_EINTR;                break;
    case SS_EBADF:              p = SSEM_EBADF;                break;
    case SS_EACCES:             p = SSEM_EACCES;               break;
    case SS_EFAULT:             p = SSEM_EFAULT;               break;
    case SS_EINVAL:             p = SSEM_EINVAL;               break;
    case SS_EMFILE:             p = SSEM_EMFILE;               break;

    case SS_EWOULDBLOCK:        p = SSEM_EWOULDBLOCK;          break;
    case SS_EINPROGRESS:        p = SSEM_EINPROGRESS;          break;
    case SS_EALREADY:           p = SSEM_EALREADY;             break;
    case SS_ENOTSOCK:           p = SSEM_ENOTSOCK;             break;
    case SS_EDESTADDRREQ:       p = SSEM_EDESTADDRREQ;         break;
    case SS_EMSGSIZE:           p = SSEM_EMSGSIZE;             break;
    case SS_EPROTOTYPE:         p = SSEM_EPROTOTYPE;           break;
    case SS_ENOPROTOOPT:        p = SSEM_ENOPROTOOPT;          break;
    case SS_EPROTONOSUPPORT:    p = SSEM_EPROTONOSUPPORT;      break;
    case SS_ESOCKTNOSUPPORT:    p = SSEM_ESOCKTNOSUPPORT;      break;
    case SS_EOPNOTSUPP:         p = SSEM_EOPNOTSUPP;           break;
    case SS_EPFNOSUPPORT:       p = SSEM_EPFNOSUPPORT;         break;
    case SS_EAFNOSUPPORT:       p = SSEM_EAFNOSUPPORT;         break;
    case SS_EADDRINUSE:         p = SSEM_EADDRINUSE;           break;
    case SS_EADDRNOTAVAIL:      p = SSEM_EADDRNOTAVAIL;        break;
    case SS_ENETDOWN:           p = SSEM_ENETDOWN;             break;
    case SS_ENETUNREACH:        p = SSEM_ENETUNREACH;          break;
    case SS_ENETRESET:          p = SSEM_ENETRESET;            break;
    case SS_ECONNABORTED:       p = SSEM_ECONNABORTED;         break;
    case SS_ECONNRESET:         p = SSEM_ECONNRESET;           break;
    case SS_ENOBUFS:            p = SSEM_ENOBUFS;              break;
    case SS_EISCONN:            p = SSEM_EISCONN;              break;
    case SS_ENOTCONN:           p = SSEM_ENOTCONN;             break;
    case SS_ESHUTDOWN:          p = SSEM_ESHUTDOWN;            break;
    case SS_ETOOMANYREFS:       p = SSEM_ETOOMANYREFS;         break;
    case SS_ETIMEDOUT:          p = SSEM_ETIMEDOUT;            break;
    case SS_ECONNREFUSED:       p = SSEM_ECONNREFUSED;         break;
    case SS_ELOOP:              p = SSEM_ELOOP;                break;
    case SS_ENAMETOOLONG:       p = SSEM_ENAMETOOLONG;         break;
    case SS_EHOSTDOWN:          p = SSEM_EHOSTDOWN;            break;
    case SS_EHOSTUNREACH:       p = SSEM_EHOSTUNREACH;         break;
    case SS_ENOTEMPTY:          p = SSEM_ENOTEMPTY;            break;
    }

    result = formatString(SSEM_ERROR, errorCode, p);
    return result;
}

//-----------------------------------------------------------------------------
// ����: ȡ��������Ķ�Ӧ��Ϣ
//-----------------------------------------------------------------------------
string iseSocketGetLastErrMsg()
{
    return iseSocketGetErrorMsg(iseSocketGetLastError());
}

//-----------------------------------------------------------------------------
// ����: �ر��׽���
//-----------------------------------------------------------------------------
void iseCloseSocket(SOCKET handle)
{
#ifdef ISE_WINDOWS
    closesocket(handle);
#endif
#ifdef ISE_LINUX
    close(handle);
#endif
}

//-----------------------------------------------------------------------------
// ����: ����IP(�����ֽ�˳��) -> ����IP
//-----------------------------------------------------------------------------
string ipToString(UINT ip)
{
#pragma pack(1)
    union CIpUnion
    {
        UINT value;
        struct
        {
            unsigned char ch1;  // value������ֽ�
            unsigned char ch2;
            unsigned char ch3;
            unsigned char ch4;
        } Bytes;
    } IpUnion;
#pragma pack()
    char str[64];

    IpUnion.value = ip;
    sprintf(str, "%u.%u.%u.%u", IpUnion.Bytes.ch4, IpUnion.Bytes.ch3,
        IpUnion.Bytes.ch2, IpUnion.Bytes.ch1);
    return &str[0];
}

//-----------------------------------------------------------------------------
// ����: ����IP -> ����IP(�����ֽ�˳��)
//-----------------------------------------------------------------------------
UINT stringToIp(const string& str)
{
#pragma pack(1)
    union CIpUnion
    {
        UINT value;
        struct
        {
            unsigned char ch1;
            unsigned char ch2;
            unsigned char ch3;
            unsigned char ch4;
        } Bytes;
    } ipUnion;
#pragma pack()
    IntegerArray intList;

    splitStringToInt(str, '.', intList);
    if (intList.size() == 4)
    {
        ipUnion.Bytes.ch1 = static_cast<unsigned char>(intList[3]);
        ipUnion.Bytes.ch2 = static_cast<unsigned char>(intList[2]);
        ipUnion.Bytes.ch3 = static_cast<unsigned char>(intList[1]);
        ipUnion.Bytes.ch4 = static_cast<unsigned char>(intList[0]);
        return ipUnion.value;
    }
    else
        return 0;
}

//-----------------------------------------------------------------------------
// ����: ȡ���׽��ֵġ����ص�ַ��
//-----------------------------------------------------------------------------
InetAddress getSocketLocalAddr(SOCKET handle)
{
    SockAddr localAddr;

    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t addrLen = sizeof(localAddr);
    if (::getsockname(handle, (struct sockaddr*)&localAddr, &addrLen) < 0)
        memset(&localAddr, 0, sizeof(localAddr));

    return InetAddress(localAddr);
}

//-----------------------------------------------------------------------------
// ����: ȡ���׽��ֵġ��Զ˵�ַ��
//-----------------------------------------------------------------------------
InetAddress getSocketPeerAddr(SOCKET handle)
{
    SockAddr peerAddr;

    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t addrLen = sizeof(peerAddr);
    if (::getpeername(handle, (struct sockaddr*)&peerAddr, &addrLen) < 0)
        memset(&peerAddr, 0, sizeof(peerAddr));

    return InetAddress(peerAddr);
}

//-----------------------------------------------------------------------------
// ����: ȡ�ÿ��ж˿ں�
// ����:
//   proto      - ����Э��(UDP,TCP)
//   startPort  - ��ʼ�˿ں�
//   checkTimes - ������
// ����:
//   ���ж˿ں� (��ʧ���򷵻� 0)
//-----------------------------------------------------------------------------
int getFreePort(NET_PROTO_TYPE proto, int startPort, int checkTimes)
{
    int i, result = 0;
    bool success;
    SockAddr addr;

    ise::networkInitialize();
    AutoFinalizer finalizer(boost::bind(&ise::networkFinalize));

    SOCKET s = socket(PF_INET, (proto == NPT_UDP? SOCK_DGRAM : SOCK_STREAM), IPPROTO_IP);
    if (s == INVALID_SOCKET) return 0;

    success = false;
    for (i = 0; i < checkTimes; i++)
    {
        result = startPort + i;
        addr = InetAddress(ntohl(INADDR_ANY), static_cast<WORD>(result)).getSockAddr();
        if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) != -1)
        {
            success = true;
            break;
        }
    }

    iseCloseSocket(s);
    if (!success) result = 0;
    return result;
}

//-----------------------------------------------------------------------------
// ����: ȡ�ñ���IP�б�
//-----------------------------------------------------------------------------
void getLocalIpList(StrList& ipList)
{
#ifdef ISE_WINDOWS
    char hostName[250];
    hostent *hostEnt;
    in_addr **addrPtr;

    ipList.clear();
    gethostname(hostName, sizeof(hostName));
    hostEnt = gethostbyname(hostName);
    if (hostEnt)
    {
        addrPtr = (in_addr**)(hostEnt->h_addr_list);
        int i = 0;
        while (addrPtr[i])
        {
            UINT ip = ntohl( *(UINT*)(addrPtr[i]) );
            ipList.add(ipToString(ip));
            i++;
        }
    }
#endif
#ifdef ISE_LINUX
    const int MAX_INTERFACES = 16;
    int fd, intfCount;
    struct ifreq buf[MAX_INTERFACES];
    struct ifconf ifc;

    ipList.clear();
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc))
        {
            intfCount = ifc.ifc_len / sizeof(struct ifreq);
            for (int i = 0; i < intfCount; i++)
            {
                ioctl(fd, SIOCGIFADDR, (char*)&buf[i]);
                UINT nIp = ((struct sockaddr_in*)(&buf[i].ifr_addr))->sin_addr.s_addr;
                ipList.add(ipToString(ntohl(nIp)));
            }
        }
        close(fd);
    }
#endif
}

//-----------------------------------------------------------------------------
// ����: ȡ�ñ���IP
//-----------------------------------------------------------------------------
string getLocalIp()
{
    StrList ipList;
    string result;

    getLocalIpList(ipList);
    if (!ipList.isEmpty())
    {
        if (ipList.getCount() == 1)
            result = ipList[0];
        else
        {
            for (int i = 0; i < ipList.getCount(); i++)
            {
                if (ipList[i] != "127.0.0.1")
                {
                    result = ipList[i];
                    break;
                }
            }

            if (result.length() == 0)
                result = ipList[0];
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: ������ַ -> IP��ַ
// ��ע: ��ʧ�ܣ��򷵻ؿ��ַ�����
//-----------------------------------------------------------------------------
string lookupHostAddr(const string& host)
{
    string result = "";

    struct hostent* hostentPtr = gethostbyname(host.c_str());
    if (hostentPtr != NULL)
        result = ipToString(ntohl(((struct in_addr *)hostentPtr->h_addr)->s_addr));

    return result;
}

//-----------------------------------------------------------------------------
// ����: ȡ���Ĵ����벢�׳��쳣
//-----------------------------------------------------------------------------
void iseThrowSocketLastError()
{
    iseThrowSocketException(iseSocketGetLastErrMsg().c_str());
}

///////////////////////////////////////////////////////////////////////////////
// class InetAddress

string InetAddress::getDisplayStr() const
{
    return formatString("%s:%u", ipToString(ip).c_str(), port);
}

///////////////////////////////////////////////////////////////////////////////
// class Socket

Socket::Socket() :
    isActive_(false),
    handle_(INVALID_SOCKET),
    domain_(PF_INET),
    type_(SOCK_STREAM),
    protocol_(IPPROTO_IP),
    isBlockMode_(true)
{
    // nothing
}

//-----------------------------------------------------------------------------

Socket::~Socket()
{
    close();
}

//-----------------------------------------------------------------------------
// ����: ���׽���
//-----------------------------------------------------------------------------
void Socket::open()
{
    if (!isActive_)
    {
        try
        {
            SOCKET handle;
            handle = socket(domain_, type_, protocol_);
            if (handle == INVALID_SOCKET)
                iseThrowSocketLastError();
            isActive_ = (handle != INVALID_SOCKET);
            if (isActive_)
            {
                handle_ = handle;
                setBlockMode(isBlockMode_);
            }
        }
        catch (SocketException&)
        {
            doClose();
            throw;
        }
    }
}

//-----------------------------------------------------------------------------
// ����: �ر��׽���
//-----------------------------------------------------------------------------
void Socket::close()
{
    if (isActive_) doClose();
}

//-----------------------------------------------------------------------------

InetAddress Socket::getLocalAddr() const
{
    return getSocketLocalAddr(handle_);
}

//-----------------------------------------------------------------------------

InetAddress Socket::getPeerAddr() const
{
    return getSocketPeerAddr(handle_);
}

//-----------------------------------------------------------------------------

void Socket::setBlockMode(bool value)
{
    // �˴���Ӧ�� value != isBlockMode_ ���жϣ���Ϊ�ڲ�ͬ��ƽ̨�£�
    // �׽���������ʽ��ȱʡֵ��һ����
    if (isActive_)
        doSetBlockMode(handle_, value);
    isBlockMode_ = value;
}

//-----------------------------------------------------------------------------

void Socket::setHandle(SOCKET value)
{
    if (value != handle_)
    {
        if (isActive()) close();
        handle_ = value;
        if (handle_ != INVALID_SOCKET)
            isActive_ = true;
    }
}

//-----------------------------------------------------------------------------

void Socket::setActive(bool value)
{
    if (value != isActive_)
    {
        if (value) open();
        else close();
    }
}

//-----------------------------------------------------------------------------

void Socket::setDomain(int value)
{
    if (value != domain_)
    {
        if (isActive()) close();
        domain_ = value;
    }
}

//-----------------------------------------------------------------------------

void Socket::setType(int value)
{
    if (value != type_)
    {
        if (isActive()) close();
        type_ = value;
    }
}

//-----------------------------------------------------------------------------

void Socket::setProtocol(int value)
{
    if (value != protocol_)
    {
        if (isActive()) close();
        protocol_ = value;
    }
}

//-----------------------------------------------------------------------------
// ����: ���׽���
//-----------------------------------------------------------------------------
void Socket::bind(WORD port)
{
    SockAddr addr = InetAddress(ntohl(INADDR_ANY), port).getSockAddr();
    int optVal = 1;

    // ǿ�����°󶨣��������������ص�Ӱ��
    setsockopt(handle_, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal));

    // ���׽���
    if (::bind(handle_, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        iseThrowSocketLastError();
}

//-----------------------------------------------------------------------------

void Socket::doSetBlockMode(SOCKET handle, bool value)
{
#ifdef ISE_WINDOWS
    UINT notBlock = (value? 0 : 1);
    if (ioctlsocket(handle, FIONBIO, (u_long*)&notBlock) < 0)
        iseThrowSocketLastError();
#endif
#ifdef ISE_LINUX
    int flag = fcntl(handle, F_GETFL);

    if (value)
        flag &= ~O_NONBLOCK;
    else
        flag |= O_NONBLOCK;

    if (fcntl(handle, F_SETFL, flag) < 0)
        iseThrowSocketLastError();
#endif
}

//-----------------------------------------------------------------------------
// ��ע:
//   �� Winsock �У��رս���ͨ�� (SS_SD_RECV �� SS_SD_BOTH) �������ǰ���ջ�
//   ��������δȡ�����ݻ���֮���������ݵ��TCP�����Ͷ˷���RST������������
//   �����á��� Linux �µ���Ϊ������ͬ�������������ӡ�
//-----------------------------------------------------------------------------
void Socket::doClose()
{
#ifdef ISE_WINDOWS
    ::shutdown(handle_, SS_SD_SEND);
#endif
#ifdef ISE_LINUX
    ::shutdown(handle_, SS_SD_BOTH);
#endif

    iseCloseSocket(handle_);
    handle_ = INVALID_SOCKET;
    isActive_ = false;
}

///////////////////////////////////////////////////////////////////////////////
// class UdpSocket

//-----------------------------------------------------------------------------
// ����: ��������
//-----------------------------------------------------------------------------
int UdpSocket::recvBuffer(void *buffer, int size)
{
    InetAddress peerAddr;
    return recvBuffer(buffer, size, peerAddr);
}

//-----------------------------------------------------------------------------
// ����: ��������
//-----------------------------------------------------------------------------
int UdpSocket::recvBuffer(void *buffer, int size, InetAddress& peerAddr)
{
    SockAddr addr;
    int bytes;
    socklen_t sockLen = sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    bytes = recvfrom(handle_, (char*)buffer, size, 0, (struct sockaddr*)&addr, &sockLen);

    if (bytes > 0)
    {
        peerAddr.ip = ntohl(addr.sin_addr.s_addr);
        peerAddr.port = ntohs(addr.sin_port);
    }

    return bytes;
}

//-----------------------------------------------------------------------------
// ����: ��������
//-----------------------------------------------------------------------------
int UdpSocket::sendBuffer(void *buffer, int size, const InetAddress& peerAddr, int sendTimes)
{
    int result = 0;
    SockAddr addr = peerAddr.getSockAddr();
    socklen_t sockLen = sizeof(addr);

    for (int i = 0; i < sendTimes; i++)
        result = sendto(handle_, (char*)buffer, size, 0, (struct sockaddr*)&addr, sockLen);

    return result;
}

//-----------------------------------------------------------------------------
// ����: ���׽���
//-----------------------------------------------------------------------------
void UdpSocket::open()
{
    Socket::open();

#ifdef ISE_WINDOWS
    if (isActive_)
    {
        // Windows�£����յ�ICMP��("ICMP port unreachable")��recvfrom������-1��
        // ����Ϊ WSAECONNRESET(10054)��������ķ������ø���Ϊ��

        #define IOC_VENDOR        0x18000000
        #define _WSAIOW(x,y)      (IOC_IN|(x)|(y))
        #define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)

        DWORD bytesReturned = 0;
        BOOL newBehavior = FALSE;
        ::WSAIoctl(getHandle(), SIO_UDP_CONNRESET, &newBehavior, sizeof(newBehavior),
            NULL, 0, &bytesReturned, NULL, NULL);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
// class UdpServer

BaseUdpServer::BaseUdpServer() :
    localPort_(0),
    listenerThreadPool_(NULL)
{
    listenerThreadPool_ = new UdpListenerThreadPool(this);
    setListenerThreadCount(1);
}

//-----------------------------------------------------------------------------

BaseUdpServer::~BaseUdpServer()
{
    delete listenerThreadPool_;
}

//-----------------------------------------------------------------------------
// ����: ���ü����˿�
//-----------------------------------------------------------------------------
void BaseUdpServer::setLocalPort(WORD value)
{
    if (value != localPort_)
    {
        if (isActive()) close();
        localPort_ = value;
    }
}

//-----------------------------------------------------------------------------
// ����: ���� UDP ������
//-----------------------------------------------------------------------------
void BaseUdpServer::open()
{
    try
    {
        if (!isActive_)
        {
            UdpSocket::open();
            if (isActive_)
            {
                bind(localPort_);
                startListenerThreads();
            }
        }
    }
    catch (SocketException&)
    {
        close();
        throw;
    }
}

//-----------------------------------------------------------------------------
// ����: �ر� UDP ������
//-----------------------------------------------------------------------------
void BaseUdpServer::close()
{
    if (isActive())
    {
        stopListenerThreads();
        UdpSocket::close();
    }
}

//-----------------------------------------------------------------------------
// ����: ȡ�ü����̵߳�����
//-----------------------------------------------------------------------------
int BaseUdpServer::getListenerThreadCount() const
{
    return listenerThreadPool_->getMaxThreadCount();
}

//-----------------------------------------------------------------------------
// ����: ���ü����̵߳�����
//-----------------------------------------------------------------------------
void BaseUdpServer::setListenerThreadCount(int value)
{
    if (value < 1) value = 1;
    listenerThreadPool_->setMaxThreadCount(value);
}

//-----------------------------------------------------------------------------
// ����: ���á��յ����ݰ����Ļص�
//-----------------------------------------------------------------------------
void BaseUdpServer::setRecvDataCallback(const UdpSvrRecvDataCallback& callback)
{
    onRecvData_ = callback;
}

//-----------------------------------------------------------------------------
// ����: ���������߳�
//-----------------------------------------------------------------------------
void BaseUdpServer::startListenerThreads()
{
    listenerThreadPool_->startThreads();
}

//-----------------------------------------------------------------------------
// ����: ֹͣ�����߳�
//-----------------------------------------------------------------------------
void BaseUdpServer::stopListenerThreads()
{
    listenerThreadPool_->stopThreads();
}

//-----------------------------------------------------------------------------
// ����: �յ����ݰ�
//-----------------------------------------------------------------------------
void BaseUdpServer::dataReceived(void *packetBuffer, int packetSize, const InetAddress& peerAddr)
{
    if (onRecvData_)
        onRecvData_(packetBuffer, packetSize, peerAddr);
}

///////////////////////////////////////////////////////////////////////////////
// class TcpSocket

//-----------------------------------------------------------------------------
// ����: shutdown ����
//-----------------------------------------------------------------------------
void TcpSocket::shutdown(bool closeSend, bool closeRecv)
{
    if (isActive_ && (closeSend || closeRecv))
    {
        int how;
        if (closeSend && !closeRecv)
            how = SS_SD_SEND;
        else if (!closeSend && closeRecv)
            how = SS_SD_RECV;
        else if (closeSend && closeRecv)
            how = SS_SD_BOTH;

        ::shutdown(handle_, how);
    }
}

///////////////////////////////////////////////////////////////////////////////
// class BaseTcpConnection

BaseTcpConnection::BaseTcpConnection() :
    isDisconnected_(false)
{
    socket_.setBlockMode(false);
}

//-----------------------------------------------------------------------------

BaseTcpConnection::BaseTcpConnection(SOCKET socketHandle) :
    isDisconnected_(false)
{
    socket_.setHandle(socketHandle);
    socket_.setBlockMode(false);
}

//-----------------------------------------------------------------------------
// ����: ���ص�ǰ�Ƿ�Ϊ����״̬
//-----------------------------------------------------------------------------
bool BaseTcpConnection::isConnected() const
{
    return socket_.isActive() && !isDisconnected_;
}

//-----------------------------------------------------------------------------
// ����: �Ͽ�����
//-----------------------------------------------------------------------------
void BaseTcpConnection::disconnect()
{
    if (isConnected())
        doDisconnect();
    isDisconnected_ = true;
}

//-----------------------------------------------------------------------------
// ����: ִ�� shutdown ����
//-----------------------------------------------------------------------------
void BaseTcpConnection::shutdown(bool closeSend, bool closeRecv)
{
    socket_.shutdown(closeSend, closeRecv);
}

//-----------------------------------------------------------------------------
// ����: ���� TCP_NODELAY ��־
//-----------------------------------------------------------------------------
void BaseTcpConnection::setNoDelay(bool value)
{
    int optVal = value ? 1 : 0;
    ::setsockopt(getSocket().getHandle(), IPPROTO_TCP, TCP_NODELAY,
        (char*)&optVal, sizeof(optVal));
}

//-----------------------------------------------------------------------------
// ����: ���� SO_KEEPALIVE ��־
//-----------------------------------------------------------------------------
void BaseTcpConnection::setKeepAlive(bool value)
{
    int optVal = value ? 1 : 0;
    ::setsockopt(getSocket().getHandle(), IPPROTO_TCP, SO_KEEPALIVE,
        (char*)&optVal, sizeof(optVal));
}

//-----------------------------------------------------------------------------
// ����: ȡ�ô����ӵı��ص�ַ
//-----------------------------------------------------------------------------
const InetAddress& BaseTcpConnection::getLocalAddr() const
{
    if (localAddr_.isEmpty())
        localAddr_ = getSocket().getLocalAddr();
    return localAddr_;
}

//-----------------------------------------------------------------------------
// ����: ȡ�ô����ӵĶԷ���ַ
//-----------------------------------------------------------------------------
const InetAddress& BaseTcpConnection::getPeerAddr() const
{
    if (peerAddr_.isEmpty())
        peerAddr_ = getSocket().getPeerAddr();
    return peerAddr_;
}

//-----------------------------------------------------------------------------
// ����: �Ͽ�����
//-----------------------------------------------------------------------------
void BaseTcpConnection::doDisconnect()
{
    socket_.close();
}

//-----------------------------------------------------------------------------
// ����: ��������
//   syncMode     - �Ƿ���ͬ����ʽ����
//   timeoutMSecs - ָ����ʱʱ��(����)��������ָ��ʱ����δ������ȫ���������˳�������
//                   �� timeoutMSecs Ϊ -1�����ʾ�����г�ʱ��⡣
// ����:
//   < 0    - δ�����κ����ݣ��ҷ������ݹ��̷����˴���
//   >= 0   - ʵ�ʷ������ֽ�����
// ��ע:
//   1. �����׳��쳣��
//   2. ������Ϊ��ʱ�����أ�����ֵ����С��0��
//-----------------------------------------------------------------------------
int BaseTcpConnection::sendBuffer(void *buffer, int size, bool syncMode, int timeoutMSecs)
{
    int result = size;

    if (syncMode)
        result = doSyncSendBuffer(buffer, size, timeoutMSecs);
    else
        result = doAsyncSendBuffer(buffer, size);

    return result;
}

//-----------------------------------------------------------------------------
// ����: ��������
//   syncMode     - �Ƿ���ͬ����ʽ����
//   timeoutMSecs - ָ����ʱʱ��(����)��������ָ��ʱ����δ������ȫ���������˳�������
//                   �� timeoutMSecs Ϊ -1�����ʾ�����г�ʱ��⡣
// ����:
//   < 0    - δ���յ��κ����ݣ��ҽ������ݹ��̷����˴���
//   >= 0   - ʵ�ʽ��յ����ֽ�����
// ��ע:
//   1. �����׳��쳣��
//   2. ������Ϊ��ʱ�����أ�����ֵ����С��0��
//-----------------------------------------------------------------------------
int BaseTcpConnection::recvBuffer(void *buffer, int size, bool syncMode, int timeoutMSecs)
{
    int result = size;

    if (syncMode)
        result = doSyncRecvBuffer(buffer, size, timeoutMSecs);
    else
        result = doAsyncRecvBuffer(buffer, size);

    return result;
}

//-----------------------------------------------------------------------------
// ����: ��������
//   timeoutMSecs - ָ����ʱʱ��(����)��������ָ��ʱ����δ������ȫ���������˳�������
//                   �� timeoutMSecs Ϊ -1�����ʾ�����г�ʱ��⡣
// ����:
//   < 0    - δ�����κ����ݣ��ҷ������ݹ��̷����˴���
//   >= 0   - ʵ�ʷ������ֽ�����
// ��ע:
//   1. �����׳��쳣��
//   2. ������Ϊ��ʱ�����أ�����ֵ����С��0��
//   3. �˴����÷������׽���ģʽ���Ա��ܼ�ʱ�˳���
//-----------------------------------------------------------------------------
int BaseTcpConnection::doSyncSendBuffer(void *buffer, int size, int timeoutMSecs)
{
    const int SELECT_WAIT_MSEC = 250;    // ÿ�εȴ�ʱ�� (����)

    int result = -1;
    bool error = false;
    fd_set fds;
    struct timeval tv;
    SOCKET socketHandle = socket_.getHandle();
    int n, r, remainSize, index;
    UINT64 startTime, elapsedMSecs;

    if (size <= 0 || !socket_.isActive())
        return result;

    remainSize = size;
    index = 0;
    startTime = getCurTicks();

    while (socket_.isActive() && remainSize > 0)
    try
    {
        tv.tv_sec = 0;
        tv.tv_usec = (timeoutMSecs? SELECT_WAIT_MSEC * 1000 : 0);

        FD_ZERO(&fds);
        FD_SET((UINT)socketHandle, &fds);

        r = select(socketHandle + 1, NULL, &fds, NULL, &tv);
        if (r < 0)
        {
            if (iseSocketGetLastError() != SS_EINTR)
            {
                error = true;    // error
                break;
            }
        }

        if (r > 0 && socket_.isActive() && FD_ISSET(socketHandle, &fds))
        {
            n = send(socketHandle, &((char*)buffer)[index], remainSize, 0);
            if (n <= 0)
            {
                int errorCode = iseSocketGetLastError();
                if ((n == 0) || (errorCode != SS_EWOULDBLOCK && errorCode != SS_EINTR))
                {
                    error = true;    // error
                    break;
                }
                else
                    n = 0;
            }

            index += n;
            remainSize -= n;
        }

        // �����Ҫ��ʱ���
        if (timeoutMSecs >= 0 && remainSize > 0)
        {
            elapsedMSecs = getTickDiff(startTime, getCurTicks());
            if (elapsedMSecs >= (UINT64)timeoutMSecs)
                break;
        }
    }
    catch (...)
    {
        error = true;
        break;
    }

    if (index > 0)
        result = index;
    else if (error)
        result = -1;
    else
        result = 0;

    return result;
}

//-----------------------------------------------------------------------------
// ����: ��������
// ����:
//   timeoutMSecs - ָ����ʱʱ��(����)��������ָ��ʱ����δ������ȫ���������˳�������
//                   �� timeoutMSecs Ϊ -1�����ʾ�����г�ʱ��⡣
// ����:
//   < 0    - δ���յ��κ����ݣ��ҽ������ݹ��̷����˴���
//   >= 0   - ʵ�ʽ��յ����ֽ�����
// ��ע:
//   1. �����׳��쳣��
//   2. ������Ϊ��ʱ�����أ�����ֵ����С��0��
//   3. �˴����÷������׽���ģʽ���Ա��ܼ�ʱ�˳���
//-----------------------------------------------------------------------------
int BaseTcpConnection::doSyncRecvBuffer(void *buffer, int size, int timeoutMSecs)
{
    const int SELECT_WAIT_MSEC = 250;    // ÿ�εȴ�ʱ�� (����)

    int result = -1;
    bool error = false;
    fd_set fds;
    struct timeval tv;
    SOCKET socketHandle = socket_.getHandle();
    int n, r, remainSize, index;
    UINT64 startTime, elapsedMSecs;

    if (size <= 0 || !socket_.isActive())
        return result;

    remainSize = size;
    index = 0;
    startTime = getCurTicks();

    while (socket_.isActive() && remainSize > 0)
    try
    {
        tv.tv_sec = 0;
        tv.tv_usec = (timeoutMSecs? SELECT_WAIT_MSEC * 1000 : 0);

        FD_ZERO(&fds);
        FD_SET((UINT)socketHandle, &fds);

        r = select(socketHandle + 1, &fds, NULL, NULL, &tv);
        if (r < 0)
        {
            if (iseSocketGetLastError() != SS_EINTR)
            {
                error = true;    // error
                break;
            }
        }

        if (r > 0 && socket_.isActive() && FD_ISSET(socketHandle, &fds))
        {
            n = recv(socketHandle, &((char*)buffer)[index], remainSize, 0);
            if (n <= 0)
            {
                int errorCode = iseSocketGetLastError();
                if ((n == 0) || (errorCode != SS_EWOULDBLOCK && errorCode != SS_EINTR))
                {
                    error = true;    // error
                    break;
                }
                else
                    n = 0;
            }

            index += n;
            remainSize -= n;
        }

        // �����Ҫ��ʱ���
        if (timeoutMSecs >= 0 && remainSize > 0)
        {
            elapsedMSecs = getTickDiff(startTime, getCurTicks());
            if (elapsedMSecs >= (UINT64)timeoutMSecs)
                break;
        }
    }
    catch (...)
    {
        error = true;
        break;
    }

    if (index > 0)
        result = index;
    else if (error)
        result = -1;
    else
        result = 0;

    return result;
}

//-----------------------------------------------------------------------------
// ����: �������� (������)
// ����:
//   < 0    - δ�����κ����ݣ��ҷ������ݹ��̷����˴���
//   >= 0   - ʵ�ʷ������ֽ�����
// ��ע:
//   �����׳��쳣��
//-----------------------------------------------------------------------------
int BaseTcpConnection::doAsyncSendBuffer(void *buffer, int size)
{
    int result = -1;
    try
    {
        result = send(socket_.getHandle(), (char*)buffer, size, 0);
        if (result <= 0)
        {
            int errorCode = iseSocketGetLastError();
            if ((result == 0) || (errorCode != SS_EWOULDBLOCK && errorCode != SS_EINTR))
                result = -1;   // error
            else
                result = 0;
        }
    }
    catch (...)
    {}

    return result;
}

//-----------------------------------------------------------------------------
// ����: �������� (������)
// ����:
//   < 0    - δ���յ��κ����ݣ��ҽ������ݹ��̷����˴���
//   >= 0   - ʵ�ʽ��յ����ֽ�����
// ��ע:
//   �����׳��쳣��
//-----------------------------------------------------------------------------
int BaseTcpConnection::doAsyncRecvBuffer(void *buffer, int size)
{
    int result = -1;
    try
    {
        result = recv(socket_.getHandle(), (char*)buffer, size, 0);
        if (result <= 0)
        {
            int errorCode = iseSocketGetLastError();
            if ((result == 0) || (errorCode != SS_EWOULDBLOCK && errorCode != SS_EINTR))
                result = -1;   // error
            else
                result = 0;
        }
    }
    catch (...)
    {}

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// class TcpClient

BaseTcpClient::BaseTcpClient() :
    connection_(NULL)
{
    // nothing
}

BaseTcpClient::~BaseTcpClient()
{
    delete connection_;
    connection_ = NULL;
}

//-----------------------------------------------------------------------------
// ����: ����TCP�������� (����ʽ)
// ��ע: ������ʧ�ܣ����׳��쳣��
//-----------------------------------------------------------------------------
void BaseTcpClient::connect(const string& ip, int port)
{
    ensureConnCreated();
    TcpSocket& socket = getSocket();

    if (isConnected()) disconnect();

    try
    {
        socket.open();
        if (socket.isActive())
        {
            SockAddr addr = InetAddress(stringToIp(ip), static_cast<WORD>(port)).getSockAddr();

            bool oldBlockMode = socket.isBlockMode();
            socket.setBlockMode(true);

            if (::connect(socket.getHandle(), (struct sockaddr*)&addr, sizeof(addr)) < 0)
                iseThrowSocketLastError();

            socket.setBlockMode(oldBlockMode);
        }
    }
    catch (SocketException&)
    {
        socket.close();
        throw;
    }
}

//-----------------------------------------------------------------------------
// ����: ����TCP�������� (������ʽ)
// ����:
//   timeoutMSecs - ���ȴ��ĺ�������Ϊ-1��ʾ���ȴ�
// ����:
//   ACS_CONNECTING - ��δ������ϣ�����δ��������
//   ACS_CONNECTED  - �����ѽ����ɹ�
//   ACS_FAILED     - ���ӹ����з����˴��󣬵�������ʧ��
// ��ע:
//   �����쳣��
//-----------------------------------------------------------------------------
int BaseTcpClient::asyncConnect(const string& ip, int port, int timeoutMSecs)
{
    int result = ACS_CONNECTING;

    ensureConnCreated();
    TcpSocket& socket = getSocket();

    if (isConnected()) disconnect();

    try
    {
        socket.open();
        if (socket.isActive())
        {
            SockAddr addr = InetAddress(stringToIp(ip), static_cast<WORD>(port)).getSockAddr();

            socket.setBlockMode(false);
            int r = ::connect(socket.getHandle(), (struct sockaddr*)&addr, sizeof(addr));
            if (r == 0)
                result = ACS_CONNECTED;
#ifdef ISE_WINDOWS
            else if (iseSocketGetLastError() != SS_EWOULDBLOCK)
#endif
#ifdef ISE_LINUX
            else if (iseSocketGetLastError() != SS_EINPROGRESS)
#endif
                result = ACS_FAILED;
        }
    }
    catch (...)
    {
        socket.close();
        result = ACS_FAILED;
    }

    if (result == ACS_CONNECTING)
        result = checkAsyncConnectState(timeoutMSecs);

    return result;
}

//-----------------------------------------------------------------------------
// ����: ����첽���ӵ�״̬
// ����:
//   timeoutMSecs - ���ȴ��ĺ�������Ϊ-1��ʾ���ȴ�
// ����:
//   ACS_CONNECTING - ��δ������ϣ�����δ��������
//   ACS_CONNECTED  - �����ѽ����ɹ�
//   ACS_FAILED     - ���ӹ����з����˴��󣬵�������ʧ��
// ��ע:
//   �����쳣��
//-----------------------------------------------------------------------------
int BaseTcpClient::checkAsyncConnectState(int timeoutMSecs)
{
    if ((connection_ == NULL) || !getSocket().isActive()) return ACS_FAILED;

    const int WAIT_STEP = 100;   // ms
    int result = ACS_CONNECTING;
    SOCKET handle = getSocket().getHandle();
    fd_set rset, wset;
    struct timeval tv;
    int ms = 0;

    timeoutMSecs = max(timeoutMSecs, -1);

    while (true)
    {
        tv.tv_sec = 0;
        tv.tv_usec = (timeoutMSecs? WAIT_STEP * 1000 : 0);
        FD_ZERO(&rset);
        FD_SET((DWORD)handle, &rset);
        wset = rset;

        int r = select(handle + 1, &rset, &wset, NULL, &tv);
        if (r > 0 && (FD_ISSET(handle, &rset) || FD_ISSET(handle, &wset)))
        {
            socklen_t errLen = sizeof(int);
            int errorCode = 0;
            // If error occurs
            if (getsockopt(handle, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errLen) < 0 || errorCode)
                result = ACS_FAILED;
            else
                result = ACS_CONNECTED;
        }
        else if (r < 0)
        {
            if (iseSocketGetLastError() != SS_EINTR)
                result = ACS_FAILED;
        }

        if (result != ACS_CONNECTING)
            break;

        // Check timeout
        if (timeoutMSecs != -1)
        {
            ms += WAIT_STEP;
            if (ms >= timeoutMSecs)
                break;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------

void BaseTcpClient::disconnect()
{
    if (connection_)
    {
        connection_->disconnect();
        delete connection_;
        connection_ = NULL;
    }
}

//-----------------------------------------------------------------------------

BaseTcpConnection& BaseTcpClient::getConnection()
{
    ensureConnCreated();
    return *connection_;
}

//-----------------------------------------------------------------------------

void BaseTcpClient::ensureConnCreated()
{
    if (connection_ == NULL)
        connection_ = createConnection();
}

//-----------------------------------------------------------------------------

TcpSocket& BaseTcpClient::getSocket()
{
    ISE_ASSERT(connection_ != NULL);
    return connection_->getSocket();
}

///////////////////////////////////////////////////////////////////////////////
// class BaseTcpServer

BaseTcpServer::BaseTcpServer() :
    localPort_(0),
    listenerThread_(NULL)
{
    // nothing
}

//-----------------------------------------------------------------------------

BaseTcpServer::~BaseTcpServer()
{
    close();
}

//-----------------------------------------------------------------------------
// ����: ����TCP������
//-----------------------------------------------------------------------------
void BaseTcpServer::open()
{
    try
    {
        if (!isActive())
        {
            socket_.open();
            socket_.bind(localPort_);
            if (listen(socket_.getHandle(), LISTEN_QUEUE_SIZE) < 0)
                iseThrowSocketLastError();
            startListenerThread();
        }
    }
    catch (SocketException&)
    {
        close();
        throw;
    }
}

//-----------------------------------------------------------------------------
// ����: �ر�TCP������
//-----------------------------------------------------------------------------
void BaseTcpServer::close()
{
    if (isActive())
    {
        stopListenerThread();
        socket_.close();
    }
}

//-----------------------------------------------------------------------------
// ����: ����/�ر�TCP������
//-----------------------------------------------------------------------------
void BaseTcpServer::setActive(bool value)
{
    if (isActive() != value)
    {
        if (value) open();
        else close();
    }
}

//-----------------------------------------------------------------------------
// ����: ����TCP�����������˿�
//-----------------------------------------------------------------------------
void BaseTcpServer::setLocalPort(WORD value)
{
    if (value != localPort_)
    {
        if (isActive()) close();
        localPort_ = value;
    }
}

//-----------------------------------------------------------------------------
// ����: ���á����������ӡ��Ļص�
//-----------------------------------------------------------------------------
void BaseTcpServer::setCreateConnCallback(const TcpSvrCreateConnCallback& callback)
{
    onCreateConn_ = callback;
}

//-----------------------------------------------------------------------------
// ����: ���á��յ����ӡ��Ļص�
//-----------------------------------------------------------------------------
void BaseTcpServer::setAcceptConnCallback(const TcpSvrAcceptConnCallback& callback)
{
    onAcceptConn_ = callback;
}

//-----------------------------------------------------------------------------
// ����: ���������߳�
//-----------------------------------------------------------------------------
void BaseTcpServer::startListenerThread()
{
    if (!listenerThread_)
    {
        listenerThread_ = new TcpListenerThread(this);
        listenerThread_->run();
    }
}

//-----------------------------------------------------------------------------
// ����: ֹͣ�����߳�
//-----------------------------------------------------------------------------
void BaseTcpServer::stopListenerThread()
{
    if (listenerThread_)
    {
        listenerThread_->terminate();
        listenerThread_->waitFor();
        delete listenerThread_;
        listenerThread_ = NULL;
    }
}

//-----------------------------------------------------------------------------
// ����: �������Ӷ���
//-----------------------------------------------------------------------------
BaseTcpConnection* BaseTcpServer::createConnection(SOCKET socketHandle)
{
    BaseTcpConnection *result = NULL;

    if (onCreateConn_)
        onCreateConn_(this, socketHandle, result);

    if (result == NULL)
        result = new BaseTcpConnection(socketHandle);

    return result;
}

//-----------------------------------------------------------------------------
// ����: �յ����� (ע: connection �ǶѶ�����ʹ�����ͷ�)
//-----------------------------------------------------------------------------
void BaseTcpServer::acceptConnection(BaseTcpConnection *connection)
{
    if (onAcceptConn_)
        onAcceptConn_(this, connection);
    else
        delete connection;
}

///////////////////////////////////////////////////////////////////////////////
// class UdpListenerThread

UdpListenerThread::UdpListenerThread(UdpListenerThreadPool *threadPool, int index) :
    threadPool_(threadPool),
    index_(index)
{
    setAutoDelete(true);
    udpServer_ = &(threadPool->getUdpServer());
    threadPool_->registerThread(this);
}

//-----------------------------------------------------------------------------

UdpListenerThread::~UdpListenerThread()
{
    threadPool_->unregisterThread(this);
}

//-----------------------------------------------------------------------------
// ����: UDP��������������
//-----------------------------------------------------------------------------
void UdpListenerThread::execute()
{
    const int MAX_UDP_BUFFER_SIZE = 8192;   // UDP���ݰ�����ֽ���
    const int SELECT_WAIT_MSEC    = 100;    // ÿ�εȴ�ʱ�� (����)

    fd_set fds;
    struct timeval tv;
    SOCKET socketHandle = udpServer_->getHandle();
    Buffer packetBuffer(MAX_UDP_BUFFER_SIZE);
    InetAddress peerAddr;
    int r, n;

    while (!isTerminated() && udpServer_->isActive())
    try
    {
        // �趨ÿ�εȴ�ʱ��
        tv.tv_sec = 0;
        tv.tv_usec = SELECT_WAIT_MSEC * 1000;

        FD_ZERO(&fds);
        FD_SET((UINT)socketHandle, &fds);

        r = select(socketHandle + 1, &fds, NULL, NULL, &tv);

        if (r > 0 && udpServer_->isActive() && FD_ISSET(socketHandle, &fds))
        {
            n = udpServer_->recvBuffer(packetBuffer.data(), MAX_UDP_BUFFER_SIZE, peerAddr);
            if (n > 0)
            {
                udpServer_->dataReceived(packetBuffer.data(), n, peerAddr);
            }
        }
        else if (r < 0)
        {
            int errorCode = iseSocketGetLastError();
            if (errorCode != SS_EINTR && errorCode != SS_EINPROGRESS)
                break;  // error
        }
    }
    catch (Exception&)
    {}
}

///////////////////////////////////////////////////////////////////////////////
// class UdpListenerThreadPool

UdpListenerThreadPool::UdpListenerThreadPool(BaseUdpServer *udpServer) :
    udpServer_(udpServer),
    maxThreadCount_(0)
{
    // nothing
}

//-----------------------------------------------------------------------------

UdpListenerThreadPool::~UdpListenerThreadPool()
{
    // nothing
}

//-----------------------------------------------------------------------------
// ����: ע���߳�
//-----------------------------------------------------------------------------
void UdpListenerThreadPool::registerThread(UdpListenerThread *thread)
{
    threadList_.add(thread);
}

//-----------------------------------------------------------------------------
// ����: ע���߳�
//-----------------------------------------------------------------------------
void UdpListenerThreadPool::unregisterThread(UdpListenerThread *thread)
{
    threadList_.remove(thread);
}

//-----------------------------------------------------------------------------
// ����: �����������߳�
//-----------------------------------------------------------------------------
void UdpListenerThreadPool::startThreads()
{
    for (int i = 0; i < maxThreadCount_; i++)
    {
        UdpListenerThread *thread;
        thread = new UdpListenerThread(this, i);
        thread->run();
    }
}

//-----------------------------------------------------------------------------
// ����: ֪ͨ���ȴ������߳��˳�
//-----------------------------------------------------------------------------
void UdpListenerThreadPool::stopThreads()
{
    const int MAX_WAIT_FOR_SECS = 5;
    threadList_.waitForAllThreads(MAX_WAIT_FOR_SECS);
}

///////////////////////////////////////////////////////////////////////////////
// class TcpListenerThread

TcpListenerThread::TcpListenerThread(BaseTcpServer *tcpServer) :
    tcpServer_(tcpServer)
{
    setAutoDelete(false);
}

//-----------------------------------------------------------------------------
// ����: TCP��������������
//-----------------------------------------------------------------------------
void TcpListenerThread::execute()
{
    const int SELECT_WAIT_MSEC = 100;    // ÿ�εȴ�ʱ�� (����)

    fd_set fds;
    struct timeval tv;
    SockAddr Addr;
    socklen_t nSockLen = sizeof(Addr);
    SOCKET socketHandle = tcpServer_->getSocket().getHandle();
    InetAddress peerAddr;
    SOCKET acceptHandle;
    int r;

    while (!isTerminated() && tcpServer_->isActive())
    try
    {
        // �趨ÿ�εȴ�ʱ��
        tv.tv_sec = 0;
        tv.tv_usec = SELECT_WAIT_MSEC * 1000;

        FD_ZERO(&fds);
        FD_SET((UINT)socketHandle, &fds);

        r = select(socketHandle + 1, &fds, NULL, NULL, &tv);

        if (r > 0 && tcpServer_->isActive() && FD_ISSET(socketHandle, &fds))
        {
            acceptHandle = accept(socketHandle, (struct sockaddr*)&Addr, &nSockLen);
            if (acceptHandle != INVALID_SOCKET)
            {
                peerAddr = InetAddress(ntohl(Addr.sin_addr.s_addr), ntohs(Addr.sin_port));
                BaseTcpConnection *connection = tcpServer_->createConnection(acceptHandle);
                tcpServer_->acceptConnection(connection);
            }
        }
        else if (r < 0)
        {
            int errorCode = iseSocketGetLastError();
            if (errorCode != SS_EINTR && errorCode != SS_EINPROGRESS)
                break;  // error
        }
    }
    catch (Exception&)
    {}
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise
