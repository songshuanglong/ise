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
// ise_xml.h
// Classes:
//   * XmlNode
//   * XmlNodeProps
//   * XmlDocument
//   * XmlReader
//   * XmlWriter
//   * XmlDocParser
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_EXT_UTILS_XML_H_
#define _ISE_EXT_UTILS_XML_H_

#include "ise/main/ise_options.h"
#include "ise/main/ise_global_defs.h"
#include "ise/main/ise_err_msgs.h"
#include "ise/main/ise_classes.h"
#include "ise/main/ise_sys_utils.h"

namespace ise
{

namespace utils
{

///////////////////////////////////////////////////////////////////////////////
// ��ǰ����

class XmlNode;
class XmlNodeProps;

///////////////////////////////////////////////////////////////////////////////
// ���Ͷ���

// XML��ǩ���
enum XML_TAG_TYPE
{
    XTT_START_TAG   = 0x01,      // ��ʼ��ǩ
    XTT_END_TAG     = 0x02,      // ������ǩ
};

typedef UINT XML_TAG_TYPES;

///////////////////////////////////////////////////////////////////////////////
// ��������

const int DEF_XML_INDENT_SPACES = 4;
const char* const S_DEF_XML_DOC_VER = "1.0";

///////////////////////////////////////////////////////////////////////////////
// �����

string strToXml(const string& str);
string xmlToStr(const string& str);

///////////////////////////////////////////////////////////////////////////////
// class XmlNode

class XmlNode
{
public:
    XmlNode();
    XmlNode(const XmlNode& src);
    virtual ~XmlNode();

    XmlNode& operator = (const XmlNode& rhs);

    void addNode(XmlNode *node);
    XmlNode* addNode();
    XmlNode* addNode(const string& name, const string& dataString);
    void insertNode(int index, XmlNode *node);

    XmlNode* findChildNode(const string& name);
    int indexOf(const string& name);
    void clear();

    XmlNode* getRootNode() const;
    XmlNode* getParentNode() const { return parentNode_; }
    int getChildCount() const;
    XmlNode* getChildNodes(int index) const;
    XmlNodeProps* getProps() const { return props_; }
    string getName() const { return name_; }
    string getDataString() const { return dataString_; }

    void setParentNode(XmlNode *node);
    void setName(const string& value) { name_ = value; }
    void setDataString(const string& value);

private:
    void initObject();
    void assignNode(XmlNode& src, XmlNode& dest);

private:
    XmlNode *parentNode_;       // ���ڵ�
    PointerList *childNodes_;   // �ӽڵ��б� (XmlNode*)[]
    XmlNodeProps *props_;       // ����ֵ�б�
    string name_;          // �ڵ�����
    string dataString_;    // �ڵ�����(xmlToStr֮�������)
};

///////////////////////////////////////////////////////////////////////////////
// class XmlNodeProps

struct XmlNodePropItem
{
    string name;
    string value;
};

class XmlNodeProps
{
public:
    XmlNodeProps();
    XmlNodeProps(const XmlNodeProps& src);
    virtual ~XmlNodeProps();

    XmlNodeProps& operator = (const XmlNodeProps& rhs);
    string& operator[](const string& name) { return valueOf(name); }

    bool add(const string& name, const string& value);
    void remove(const string& name);
    void clear();
    int indexOf(const string& name);
    bool propExists(const string& name);
    string& valueOf(const string& name);

    int getCount() const { return items_.getCount(); }
    XmlNodePropItem getItems(int index) const;
    string getPropString() const;
    void setPropString(const string& propString);

private:
    XmlNodePropItem* getItemPtr(int index) { return (XmlNodePropItem*)items_[index]; }
    void parsePropString(const string& propStr);

private:
    PointerList items_;      // (XmlNodePropItem*)[]
};

///////////////////////////////////////////////////////////////////////////////
// class XmlDocument

class XmlDocument
{
public:
    XmlDocument();
    XmlDocument(const XmlDocument& src);
    virtual ~XmlDocument();

    XmlDocument& operator = (const XmlDocument& rhs);

    bool saveToStream(Stream& stream);
    bool loadFromStream(Stream& stream);
    bool saveToString(string& str);
    bool loadFromString(const string& str);
    bool saveToFile(const string& fileName);
    bool loadFromFile(const string& fileName);
    void clear();

    bool getAutoIndent() const { return autoIndent_; }
    int getIndentSpaces() const { return indentSpaces_; }
    string getEncoding() const { return encoding_; }
    XmlNode* getRootNode() { return &rootNode_; }

    void setAutoIndent(bool value) { autoIndent_ = value; }
    void setIndentSpaces(int value) { indentSpaces_ = value; }
    void setEncoding(const string& value) { encoding_ = value; }

private:
    bool autoIndent_;       // �Ƿ��Զ�����
    int indentSpaces_;      // �����ո���
    XmlNode rootNode_;      // ���ڵ�
    string encoding_;  // XML����
};

///////////////////////////////////////////////////////////////////////////////
// class XmlReader

class XmlReader : boost::noncopyable
{
public:
    XmlReader(XmlDocument *owner, Stream *stream);
    virtual ~XmlReader();

    void readHeader(string& version, string& encoding);
    void readRootNode(XmlNode *node);

private:
    XML_TAG_TYPES readXmlData(string& name, string& prop, string& data);
    XML_TAG_TYPES readNode(XmlNode *node);

private:
    XmlDocument *owner_;
    Stream *stream_;
    Buffer buffer_;
    int position_;
};

///////////////////////////////////////////////////////////////////////////////
// class XmlWriter

class XmlWriter : boost::noncopyable
{
public:
    XmlWriter(XmlDocument *owner, Stream *stream);
    virtual ~XmlWriter();

    void writeHeader(const string& version = S_DEF_XML_DOC_VER, const string& encoding = "");
    void writeRootNode(XmlNode *node);

private:
    void flushBuffer();
    void writeLn(const string& str);
    void writeNode(XmlNode *node, int indent);

private:
    XmlDocument *owner_;
    Stream *stream_;
    string buffer_;
};

///////////////////////////////////////////////////////////////////////////////
// class XmlDocParser - XML�ĵ������
//
// ���ʽ���:
//   NamePath: XMLֵ������·��������һϵ��XML�ڵ�����ɣ�������֮���õ��(.)�ָ������
//   һ�����ƿ����ǽڵ����������XML�ĸ��ڵ�����������·���б�ʡ�ԡ�����·�������ִ�Сд��
//   ��ע����ǣ���������֮���õ�ŷָ������Ը������ڲ���Ӧ���е�ţ����������
//   ʾ��: "Database.MainDb.HostName"

class XmlDocParser : boost::noncopyable
{
public:
    XmlDocParser();
    virtual ~XmlDocParser();

    bool loadFromFile(const string& fileName);

    string getString(const string& namePath);
    int getInteger(const string& namePath, int defaultVal = 0);
    double getFloat(const string& namePath, double defaultVal = 0);
    bool getBoolean(const string& namePath, bool defaultVal = 0);

    XmlDocument& getXmlDoc() { return xmlDoc_; }

private:
    void splitNamePath(const string& namePath, StrList& result);
private:
    XmlDocument xmlDoc_;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace utils

} // namespace ise

#endif // _ISE_EXT_UTILS_XML_H_
