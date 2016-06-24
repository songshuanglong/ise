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
// ise_dbi_mysql.h
// Classes:
//   * MySqlConnection       - ���ݿ�������
//   * MySqlField            - �ֶ�������
//   * MySqlDataSet          - ���ݼ���
//   * MySqlQuery            - ���ݲ�ѯ����
//   * MySqlDatabase         - ���ݿ���
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_DBI_MYSQL_H_
#define _ISE_DBI_MYSQL_H_

#include "ise/main/ise_database.h"

#include <errmsg.h>
#include <mysql.h>

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ��ǰ����

class MySqlConnection;
class MySqlField;
class MySqlDataSet;
class MySqlQuery;
class MySqlDatabase;

///////////////////////////////////////////////////////////////////////////////
// class MySqlConnection - ���ݿ�������

class MySqlConnection : public DbConnection
{
public:
    MySqlConnection(Database *database);
    virtual ~MySqlConnection();

    // �������� (��ʧ�����׳��쳣)
    virtual void doConnect();
    // �Ͽ�����
    virtual void doDisconnect();

    // ȡ��MySQL���Ӷ���
    MYSQL& getConnObject() { return connObject_; }

private:
    MYSQL connObject_;            // ���Ӷ���
};

///////////////////////////////////////////////////////////////////////////////
// class MySqlField - �ֶ�������

class MySqlField : public DbField
{
public:
    MySqlField();
    virtual ~MySqlField() {}

    void setData(void *dataPtr, int dataSize);
    virtual bool isNull() const { return (dataPtr_ == NULL); }
    virtual string asString() const;

private:
    char* dataPtr_;               // ָ���ֶ�����
    int dataSize_;                // �ֶ����ݵ����ֽ���
};

///////////////////////////////////////////////////////////////////////////////
// class MySqlDataSet - ���ݼ���

class MySqlDataSet : public DbDataSet
{
public:
    MySqlDataSet(DbQuery* dbQuery);
    virtual ~MySqlDataSet();

    virtual bool rewind();
    virtual bool next();

    virtual UINT64 getRecordCount();
    virtual bool isEmpty();

protected:
    virtual void initDataSet();
    virtual void initFieldDefs();

private:
    MYSQL& getConnObject();
    void freeDataSet();

private:
    MYSQL_RES* res_;      // MySQL��ѯ�����
    MYSQL_ROW row_;       // MySQL��ѯ�����
};

///////////////////////////////////////////////////////////////////////////////
// class MySqlQuery - ��ѯ����

class MySqlQuery : public DbQuery
{
public:
    MySqlQuery(Database *database);
    virtual ~MySqlQuery();

    virtual string escapeString(const string& str);
    virtual UINT getAffectedRowCount();
    virtual UINT64 getLastInsertId();

protected:
    virtual void doExecute(DbDataSet *resultDataSet);

private:
    MYSQL& getConnObject();
};

///////////////////////////////////////////////////////////////////////////////
// class MySqlDatabase

class MySqlDatabase : public Database
{
public:
    virtual DbConnection* createDbConnection() { return new MySqlConnection(this); }
    virtual DbDataSet* createDbDataSet(DbQuery* dbQuery) { return new MySqlDataSet(dbQuery); }
    virtual DbQuery* createDbQuery() { return new MySqlQuery(this); }
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_DBI_MYSQL_H_
