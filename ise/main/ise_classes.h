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
// ise_classes.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_CLASSES_H_
#define _ISE_CLASSES_H_

#include "ise/main/ise_options.h"

#ifdef ISE_WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#endif

#ifdef ISE_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/file.h>
#include <iostream>
#include <fstream>
#include <string>
#endif

#include "ise/main/ise_global_defs.h"
#include "ise/main/ise_err_msgs.h"
#include "ise/main/ise_exceptions.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// class declares

class Buffer;
class DateTime;
class Timestamp;
class AutoFinalizer;
class AutoInvoker;
class AutoInvokable;
class Mutex;
class Semaphore;
class Condition;
class SignalMasker;
class AtomicInt;
class AtomicInt64;
class SeqNumberAlloc;
class Stream;
class MemoryStream;
class FileStream;
class PointerList;
class PropertyList;
class Strings;
class StrList;
class Url;
class Packet;
template<typename ObjectType> class CustomObjectList;
template<typename ObjectType> class ObjectList;
template<typename T> class CallbackList;
template<typename T> class BlockingQueue;
class ObjectContext;
class Logger;

///////////////////////////////////////////////////////////////////////////////
// class Buffer - ������

class Buffer
{
public:
    Buffer();
    Buffer(const Buffer& src);
    explicit Buffer(int size);
    Buffer(const void *buffer, int size);
    virtual ~Buffer();

    Buffer& operator = (const Buffer& rhs);
    const char& operator[] (int index) const { return (static_cast<char*>(buffer_))[index]; }
    char& operator[] (int index) { return const_cast<char&>((static_cast<const Buffer&>(*this))[index]); }
    operator char*() const { return static_cast<char*>(buffer_); }
    char* data() const { return static_cast<char*>(buffer_); }
    char* c_str() const;
    void assign(const void *buffer, int size);
    void clear() { setSize(0); }
    void setSize(int size, bool initZero = false);
    int getSize() const { return size_; }
    void ensureSize(int size) { if (getSize() < size) setSize(size); }
    void setPosition(int position);
    int getPosition() const { return position_; }

    bool loadFromStream(Stream& stream);
    bool loadFromFile(const string& fileName);
    bool saveToStream(Stream& stream);
    bool saveToFile(const string& fileName);
private:
    inline void init() { buffer_ = NULL; size_ = 0; position_ = 0; }
    void assign(const Buffer& src);
    void verifyPosition();
protected:
    void *buffer_;
    int size_;
    int position_;
};

///////////////////////////////////////////////////////////////////////////////
// class DateTime - ����ʱ����

class DateTime
{
public:
    DateTime() { time_ = 0; }
    DateTime(const DateTime& src) { time_ = src.time_; }
    explicit DateTime(time_t src) { time_ = src; }
    explicit DateTime(const string& src) { *this = src; }

    static DateTime now();

    DateTime& operator = (const DateTime& rhs) { time_ = rhs.time_; return *this; }
    DateTime& operator = (const time_t rhs) { time_ = rhs; return *this; }
    DateTime& operator = (const Timestamp& rhs);
    DateTime& operator = (const string& dateTimeStr);

    DateTime operator + (const DateTime& rhs) const { return DateTime(time_ + rhs.time_); }
    DateTime operator + (time_t rhs) const { return DateTime(time_ + rhs); }
    DateTime operator - (const DateTime& rhs) const { return DateTime(time_ - rhs.time_); }
    DateTime operator - (time_t rhs) const { return DateTime(time_ - rhs); }

    bool operator == (const DateTime& rhs) const { return time_ == rhs.time_; }
    bool operator != (const DateTime& rhs) const { return time_ != rhs.time_; }
    bool operator > (const DateTime& rhs) const  { return time_ > rhs.time_; }
    bool operator < (const DateTime& rhs) const  { return time_ < rhs.time_; }
    bool operator >= (const DateTime& rhs) const { return time_ >= rhs.time_; }
    bool operator <= (const DateTime& rhs) const { return time_ <= rhs.time_; }

    time_t epochTime() const { return time_; }

    void encodeDateTime(int year, int month, int day,
        int hour = 0, int minute = 0, int second = 0);
    void decodeDateTime(int *year, int *month, int *day,
        int *hour, int *minute, int *second,
        int *weekDay = NULL, int *yearDay = NULL) const;

    string toDateString(const string& dateSep = "-") const;
    string toDateTimeString(const string& dateSep = "-",
        const string& dateTimeSep = " ", const string& timeSep = ":") const;

private:
    time_t time_;     // (��1970-01-01 00:00:00 �����������UTCʱ��)
};

///////////////////////////////////////////////////////////////////////////////
// class Timestamp - ʱ����� (���뾫��)

class Timestamp
{
public:
    typedef INT64 TimeVal;   // UTC time value in millisecond resolution
    typedef INT64 TimeDiff;  // difference between two timestamps in milliseconds

public:
    Timestamp() { value_ = 0; }
    explicit Timestamp(TimeVal value) { value_ = value; }
    Timestamp(const Timestamp& src) { value_ = src.value_; }

    static Timestamp now();

    Timestamp& operator = (const Timestamp& rhs) { value_ = rhs.value_; return *this; }
    Timestamp& operator = (TimeVal rhs) { value_ = rhs; return *this; }
    Timestamp& operator = (const DateTime& rhs) { setEpochTime(rhs.epochTime()); return *this; }

    bool operator == (const Timestamp& rhs) const { return value_ == rhs.value_; }
    bool operator != (const Timestamp& rhs) const { return value_ != rhs.value_; }
    bool operator > (const Timestamp& rhs) const  { return value_ > rhs.value_; }
    bool operator < (const Timestamp& rhs) const  { return value_ < rhs.value_; }
    bool operator >= (const Timestamp& rhs) const { return value_ >= rhs.value_; }
    bool operator <= (const Timestamp& rhs) const { return value_ <= rhs.value_; }

