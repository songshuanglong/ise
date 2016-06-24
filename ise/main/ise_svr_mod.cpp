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
// �ļ�����: ise_svr_mod.cpp
// ��������: ����ģ��֧��
///////////////////////////////////////////////////////////////////////////////

#include "ise/main/ise_svr_mod.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// classs IseServerModule

void IseServerModule::broadcastMessage(BaseSvrModMessage& message)
{
    (static_cast<IseSvrModBusiness*>(&iseApp().iseBusiness()))->broadcastMessage(message);
}

///////////////////////////////////////////////////////////////////////////////
// classs IseServerModuleMgr

IseServerModuleMgr::IseServerModuleMgr()
{
    // nothing
}

IseServerModuleMgr::~IseServerModuleMgr()
{
    clearServerModuleList();
}

//-----------------------------------------------------------------------------
// ����: ��ʼ������ģ���б�
//-----------------------------------------------------------------------------
void IseServerModuleMgr::initServerModuleList(const IseServerModuleList& list)
{
    items_.clear();
    for (size_t i = 0; i < list.size(); ++i)
    {
        IseServerModule *module = list[i];
        module->svrModIndex_ = static_cast<int>(i);
        items_.push_back(module);
    }
}

//-----------------------------------------------------------------------------
// ����: ������з���ģ��
//-----------------------------------------------------------------------------
void IseServerModuleMgr::clearServerModuleList()
{
    for (size_t i = 0; i < items_.size(); ++i)
        delete items_[i];
    items_.clear();
}

///////////////////////////////////////////////////////////////////////////////
// class IseSvrModBusiness

//-----------------------------------------------------------------------------
// ����: ��ʼ�� (ʧ�����׳��쳣)
//-----------------------------------------------------------------------------
void IseSvrModBusiness::initialize()
{
    IseBusiness::initialize();

    // �������з���ģ��
    IseServerModuleList svrModList;
    createServerModules(svrModList);
    serverModuleMgr_.initServerModuleList(svrModList);

    // ��ʼ������ӳ���
    initActionCodeMap();
    initUdpGroupIndexMap();
    initTcpServerIndexMap();

    // ����ISE����
    updateIseOptions();
}

//-----------------------------------------------------------------------------
// ����: ������ (���۳�ʼ���Ƿ����쳣������ʱ����ִ��)
//-----------------------------------------------------------------------------
void IseSvrModBusiness::finalize()
{
    try { serverModuleMgr_.clearServerModuleList(); } catch (...) {}

    IseBusiness::finalize();
}

//-----------------------------------------------------------------------------
// ����: UDP���ݰ�����
//-----------------------------------------------------------------------------
void IseSvrModBusiness::classifyUdpPacket(void *packetBuffer, int packetSize, int& groupIndex)
{
    groupIndex = -1;
    if (packetSize <= 0 || !filterUdpPacket(packetBuffer, packetSize)) return;

    UINT actionCode = getUdpPacketActionCode(packetBuffer, packetSize);
    ActionCodeMap::iterator iter = actionCodeMap_.find(actionCode);
    if (iter != actionCodeMap_.end())
    {
        groupIndex = iter->second;
    }
}

//-----------------------------------------------------------------------------
// ����: �յ���UDP���ݰ�
//-----------------------------------------------------------------------------
void IseSvrModBusiness::onRecvedUdpPacket(UdpWorkerThread& workerThread,
    int groupIndex, UdpPacket& packet)
{
    UdpGroupIndexMap::iterator iter = udpGroupIndexMap_.find(groupIndex);
    if (iter != udpGroupIndexMap_.end())
    {
        int modIndex = iter->second;
        serverModuleMgr_.getItem(modIndex).onRecvedUdpPacket(workerThread, packet);
    }
}

//-----------------------------------------------------------------------------
// ����: ������һ���µ�TCP����
//-----------------------------------------------------------------------------
void IseSvrModBusiness::onTcpConnected(const TcpConnectionPtr& connection)
{
    TcpServerIndexMap::iterator iter = tcpServerIndexMap_.find(connection->getServerIndex());
    if (iter != tcpServerIndexMap_.end())
    {
        int modIndex = iter->second;
        serverModuleMgr_.getItem(modIndex).onTcpConnected(connection);
    }
}

