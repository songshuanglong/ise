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
// ise_svr_mod.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_SVR_MOD_H_
#define _ISE_SVR_MOD_H_

#include "ise/main/ise_options.h"
#include "ise/main/ise_application.h"
#include "ise/main/ise_svr_mod_msgs.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ��ǰ����

class IseServerModule;
class IseServerModuleMgr;
class IseSvrModBusiness;

///////////////////////////////////////////////////////////////////////////////
// ���Ͷ���

// ������������
typedef std::vector<UINT> ActionCodeArray;

typedef IseOptions::UdpRequestGroupOption UdpGroupOptions;
typedef IseOptions::TcpServerOption TcpServerOptions;

typedef std::vector<IseServerModule*> IseServerModuleList;

///////////////////////////////////////////////////////////////////////////////
// class IseServerModule - ������ģ�����

class IseServerModule : boost::noncopyable
{
public:
    friend class IseServerModuleMgr;
public:
    IseServerModule() : svrModIndex_(0) {}
    virtual ~IseServerModule() {}

    // ȡ�ø÷���ģ���е�UDP�������
    virtual int getUdpGroupCount() { return 0; }
    // ȡ�ø�ģ����ĳUDP������ӹܵĶ�������
    virtual void getUdpGroupActionCodes(int groupIndex, ActionCodeArray& list) {}
    // ȡ�ø�ģ����ĳUDP��������
    virtual void getUdpGroupOptions(int groupIndex, UdpGroupOptions& options) {}
    // ȡ�ø÷���ģ���е�TCP����������
    virtual int getTcpServerCount() { return 0; }
    // ȡ�ø�ģ����ĳTCP������������
    virtual void getTcpServerOptions(int serverIndex, TcpServerOptions& options) {}

    // �յ���UDP���ݰ�
    virtual void onRecvedUdpPacket(UdpWorkerThread& workerThread, UdpPacket& packet) {}

    // ������һ���µ�TCP����
    virtual void onTcpConnected(const TcpConnectionPtr& connection) {}
    // �Ͽ���һ��TCP����
    virtual void onTcpDisconnected(const TcpConnectionPtr& connection) {}
    // TCP�����ϵ�һ���������������
    virtual void onTcpRecvComplete(const TcpConnectionPtr& connection, void *packetBuffer,
        int packetSize, const Context& context) {}
    // TCP�����ϵ�һ���������������
    virtual void onTcpSendComplete(const TcpConnectionPtr& connection, const Context& context) {}

    // ���ش�ģ�����踨�������̵߳�����
    virtual int getAssistorThreadCount() { return 0; }
    // ���������߳�ִ��(assistorIndex: 0-based)
    virtual void assistorThreadExecute(AssistorThread& assistorThread, int assistorIndex) {}
    // ϵͳ�ػ��߳�ִ�� (secondCount: 0-based)
    virtual void daemonThreadExecute(Thread& thread, int secondCount) {}

    // �յ�����ģ����Ϣ
    virtual void onSvrModMessage(BaseSvrModMessage& message) {}

    // ���ط���ģ�����
    int getSvrModIndex() const { return svrModIndex_; }

    // �����з���ģ��㲥��Ϣ
    void broadcastMessage(BaseSvrModMessage& message);

private:
    int svrModIndex_;   // (0-based)
};

///////////////////////////////////////////////////////////////////////////////
// class IseServerModuleMgr - ����ģ�������

class IseServerModuleMgr : boost::noncopyable
{
public:
    IseServerModuleMgr();
    virtual ~IseServerModuleMgr();

    void initServerModuleList(const IseServerModuleList& list);
    void clearServerModuleList();

    inline int getCount() const { return static_cast<int>(items_.size()); }
    inline IseServerModule& getItem(int index) { return *items_[index]; }

private:
    IseServerModuleList items_;   // ����ģ���б�( (IseServerModule*)[] )
};

///////////////////////////////////////////////////////////////////////////////
// class IseSvrModBusiness - ֧�ַ���ģ���ISEҵ����

class IseSvrModBusiness : public IseBusiness
{
public:
    friend class IseServerModule;

public:
    IseSvrModBusiness() {}
    virtual ~IseSvrModBusiness() {}
public:
    virtual void initialize();
    virtual void finalize();

    virtual void classifyUdpPacket(void *packetBuffer, int packetSize, int& groupIndex);
    virtual void onRecvedUdpPacket(UdpWorkerThread& workerThread, int groupIndex, UdpPacket& packet);

    virtual void onTcpConnected(const TcpConnectionPtr& connection);
    virtual void onTcpDisconnected(const TcpConnectionPtr& connection);
    virtual void onTcpRecvComplete(const TcpConnectionPtr& connection, void *packetBuffer,
        int packetSize, const Context& context);
    virtual void onTcpSendComplete(const TcpConnectionPtr& connection, const Context& context);

    virtual void assistorThreadExecute(AssistorThread& assistorThread, int assistorIndex);
    virtual void daemonThreadExecute(Thread& thread, int secondCount);

public:
    int getAssistorIndex(int serverModuleIndex, int localAssistorIndex);
    void broadcastMessage(BaseSvrModMessage& message);

protected:
    // UDP���ݰ����˺��� (����: true-��Ч��, false-��Ч��)
    virtual bool filterUdpPacket(void *packetBuffer, int packetSize) { return true; }
    // ȡ��UDP���ݰ��еĶ�������
    virtual UINT getUdpPacketActionCode(void *packetBuffer, int packetSize) { return 0; }
    // �������з���ģ��
    virtual void createServerModules(IseServerModuleList& svrModList) {}

private:
    int getUdpGroupCount();
    int getTcpServerCount();
    void initActionCodeMap();
    void initUdpGroupIndexMap();
    void initTcpServerIndexMap();
    void updateIseOptions();

protected:
    typedef std::map<UINT, int> ActionCodeMap;       // <��������, UDP����>
    typedef std::map<UINT, int> UdpGroupIndexMap;    // <ȫ��UDP����, ����ģ���>
    typedef std::map<UINT, int> TcpServerIndexMap;   // <ȫ��TCP���������, ����ģ���>

    IseServerModuleMgr serverModuleMgr_;             // ����ģ�������
    ActionCodeMap actionCodeMap_;                    // <��������, UDP����> ӳ���
    UdpGroupIndexMap udpGroupIndexMap_;              // <ȫ��UDP����, ����ģ���> ӳ���
    TcpServerIndexMap tcpServerIndexMap_;            // <ȫ��TCP���������, ����ģ���> ӳ���
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_SVR_MOD_H_