    Timestamp  operator +  (TimeDiff d) const { return Timestamp(value_ + d); }
    Timestamp  operator -  (TimeDiff d) const { return Timestamp(value_ - d); }
    TimeDiff   operator -  (const Timestamp& rhs) const { return value_ - rhs.value_; }
    Timestamp& operator += (TimeDiff d) { value_ += d; return *this; }
    Timestamp& operator -= (TimeDiff d) { value_ -= d; return *this; }

    void update();
    void setEpochTime(time_t value);
    time_t epochTime() const;
    TimeVal epochMilliseconds() const;

    string toString(const string& dateSep = "-", const string& dateTimeSep = " ",
        const string& timeSep = ":", const string& microSecSep = ".") const;

private:
    TimeVal value_;
};

///////////////////////////////////////////////////////////////////////////////
// class AutoFinalizer - ���� RAII ���Զ�������

class AutoFinalizer : boost::noncopyable
{
public:
    AutoFinalizer(const Functor& f) : f_(f) {}
    ~AutoFinalizer() { f_(); }
private:
    Functor f_;
};

///////////////////////////////////////////////////////////////////////////////
// class AutoInvokable/AutoInvoker - �Զ���������/�Զ�������
//
// ˵��:
// 1. ������������ʹ�ã������𵽺� "����ָ��" ���Ƶ����ã�������ջ�����Զ����ٵ����ԣ���ջ
//    ����������������Զ����� AutoInvokable::invokeInitialize() �� invokeFinalize()��
//    �˶���һ��ʹ������Ҫ��Դ�ĶԳ��Բ�������(�������/����)��
// 2. ʹ������̳� AutoInvokable �࣬��д invokeInitialize() �� invokeFinalize()
//    ������������Ҫ���õĵط����� AutoInvoker ��ջ����

class AutoInvokable
{
public:
    friend class AutoInvoker;
    virtual ~AutoInvokable() {}
protected:
    virtual void invokeInitialize() {}
    virtual void invokeFinalize() {}
};

class AutoInvoker : boost::noncopyable
{
public:
    explicit AutoInvoker(AutoInvokable& object)
        { object_ = &object; object_->invokeInitialize(); }

    explicit AutoInvoker(AutoInvokable *object)
        { object_ = object; if (object_) object_->invokeInitialize(); }

    virtual ~AutoInvoker()
        { if (object_) object_->invokeFinalize(); }

private:
    AutoInvokable *object_;
};

///////////////////////////////////////////////////////////////////////////////
// class AutoLocker - �߳��Զ�������
//
// ˵��:
// 1. ��������C++��ջ�����Զ����ٵ����ԣ��ڶ��̻߳����½��оֲ���Χ�ٽ������⣻
// 2. ʹ�÷���: ����Ҫ����ķ�Χ���Ծֲ�������ʽ���������󼴿ɣ�
//
// ʹ�÷���:
//   �����Ѷ���: Mutex mutex_;
//   �Զ������ͽ���:
//   {
//       AutoLocker locker(mutex_);
//       //...
//   }

typedef AutoInvoker AutoLocker;

///////////////////////////////////////////////////////////////////////////////
// class BaseMutex - ����������

class BaseMutex:
    public AutoInvokable,
    boost::noncopyable
{
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
protected:
    virtual void invokeInitialize() { lock(); }
    virtual void invokeFinalize() { unlock(); }
};

///////////////////////////////////////////////////////////////////////////////
// class Mutex - �̻߳�����
//
// ˵��:
// 1. �������ڶ��̻߳������ٽ������⣬���������� lock��unlock��
// 2. �߳�������Ƕ�׵��� lock��Ƕ�׵��ú���������ͬ������ unlock �ſɽ�����

class Mutex : public BaseMutex
{
public:
    Mutex();
    virtual ~Mutex();

    virtual void lock();
    virtual void unlock();

private:
#ifdef ISE_WINDOWS
    CRITICAL_SECTION critiSect_;
#endif
#ifdef ISE_LINUX
    pthread_mutex_t mutex_;
    friend class Condition;
#endif
};

///////////////////////////////////////////////////////////////////////////////
// class Semaphore - �ź�����

class Semaphore : boost::noncopyable
{
public:
    explicit Semaphore(UINT initValue = 0);
    virtual ~Semaphore();

    void increase();
    void wait();
    void reset();

private:
    void doCreateSem();
    void doDestroySem();

private:
#ifdef ISE_WINDOWS
    HANDLE sem_;
    friend class Condition;
#endif
#ifdef ISE_LINUX
    sem_t sem_;
#endif

    UINT initValue_;
};

///////////////////////////////////////////////////////////////////////////////
// class Condition - ����������
//
// ˵��:
// * ����������ʹ�����Ǻ�һ�������� (Condition::Mutex) �����һ��
// * ���� ise::Mutex ��� Windows ʵ�ֲ����˷��ں˶���� CRITICAL_SECTION���ʲ���
//   �� Condition ���ʹ�á�Ϊ��ͳһ������ Linux ƽ̨���� Windows ƽ̨����һ��ʹ��
//   Condition::Mutex �� Condition ���䡣
// * ���� wait() ʱ����ԭ�ӵ� unlock mutex ������ȴ���ִ�����ʱ���Զ����� lock mutex.
// * Ӧ�ò���ѭ����ʽ���� wait()���Է���ٻ��� (spurious wakeup)��
// * ����ʹ�÷�ʽ��:
//
//   class Example
//   {
//   public:
//       Example() : condition_(mutex_) {}
//
//       void addToQueue()
//       {
//           {
//               AutoLocker locker(mutex_);
//               queue_.push_back(...);
//           }
//           condition_.notify();
//       }
//
//       void extractFromQueue()
//       {
//           AutoLocker locker(mutex_);
//           while (queue_.empty())
//               condition_.wait();
//           assert(!queue_.empty());
//
//           int top = queue_.front();
//           queue_.pop_front();
//           ...
//       }
//
//   private:
//       Condition::Mutex mutex_;
//       Condition condition_;
//   };