//-----------------------------------------------------------------------------
// ����: �Ͽ���һ��TCP����
//-----------------------------------------------------------------------------
void IseSvrModBusiness::onTcpDisconnected(const TcpConnectionPtr& connection)
{
    TcpServerIndexMap::iterator iter = tcpServerIndexMap_.find(connection->getServerIndex());
    if (iter != tcpServerIndexMap_.end())
    {
        int modIndex = iter->second;
        serverModuleMgr_.getItem(modIndex).onTcpDisconnected(connection);
    }
}

//-----------------------------------------------------------------------------
// ����: TCP�����ϵ�һ���������������
//-----------------------------------------------------------------------------
void IseSvrModBusiness::onTcpRecvComplete(const TcpConnectionPtr& connection, void *packetBuffer,
    int packetSize, const Context& context)
{
    TcpServerIndexMap::iterator iter = tcpServerIndexMap_.find(connection->getServerIndex());
    if (iter != tcpServerIndexMap_.end())
    {
        int modIndex = iter->second;
        serverModuleMgr_.getItem(modIndex).onTcpRecvComplete(connection,
            packetBuffer, packetSize, context);
    }
}

//-----------------------------------------------------------------------------
// ����: TCP�����ϵ�һ���������������
//-----------------------------------------------------------------------------
void IseSvrModBusiness::onTcpSendComplete(const TcpConnectionPtr& connection, const Context& context)
{
    TcpServerIndexMap::iterator iter = tcpServerIndexMap_.find(connection->getServerIndex());
    if (iter != tcpServerIndexMap_.end())
    {
        int modIndex = iter->second;
        serverModuleMgr_.getItem(modIndex).onTcpSendComplete(connection, context);
    }
}