class Condition : boost::noncopyable
{
public:
    class Mutex : public BaseMutex
    {
    public:
        Mutex();
        virtual ~Mutex();

        virtual void lock();
        virtual void unlock();

    private:
#ifdef ISE_WINDOWS
        HANDLE mutex_;
#endif
#ifdef ISE_LINUX
        ise::Mutex mutex_;
#endif
        friend class Condition;
    };

public:
    Condition(Condition::Mutex& mutex);
    ~Condition();

    void wait();
    void notify();
    void notifyAll();

    Condition::Mutex& getMutex() { return mutex_; }

private:
    Condition::Mutex& mutex_;

#ifdef ISE_WINDOWS
    Semaphore sem_;
    boost::scoped_ptr<AtomicInt> waiters_;
#endif

#ifdef ISE_LINUX
    pthread_cond_t cond_;
#endif
};

///////////////////////////////////////////////////////////////////////////////
// class SignalMasker - �ź�������

#ifdef ISE_LINUX
class SignalMasker : boost::noncopyable
{
public:
    explicit SignalMasker(bool isAutoRestore = false);
    virtual ~SignalMasker();

    // ���� Block/UnBlock ����������źż���
    void setSignals(int sigCount, ...);
    void setSignals(int sigCount, va_list argList);

    // �ڽ��̵�ǰ�����źż������ setSignals ���õ��ź�
    void block();
    // �ڽ��̵�ǰ�����źż��н�� setSignals ���õ��ź�
    void unBlock();

    // �����������źż��ָ�Ϊ Block/UnBlock ֮ǰ��״̬
    void restore();

private:
    int sigProcMask(int how, const sigset_t *newSet, sigset_t *oldSet);

private:
    sigset_t oldSet_;
    sigset_t newSet_;
    bool isBlock_;
    bool isAutoRestore_;
};
#endif

///////////////////////////////////////////////////////////////////////////////
// class AtomicInteger - �ṩԭ�Ӳ�����������

#ifdef ISE_WINDOWS

class AtomicInt : boost::noncopyable
{
public:
    AtomicInt() : value_(0) {}

    LONG get() { return InterlockedCompareExchange(&value_, 0, 0); }
    LONG set(LONG newValue) { return InterlockedExchange(&value_, newValue); }
    LONG getAndAdd(LONG x) { return InterlockedExchangeAdd(&value_, x); }
    LONG addAndGet(LONG x) { return getAndAdd(x) + x; }
    LONG increment() { return InterlockedIncrement(&value_); }
    LONG decrement() { return InterlockedDecrement(&value_); }
    LONG getAndSet(LONG newValue) { return set(newValue); }

private:
    volatile LONG value_;
};

class AtomicInt64 : boost::noncopyable
{
public:
    AtomicInt64() : value_(0) {}

    INT64 get()
    {
        AutoLocker locker(mutex_);
        return value_;
    }
    INT64 set(INT64 newValue)
    {
        AutoLocker locker(mutex_);
        INT64 temp = value_;
        value_ = newValue;
        return temp;
    }
    INT64 getAndAdd(INT64 x)
    {
        AutoLocker locker(mutex_);
        INT64 temp = value_;
        value_ += x;
        return temp;
    }
    INT64 addAndGet(INT64 x)
    {
        AutoLocker locker(mutex_);
        value_ += x;
        return value_;
    }
    INT64 increment()
    {
        AutoLocker locker(mutex_);
        ++value_;
        return value_;
    }
    INT64 decrement()
    {
        AutoLocker locker(mutex_);
        --value_;
        return value_;
    }
    INT64 getAndSet(INT64 newValue)
    {
        return set(newValue);
    }

private:
    volatile INT64 value_;
    Mutex mutex_;
};

#endif

#ifdef ISE_LINUX

template<typename T>
class AtomicInteger : boost::noncopyable
{
public:
    AtomicInteger() : value_(0) {}

    T get() { return __sync_val_compare_and_swap(&value_, 0, 0); }
    T set(T newValue) { return __sync_lock_test_and_set(&value_, newValue); }
    T getAndAdd(T x) { return __sync_fetch_and_add(&value_, x); }
    T addAndGet(T x) { return __sync_add_and_fetch(&value_, x); }
    T increment() { return addAndGet(1); }
    T decrement() { return addAndGet(-1); }
    T getAndSet(T newValue) { return set(newValue); }

private:
    volatile T value_;
};

class AtomicInt : public AtomicInteger<long> {};
class AtomicInt64 : public AtomicInteger<INT64> {};

#endif

///////////////////////////////////////////////////////////////////////////////
// class SeqNumberAlloc - �������кŷ�������
//
// ˵��:
// 1. �������̰߳�ȫ��ʽ����һ�����ϵ������������У��û�����ָ�����е���ʼֵ��
// 2. ����һ���������ݰ���˳��ſ��ƣ�

class SeqNumberAlloc : boost::noncopyable
{
public:
    explicit SeqNumberAlloc(UINT64 startId = 0);

    // ����һ���·����ID
    UINT64 allocId();

private:
    Mutex mutex_;
    UINT64 currentId_;
};

///////////////////////////////////////////////////////////////////////////////
// class Stream - �� ����

enum SEEK_ORIGIN
{
    SO_BEGINNING    = 0,
    SO_CURRENT      = 1,
    SO_END          = 2
};

class Stream
{
public:
    virtual ~Stream() {}

    virtual int read(void *buffer, int count) = 0;
    virtual int write(const void *buffer, int count) = 0;
    virtual INT64 seek(INT64 offset, SEEK_ORIGIN seekOrigin) = 0;

    void readBuffer(void *buffer, int count);
    void writeBuffer(const void *buffer, int count);

    INT64 getPosition() { return seek(0, SO_CURRENT); }
    void setPosition(INT64 pos) { seek(pos, SO_BEGINNING); }

    virtual INT64 getSize();
    virtual void setSize(INT64 size) {}
};

///////////////////////////////////////////////////////////////////////////////
// class MemoryStream - �ڴ�����

class MemoryStream : public Stream
{
public:
    enum { DEFAULT_MEMORY_DELTA = 1024 };    // ȱʡ�ڴ��������� (�ֽ����������� 2 �� N �η�)
    enum { MIN_MEMORY_DELTA = 256 };         // ��С�ڴ���������

public:
    explicit MemoryStream(int memoryDelta = DEFAULT_MEMORY_DELTA);
    MemoryStream(const MemoryStream& src);
    virtual ~MemoryStream();

    MemoryStream& operator = (const MemoryStream& rhs);

    virtual int read(void *buffer, int count);
    virtual int write(const void *buffer, int count);
    virtual INT64 seek(INT64 offset, SEEK_ORIGIN seekOrigin);
    virtual void setSize(INT64 size);
    bool loadFromStream(Stream& stream);
    bool loadFromFile(const string& fileName);
    bool saveToStream(Stream& stream);
    bool saveToFile(const string& fileName);
    void clear();
    char* getMemory() { return memory_; }

private:
    void init();
    void assign(const MemoryStream& src);
    void setMemoryDelta(int newMemoryDelta);
    void setPointer(char *memory, int size);
    void setCapacity(int newCapacity);
    char* realloc(int& newCapacity);

private:
    char *memory_;
    int capacity_;
    int size_;
    int position_;
    int memoryDelta_;
};

///////////////////////////////////////////////////////////////////////////////
// class FileStream - �ļ�����

// �ļ����򿪷�ʽ (UINT openMode)
#ifdef ISE_WINDOWS
enum
{
    FM_CREATE           = 0xFFFF,
    FM_OPEN_READ        = 0x0000,
    FM_OPEN_WRITE       = 0x0001,
    FM_OPEN_READ_WRITE  = 0x0002,

    FM_SHARE_EXCLUSIVE  = 0x0010,
    FM_SHARE_DENY_WRITE = 0x0020,
    FM_SHARE_DENY_NONE  = 0x0040
};
#endif
#ifdef ISE_LINUX
enum
{
    FM_CREATE           = 0xFFFF,
    FM_OPEN_READ        = O_RDONLY,  // 0
    FM_OPEN_WRITE       = O_WRONLY,  // 1
    FM_OPEN_READ_WRITE  = O_RDWR,    // 2

    FM_SHARE_EXCLUSIVE  = 0x0010,
    FM_SHARE_DENY_WRITE = 0x0020,
    FM_SHARE_DENY_NONE  = 0x0030
};
#endif

// ȱʡ�ļ���ȡȨ�� (rights)
#ifdef ISE_WINDOWS
enum { DEFAULT_FILE_ACCESS_RIGHTS = 0 };
#endif
#ifdef ISE_LINUX
enum { DEFAULT_FILE_ACCESS_RIGHTS = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH };
#endif

class FileStream :
    public Stream,
    boost::noncopyable
{
public:
    FileStream();
    FileStream(const string& fileName, UINT openMode, UINT rights = DEFAULT_FILE_ACCESS_RIGHTS);
    virtual ~FileStream();

    bool open(const string& fileName, UINT openMode,
        UINT rights = DEFAULT_FILE_ACCESS_RIGHTS, FileException* exception = NULL);
    void close();

    virtual int read(void *buffer, int count);
    virtual int write(const void *buffer, int count);
    virtual INT64 seek(INT64 offset, SEEK_ORIGIN seekOrigin);
    virtual void setSize(INT64 size);

    string getFileName() const { return fileName_; }
    HANDLE getHandle() const { return handle_; }
    bool isOpen() const;

private:
    void init();
    HANDLE fileCreate(const string& fileName, UINT rights);
    HANDLE fileOpen(const string& fileName, UINT openMode);
    void fileClose(HANDLE handle);
    int fileRead(HANDLE handle, void *buffer, int count);
    int fileWrite(HANDLE handle, const void *buffer, int count);
    INT64 fileSeek(HANDLE handle, INT64 offset, SEEK_ORIGIN seekOrigin);

private:
    string fileName_;
    HANDLE handle_;
};

///////////////////////////////////////////////////////////////////////////////
// class PointerList - �б���
//
// ˵��:
// 1. �����ʵ��ԭ���� Delphi::TList ��ȫ��ͬ��
// 2. ���б����������ŵ�:
//    a. ���з�������ȷ (STL���ޱ�ǿ�����Ի�ɬ)��
//    b. ֧���±������ȡ����Ԫ�� (STL::list��֧��)��
//    c. ֧�ֿ��ٻ�ȡ�б��� (STL::list��֧��)��
//    d. ֧��β��������ɾԪ�أ�
// 3. ���б���������ȱ��:
//    a. ��֧��ͷ�����в��Ŀ�����ɾԪ�أ�
//    b. ֻ֧�ֵ�һ����Ԫ��(Pointer����)��

class PointerList
{
public:
    PointerList();
    virtual ~PointerList();

    int add(POINTER item);
    void insert(int index, POINTER item);
    void del(int index);
    int remove(POINTER item);
    POINTER extract(POINTER item);
    void move(int curIndex, int newIndex);
    void resize(int count);
    void clear();

    POINTER first() const;
    POINTER last() const;
    int indexOf(POINTER item) const;
    int getCount() const;
    bool isEmpty() const { return (getCount() <= 0); }

    PointerList& operator = (const PointerList& rhs);
    const POINTER& operator[] (int index) const;
    POINTER& operator[] (int index);

protected:
    virtual void grow();

    POINTER get(int index) const;
    void put(int index, POINTER item);
    void setCapacity(int newCapacity);
    void setCount(int newCount);

private:
    POINTER *list_;
    int count_;
    int capacity_;
};

///////////////////////////////////////////////////////////////////////////////
// class PropertyList - �����б���
//
// ˵��:
// 1. �����б��е�ÿ����Ŀ��������(Name)������ֵ(Value)��ɡ�
// 2. �����������ظ��������ִ�Сд�������в��ɺ��еȺ�"="������ֵ��Ϊ����ֵ��

class PropertyList
{
public:
    enum { NAME_VALUE_SEP = '=' };        // Name �� Value ֮��ķָ���
    enum { PROP_ITEM_SEP  = ',' };        // ������Ŀ֮��ķָ���
    enum { PROP_ITEM_QUOT = '"' };

    struct PropertyItem
    {
        string name, value;
    public:
        PropertyItem(const PropertyItem& src) :
            name(src.name), value(src.value) {}
        PropertyItem(const string& _name, const string& _value) :
            name(_name), value(_value) {}
    };

public:
    PropertyList();
    PropertyList(const PropertyList& src);
    virtual ~PropertyList();

    void add(const string& name, const string& value);
    bool remove(const string& name);
    void clear();
    int indexOf(const string& name) const;
    bool nameExists(const string& name) const;
    bool getValue(const string& name, string& value) const;
    int getCount() const { return items_.getCount(); }
    bool isEmpty() const { return (getCount() <= 0); }
    const PropertyItem& getItem(int index) const;
    string getPropString() const;
    void setPropString(const string& propString);

    PropertyList& operator = (const PropertyList& rhs);
    string& operator[] (const string& name);

private:
    void assign(const PropertyList& src);
    PropertyItem* find(const string& name);
    static bool isReservedChar(char ch);
    static bool hasReservedChar(const string& str);
    static char* scanStr(char *str, char ch);
    static string makeQuotedStr(const string& str);
    static string extractQuotedStr(char*& strPtr);

private:
    PointerList items_;                        // (PropertyItem* [])
};

///////////////////////////////////////////////////////////////////////////////
// class Strings - �ַ����б������

class Strings
{
private:
    enum STRINGS_DEFINED
    {
        SD_DELIMITER         = 0x0001,
        SD_QUOTE_CHAR        = 0x0002,
        SD_NAME_VALUE_SEP    = 0x0004,
        SD_LINE_BREAK        = 0x0008
    };

public:
    // Calls beginUpdate() and endUpdate() automatically in a scope.
    class AutoUpdater
    {
    private:
        Strings& strings;
    public:
        AutoUpdater(Strings& _strings) : strings(_strings)
            { strings.beginUpdate(); }
        ~AutoUpdater()
            { strings.endUpdate(); }
    };

public:
    Strings();
    virtual ~Strings() {}

    virtual int add(const char* str);
    virtual int add(const char* str, POINTER data);
    virtual void addStrings(const Strings& strings);
    virtual void insert(int index, const char* str) = 0;
    virtual void insert(int index, const char* str, POINTER data);
    virtual void clear() = 0;
    virtual void del(int index) = 0;
    virtual bool equals(const Strings& strings);
    virtual void exchange(int index1, int index2);
    virtual void move(int curIndex, int newIndex);
    virtual bool exists(const char* str) const;
    virtual int indexOf(const char* str) const;
    virtual int indexOfName(const char* name) const;
    virtual int indexOfData(POINTER data) const;

    virtual bool loadFromStream(Stream& stream);
    virtual bool loadFromFile(const char* fileName);
    virtual bool saveToStream(Stream& stream) const;
    virtual bool saveToFile(const char* fileName) const;

    virtual int getCapacity() const { return getCount(); }
    virtual void setCapacity(int value) {}
    virtual int getCount() const = 0;
    bool isEmpty() const { return (getCount() <= 0); }
    char getDelimiter() const;
    void setDelimiter(char value);
    string getLineBreak() const;
    void setLineBreak(const char* value);
    char getQuoteChar() const;
    void setQuoteChar(char value);
    char getNameValueSeparator() const;
    void setNameValueSeparator(char value);
    string combineNameValue(const char* name, const char* value) const;
    string getName(int index) const;
    string getValue(const char* name) const;
    string getValue(int index) const;
    void setValue(const char* name, const char* value);
    void setValue(int index, const char* value);
    virtual POINTER getData(int index) const { return NULL; }
    virtual void setData(int index, POINTER data) {}
    virtual string getText() const;
    virtual void setText(const char* value);
    string getCommaText() const;
    void setCommaText(const char* value);
    string getDelimitedText() const;
    void setDelimitedText(const char* value);
    virtual const string& getString(int index) const = 0;
    virtual void setString(int index, const char* value);

    void beginUpdate();
    void endUpdate();

    Strings& operator = (const Strings& rhs);
    const string& operator[] (int index) const { return getString(index); }

protected:
    virtual void setUpdateState(bool isUpdating) {}
    virtual int compareStrings(const char* str1, const char* str2) const;

protected:
    void init();
    void error(const char* msg, int data) const;
    int getUpdateCount() const { return updateCount_; }
    string extractName(const char* str) const;

private:
    void assign(const Strings& src);

protected:
    UINT defined_;
    char delimiter_;
    string lineBreak_;
    char quoteChar_;
    char nameValueSeparator_;
    int updateCount_;
};

///////////////////////////////////////////////////////////////////////////////
// class StrList - �ַ����б���

class StrList : public Strings
{
public:
    friend int stringListCompareProc(const StrList& stringList, int index1, int index2);

public:
    /// The comparison function prototype that used by Sort().
    typedef int (*StringListCompareProc)(const StrList& stringList, int index1, int index2);

    /// Indicates the response when an application attempts to add a duplicate entry to a list.
    enum DUPLICATE_MODE
    {
        DM_IGNORE,      // Ignore attempts to add duplicate entries (do not add the duplicate).
        DM_ACCEPT,      // Allow the list to contain duplicate entries (add the duplicate).
        DM_ERROR        // Throw an exception when the application tries to add a duplicate.
    };

public:
    StrList();
    StrList(const StrList& src);
    virtual ~StrList();

    virtual int add(const char* str);
    virtual int add(const char* str, POINTER data);
    int add(const string& str) { return add(str.c_str()); }
    virtual void clear();
    virtual void del(int index);
    virtual void exchange(int index1, int index2);
    virtual int indexOf(const char* str) const;
    virtual void insert(int index, const char* str);
    virtual void insert(int index, const char* str, POINTER data);

    virtual int getCapacity() const { return capacity_; }
    virtual void setCapacity(int value);
    virtual int getCount() const { return count_; }
    virtual POINTER getData(int index) const;
    virtual void setData(int index, POINTER data);
    virtual const string& getString(int index) const;
    virtual void setString(int index, const char* value);

    virtual bool find(const char* str, int& index) const;
    virtual void sort();
    virtual void sort(StringListCompareProc compareProc);

    DUPLICATE_MODE getDupMode() const { return dupMode_; }
    void setDupMode(DUPLICATE_MODE value) { dupMode_ = value; }
    bool getSorted() const { return isSorted_; }
    virtual void setSorted(bool value);
    bool getCaseSensitive() const { return isCaseSensitive_; }
    virtual void setCaseSensitive(bool value);

    StrList& operator = (const StrList& rhs);

protected: // override
    virtual void setUpdateState(bool isUpdating);
    virtual int compareStrings(const char* str1, const char* str2) const;

protected: // virtual
    /// Occurs immediately before the list of strings changes.
    virtual void onChanging() {}
    /// Occurs immediately after the list of strings changes.
    virtual void onChanged() {}
    /// Internal method used to insert a string to the list.
    virtual void insertItem(int index, const char* str, POINTER data);

private:
    void init();
    void assign(const StrList& src);
    void internalClear();
    string& stringObjectNeeded(int index) const;
    void exchangeItems(int index1, int index2);
    void grow();
    void quickSort(int l, int r, StringListCompareProc compareProc);

private:
    struct StringItem
    {
        string *str;
        POINTER data;
    };

private:
    StringItem *list_;
    int count_;
    int capacity_;
    DUPLICATE_MODE dupMode_;
    bool isSorted_;
    bool isCaseSensitive_;
};

///////////////////////////////////////////////////////////////////////////////
// class Url - URL������

class Url
{
public:
    // The URL parts.
    enum URL_PART
    {
        URL_PROTOCOL  = 0x0001,
        URL_HOST      = 0x0002,
        URL_PORT      = 0x0004,
        URL_PATH      = 0x0008,
        URL_FILENAME  = 0x0010,
        URL_BOOKMARK  = 0x0020,
        URL_USERNAME  = 0x0040,
        URL_PASSWORD  = 0x0080,
        URL_PARAMS    = 0x0100,
        URL_ALL       = 0xFFFF,
    };

public:
    Url(const string& url = "");
    Url(const Url& src);
    virtual ~Url() {}

    void clear();
    Url& operator = (const Url& rhs);

    string getUrl() const;
    string getUrl(UINT parts);
    void setUrl(const string& value);

    const string& getProtocol() const { return protocol_; }
    const string& getHost() const { return host_; }
    const string& getPort() const { return port_; }
    const string& getPath() const { return path_; }
    const string& getFileName() const { return fileName_; }
    const string& getBookmark() const { return bookmark_; }
    const string& getUserName() const { return userName_; }
    const string& getPassword() const { return password_; }
    const string& getParams() const { return params_; }

    void setProtocol(const string& value) { protocol_ = value; }
    void setHost(const string& value) { host_ = value; }
    void setPort(const string& value) { port_ = value; }
    void setPath(const string& value) { path_ = value; }
    void setFileName(const string& value) { fileName_ = value; }
    void setBookmark(const string& value) { bookmark_ = value; }
    void setUserName(const string& value) { userName_ = value; }
    void setPassword(const string& value) { password_ = value; }
    void setParams(const string& value) { params_ = value; }

private:
    string protocol_;
    string host_;
    string port_;
    string path_;
    string fileName_;
    string bookmark_;
    string userName_;
    string password_;
    string params_;
};

///////////////////////////////////////////////////////////////////////////////
// class Packet - ���ݰ�����

class Packet
{
public:
    // ȱʡ�ڴ��������� (�ֽ����������� 2 �� N �η�)
    enum { DEFAULT_MEMORY_DELTA = 1024 };

public:
    Packet();
    virtual ~Packet();

    bool pack();
    bool unpack(void *buffer, int bytes);
    bool unpack(const Buffer& buffer);
    void clear();
    void ensurePacked();

    char* getBuffer() const { return (stream_? static_cast<char*>(stream_->getMemory()) : NULL); }
    int getSize() const { return (stream_? static_cast<int>(stream_->getSize()) : 0); }
    bool isAvailable() const { return isAvailable_; }
    bool IsPacked() const { return isPacked_; }

protected:
    void throwUnpackError();
    void throwPackError();
    void checkUnsafeSize(int value);

    void readBuffer(void *buffer, int bytes);
    void readINT8(INT8& value) { readBuffer(&value, sizeof(INT8)); }
    void readINT16(INT16& value) { readBuffer(&value, sizeof(INT16)); }
    void readINT32(INT32& value) { readBuffer(&value, sizeof(INT32)); }
    void readINT64(INT64& value) { readBuffer(&value, sizeof(INT64)); }
    void readString(string& str);
    void readBlob(string& str);
    void readBlob(Stream& stream);
    void readBlob(Buffer& buffer);
    INT8 readINT8() { INT8 v; readINT8(v); return v; }
    INT16 readINT16() { INT16 v; readINT16(v); return v; }
    INT32 readINT32() { INT32 v; readINT32(v); return v; }
    INT64 readINT64() { INT64 v; readINT64(v); return v; }
    bool readBool() { INT8 v; readINT8(v); return (v? true : false); }
    string readString() { string v; readString(v); return v; }

    void writeBuffer(const void *buffer, int bytes);
    void writeINT8(const INT8& value) { writeBuffer(&value, sizeof(INT8)); }
    void writeINT16(const INT16& value) { writeBuffer(&value, sizeof(INT16)); }
    void writeINT32(const INT32& value) { writeBuffer(&value, sizeof(INT32)); }
    void writeINT64(const INT64& value) { writeBuffer(&value, sizeof(INT64)); }
    void writeBool(bool value) { writeINT8(value ? 1 : 0); }
    void writeString(const string& str);
    void writeBlob(void *buffer, int bytes);
    void writeBlob(const Buffer& buffer);

    void fixStrLength(string& str, int length);
    void truncString(string& str, int maxLength);

protected:
    virtual void doPack() {}
    virtual void doUnpack() {}
    virtual void doAfterPack() {}
    virtual void doEncrypt() {}
    virtual void doDecrypt() {}
    virtual void doCompress() {}
    virtual void doDecompress() {}

private:
    void init();

protected:
    MemoryStream *stream_;
    bool isAvailable_;
    bool isPacked_;
};

///////////////////////////////////////////////////////////////////////////////
// class CustomObjectList - �����б����

template<typename ObjectType>
class CustomObjectList
{
public:
    class InternalMutex : public BaseMutex
    {
    public:
        InternalMutex(bool active) : mutex_(NULL) { if (active) mutex_ = new Mutex(); }
        virtual ~InternalMutex() { delete mutex_; }
    public:
        virtual void lock() { if (mutex_) mutex_->lock(); }
        virtual void unlock() { if (mutex_) mutex_->unlock(); }
    private:
        Mutex *mutex_;
    };

    typedef ObjectType* ObjectPtr;

public:
    CustomObjectList() :
        mutex_(false), isOwnsObjects_(true) {}

    CustomObjectList(bool isThreadSafe, bool isOwnsObjects) :
        mutex_(isThreadSafe), isOwnsObjects_(isOwnsObjects) {}

    virtual ~CustomObjectList() { clear(); }

protected:
    virtual void notifyDelete(int index)
    {
        if (isOwnsObjects_)
        {
            ObjectPtr item = static_cast<ObjectPtr>(items_[index]);
            items_[index] = NULL;
            delete item;
        }
    }

protected:
    int add(ObjectPtr item, bool allowDuplicate = true)
    {
        ISE_ASSERT(item);
        AutoLocker locker(mutex_);

        if (allowDuplicate || items_.indexOf(item) == -1)
            return items_.add(item);
        else
            return -1;
    }

    int remove(ObjectPtr item)
    {
        AutoLocker locker(mutex_);

        int result = items_.indexOf(item);
        if (result >= 0)
        {
            notifyDelete(result);
            items_.del(result);
        }
        return result;
    }

    ObjectPtr extract(ObjectPtr item)
    {
        AutoLocker locker(mutex_);

        ObjectPtr result = NULL;
        int i = items_.remove(item);
        if (i >= 0)
            result = item;
        return result;
    }

    ObjectPtr extract(int index)
    {
        AutoLocker locker(mutex_);

        ObjectPtr result = NULL;
        if (index >= 0 && index < items_.getCount())
        {
            result = static_cast<ObjectPtr>(items_[index]);
            items_.del(index);
        }
        return result;
    }

    void del(int index)
    {
        AutoLocker locker(mutex_);

        if (index >= 0 && index < items_.getCount())
        {
            notifyDelete(index);
            items_.del(index);
        }
    }

    void insert(int index, ObjectPtr item)
    {
        ISE_ASSERT(item);
        AutoLocker locker(mutex_);
        items_.insert(index, item);
    }

    int indexOf(ObjectPtr item) const
    {
        AutoLocker locker(mutex_);
        return items_.indexOf(item);
    }

    bool exists(ObjectPtr item) const
    {
        AutoLocker locker(mutex_);
        return items_.indexOf(item) >= 0;
    }

    ObjectPtr first() const
    {
        AutoLocker locker(mutex_);
        return static_cast<ObjectPtr>(items_.first());
    }

    ObjectPtr last() const
    {
        AutoLocker locker(mutex_);
        return static_cast<ObjectPtr>(items_.last());
    }

    void clear()
    {
        AutoLocker locker(mutex_);

        for (int i = items_.getCount() - 1; i >= 0; i--)
            notifyDelete(i);
        items_.clear();
    }

    void freeObjects()
    {
        AutoLocker locker(mutex_);

        for (int i = items_.getCount() - 1; i >= 0; i--)
        {
            ObjectPtr item = static_cast<ObjectPtr>(items_[i]);
            items_[i] = NULL;
            delete item;
        }
    }

    int getCount() const { return items_.getCount(); }
    ObjectPtr& getItem(int index) const { return (ObjectPtr&)items_[index]; }
    CustomObjectList& operator = (const CustomObjectList& rhs) { items_ = rhs.items_; return *this; }
    ObjectPtr& operator[] (int index) const { return getItem(index); }
    bool isEmpty() const { return (getCount() <= 0); }
    void setOwnsObjects(bool value) { isOwnsObjects_ = value; }

protected:
    PointerList items_;       // �����б�
    mutable InternalMutex mutex_;
    bool isOwnsObjects_;      // Ԫ�ر�ɾ��ʱ���Ƿ��Զ��ͷ�Ԫ�ض���
};

///////////////////////////////////////////////////////////////////////////////
// class ObjectList - �����б���

template<typename ObjectType>
class ObjectList : public CustomObjectList<ObjectType>
{
public:
    ObjectList() :
        CustomObjectList<ObjectType>(false, true) {}
    ObjectList(bool isThreadSafe, bool isOwnsObjects) :
        CustomObjectList<ObjectType>(isThreadSafe, isOwnsObjects) {}
    virtual ~ObjectList() {}

    using CustomObjectList<ObjectType>::add;
    using CustomObjectList<ObjectType>::remove;
    using CustomObjectList<ObjectType>::extract;
    using CustomObjectList<ObjectType>::del;
    using CustomObjectList<ObjectType>::insert;
    using CustomObjectList<ObjectType>::indexOf;
    using CustomObjectList<ObjectType>::exists;
    using CustomObjectList<ObjectType>::first;
    using CustomObjectList<ObjectType>::last;
    using CustomObjectList<ObjectType>::clear;
    using CustomObjectList<ObjectType>::freeObjects;
    using CustomObjectList<ObjectType>::getCount;
    using CustomObjectList<ObjectType>::getItem;
    using CustomObjectList<ObjectType>::operator=;
    using CustomObjectList<ObjectType>::operator[];
    using CustomObjectList<ObjectType>::isEmpty;
    using CustomObjectList<ObjectType>::setOwnsObjects;
};

///////////////////////////////////////////////////////////////////////////////
// class CallbackList - �ص��б�

template<typename T>
class CallbackList
{
public:
    void registerCallback(const T& callback)
    {
        AutoLocker locker(mutex_);
        if (callback)
            items_.push_back(callback);
    }

    int getCount() const { return static_cast<int>(items_.size()); }
    const T& getItem(int index) const { return items_[index]; }

private:
    std::vector<T> items_;
    Mutex mutex_;
};

///////////////////////////////////////////////////////////////////////////////
// class BlockingQueue - ����������

template<typename T>
class BlockingQueue : boost::noncopyable
{
public:
    BlockingQueue() : condition_(mutex_) {}

    void put(const T& item)
    {
        AutoLocker locker(mutex_);
        queue_.push_back(item);
        condition_.notify();
    }

    T take()
    {
        AutoLocker locker(mutex_);
        while (queue_.empty())
            condition_.wait();
        ISE_ASSERT(!queue_.empty());

        T item(queue_.front());
        queue_.pop_front();
        return item;
    }

    size_t getCount() const
    {
        AutoLocker locker(mutex_);
        return queue_.size();
    }

private:
    Condition::Mutex mutex_;
    Condition condition_;
    std::deque<T> queue_;
};

///////////////////////////////////////////////////////////////////////////////
// class ObjectContext - �Ӵ���̳и��������������

class ObjectContext
{
public:
    void setContext(const boost::any& value) { context_ = value; }
    const boost::any& getContext() const { return context_; }
    boost::any& getContext() { return context_; }
private:
    boost::any context_;
};

///////////////////////////////////////////////////////////////////////////////
// class Singleton - ȫ�ֵ�����

#ifdef ISE_WINDOWS

template<typename T>
class Singleton : boost::noncopyable
{
public:
    static T& instance()
    {
        // DCL with volatile
        if (instance_ == NULL)
        {
            AutoLocker locker(mutex_);
            if (instance_ == NULL)
                instance_ = new T();
            return *instance_;
        }
        return *instance_;
    }
protected:
    Singleton() {}
    ~Singleton() {}
private:
    static T* volatile instance_;
    static Mutex mutex_;
};

template<typename T> T* volatile Singleton<T>::instance_ = NULL;
template<typename T> Mutex Singleton<T>::mutex_;

#endif
#ifdef ISE_LINUX

template<typename T>
class Singleton : boost::noncopyable
{
public:
    static T& instance()
    {
        pthread_once(&once_, &Singleton::init);
        return *instance_;
    }
protected:
    Singleton() {}
    ~Singleton() {}
private:
    static void init()
    {
        instance_ = new T();
    }
private:
    static T* instance_;
    static pthread_once_t once_;
};

template<typename T> T* Singleton<T>::instance_ = NULL;
template<typename T> pthread_once_t Singleton<T>::once_ = PTHREAD_ONCE_INIT;

#endif

///////////////////////////////////////////////////////////////////////////////
// class Logger - ��־��

class Logger : public Singleton<Logger>
{
public:
    void setFileName(const string& fileName, bool isNewFileDaily = false);

    void writeStr(const char *str);
    void writeStr(const string& str) { writeStr(str.c_str()); }
    void writeFmt(const char *format, ...);
    void writeException(const Exception& e);

private:
    Logger();

private:
    string getLogFileName();
    bool openFile(FileStream& fileStream, const string& fileName);
    void writeToFile(const string& str);

private:
    string fileName_;       // ��־�ļ���
    bool isNewFileDaily_;   // �Ƿ�ÿ����һ���������ļ��洢��־
    Mutex mutex_;

    friend class Singleton<Logger>;
};

///////////////////////////////////////////////////////////////////////////////
// ȫ�ֺ���

inline Logger& logger() { return Logger::instance(); }

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_CLASSES_H_