//-----------------------------------------------------------------------------
// ����: ���������߳�ִ��(assistorIndex: 0-based)
//-----------------------------------------------------------------------------
void IseSvrModBusiness::assistorThreadExecute(AssistorThread& assistorThread, int assistorIndex)
{
    int index1, index2 = 0;

    for (int i = 0; i < serverModuleMgr_.getCount(); i++)
    {
        index1 = index2;
        index2 += serverModuleMgr_.getItem(i).getAssistorThreadCount();

        if (assistorIndex >= index1 && assistorIndex < index2)
        {
            serverModuleMgr_.getItem(i).assistorThreadExecute(assistorThread, assistorIndex - index1);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// ����: ϵͳ�ػ��߳�ִ�� (secondCount: 0-based)
//-----------------------------------------------------------------------------
void IseSvrModBusiness::daemonThreadExecute(Thread& thread, int secondCount)
{
    for (int i = 0; i < serverModuleMgr_.getCount(); i++)
        serverModuleMgr_.getItem(i).daemonThreadExecute(thread, secondCount);
}

//-----------------------------------------------------------------------------
// ����: ���ݷ���ģ����ź�ģ���ڵľֲ������߳���ţ�ȡ�ô˸����̵߳�ȫ�����
//-----------------------------------------------------------------------------
int IseSvrModBusiness::getAssistorIndex(int serverModuleIndex, int localAssistorIndex)
{
    int result = -1;

    if (serverModuleIndex >= 0 && serverModuleIndex < serverModuleMgr_.getCount())
    {
        result = 0;
        for (int i = 0; i < serverModuleIndex; i++)
            result += serverModuleMgr_.getItem(i).getAssistorThreadCount();
        result += localAssistorIndex;
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: �㲥��Ϣ������ģ��
//-----------------------------------------------------------------------------
void IseSvrModBusiness::broadcastMessage(BaseSvrModMessage& message)
{
    for (int i = 0; i < serverModuleMgr_.getCount(); i++)
    {
        if (message.isHandled) break;
        serverModuleMgr_.getItem(i).onSvrModMessage(message);
    }
}

//-----------------------------------------------------------------------------
// ����: ȡ��ȫ��UDP�������
//-----------------------------------------------------------------------------
int IseSvrModBusiness::getUdpGroupCount()
{
    int result = 0;

    for (int i = 0; i < serverModuleMgr_.getCount(); i++)
    {
        IseServerModule& serverModule = serverModuleMgr_.getItem(i);
        result += serverModule.getUdpGroupCount();
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: ȡ��ȫ��TCP����������
//-----------------------------------------------------------------------------
int IseSvrModBusiness::getTcpServerCount()
{
    int result = 0;

    for (int i = 0; i < serverModuleMgr_.getCount(); i++)
    {
        IseServerModule& serverModule = serverModuleMgr_.getItem(i);
        result += serverModule.getTcpServerCount();
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: ��ʼ�� actionCodeMap_
//-----------------------------------------------------------------------------
void IseSvrModBusiness::initActionCodeMap()
{
    int globalGroupIndex = 0;
    actionCodeMap_.clear();

    for (int modIndex = 0; modIndex < serverModuleMgr_.getCount(); modIndex++)
    {
        IseServerModule& serverModule = serverModuleMgr_.getItem(modIndex);
        for (int groupIndex = 0; groupIndex < serverModule.getUdpGroupCount(); groupIndex++)
        {
            ActionCodeArray actionCodes;
            serverModule.getUdpGroupActionCodes(groupIndex, actionCodes);

            for (UINT i = 0; i < actionCodes.size(); i++)
                actionCodeMap_[actionCodes[i]] = globalGroupIndex;

            globalGroupIndex++;
        }
    }
}

//-----------------------------------------------------------------------------
// ����: ��ʼ�� udpGroupIndexMap_
//-----------------------------------------------------------------------------
void IseSvrModBusiness::initUdpGroupIndexMap()
{
    int globalGroupIndex = 0;
    udpGroupIndexMap_.clear();

    for (int modIndex = 0; modIndex < serverModuleMgr_.getCount(); modIndex++)
    {
        IseServerModule& serverModule = serverModuleMgr_.getItem(modIndex);
        for (int groupIndex = 0; groupIndex < serverModule.getUdpGroupCount(); groupIndex++)
        {
            udpGroupIndexMap_[globalGroupIndex] = modIndex;
            globalGroupIndex++;
        }
    }
}

//-----------------------------------------------------------------------------
// ����: ��ʼ�� tcpServerIndexMap_
//-----------------------------------------------------------------------------
void IseSvrModBusiness::initTcpServerIndexMap()
{
    int globalServerIndex = 0;
    tcpServerIndexMap_.clear();

    for (int modIndex = 0; modIndex < serverModuleMgr_.getCount(); modIndex++)
    {
        IseServerModule& serverModule = serverModuleMgr_.getItem(modIndex);
        for (int serverIndex = 0; serverIndex < serverModule.getTcpServerCount(); serverIndex++)
        {
            tcpServerIndexMap_[globalServerIndex] = modIndex;
            globalServerIndex++;
        }
    }
}

//-----------------------------------------------------------------------------
// ����: ����ISE����
//-----------------------------------------------------------------------------
void IseSvrModBusiness::updateIseOptions()
{
    IseOptions& options = iseApp().iseOptions();

    // ����UDP��������������
    options.setUdpRequestGroupCount(getUdpGroupCount());
    // ����TCP��������������
    options.setTcpServerCount(getTcpServerCount());

    // UDP��������������
    int globalGroupIndex = 0;
    for (int modIndex = 0; modIndex < serverModuleMgr_.getCount(); modIndex++)
    {
        IseServerModule& serverModule = serverModuleMgr_.getItem(modIndex);
        for (int groupIndex = 0; groupIndex < serverModule.getUdpGroupCount(); groupIndex++)
        {
            UdpGroupOptions grpOpt;
            serverModule.getUdpGroupOptions(groupIndex, grpOpt);
            options.setUdpRequestQueueCapacity(globalGroupIndex, grpOpt.requestQueueCapacity);
            options.setUdpWorkerThreadCount(globalGroupIndex, grpOpt.minWorkerThreads, grpOpt.maxWorkerThreads);
            globalGroupIndex++;
        }
    }

    // TCP�������������
    int globalServerIndex = 0;
    for (int modIndex = 0; modIndex < serverModuleMgr_.getCount(); modIndex++)
    {
        IseServerModule& serverModule = serverModuleMgr_.getItem(modIndex);
        for (int serverIndex = 0; serverIndex < serverModule.getTcpServerCount(); serverIndex++)
        {
            TcpServerOptions svrOpt;
            serverModule.getTcpServerOptions(serverIndex, svrOpt);
            options.setTcpServerPort(globalServerIndex, svrOpt.serverPort);
            options.setTcpServerEventLoopCount(globalServerIndex, svrOpt.eventLoopCount);
            globalServerIndex++;
        }
    }

    // ���ø��������̵߳�����
    int assistorCount = 0;
    for (int i = 0; i < serverModuleMgr_.getCount(); i++)
        assistorCount += serverModuleMgr_.getItem(i).getAssistorThreadCount();
    options.setAssistorThreadCount(assistorCount);
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise
