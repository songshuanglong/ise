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
// �ļ�����: ise_classes.cpp
// ��������: ͨ�û������
///////////////////////////////////////////////////////////////////////////////

#include "ise/main/ise_classes.h"
#include "ise/main/ise_sys_utils.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ���Ͷ���

union Int64Rec
{
    INT64 value;
    struct {
        INT32 lo;
        INT32 hi;
    } ints;
};

///////////////////////////////////////////////////////////////////////////////
// class Buffer

Buffer::Buffer()
{
    init();
}

//-----------------------------------------------------------------------------

Buffer::Buffer(const Buffer& src)
{
    init();
    assign(src);
}

//-----------------------------------------------------------------------------

Buffer::Buffer(int size)
{
    init();
    setSize(size);
}

//-----------------------------------------------------------------------------

Buffer::Buffer(const void *buffer, int size)
{
    init();
    assign(buffer, size);
}

//-----------------------------------------------------------------------------

Buffer::~Buffer()
{
    if (buffer_)
        free(buffer_);
}

//-----------------------------------------------------------------------------

Buffer& Buffer::operator = (const Buffer& rhs)
{
    if (this != &rhs)
        assign(rhs);
    return *this;
}

//-----------------------------------------------------------------------------
// ����: �� buffer �е� size ���ֽڸ��Ƶ� *this �У�������С����Ϊ size
//-----------------------------------------------------------------------------
void Buffer::assign(const void *buffer, int size)
{
    setSize(size);
    if (size_ > 0)
        memmove(buffer_, buffer, size_);
    verifyPosition();
}

//-----------------------------------------------------------------------------
// ����: ���û����С
// ����:
//   size     - �»�������С
//   initZero - ���»������Ⱦɻ��������Ƿ񽫶���Ŀռ���'\0'���
// ��ע:
//   �µĻ���ᱣ��ԭ������
//-----------------------------------------------------------------------------
void Buffer::setSize(int size, bool initZero)
{
    if (size <= 0)
    {
        if (buffer_) free(buffer_);
        buffer_ = NULL;
        size_ = 0;
        position_ = 0;
    }
    else if (size != size_)
    {
        void *newBuf;

        // ��� buffer_ == NULL���� realloc �൱�� malloc��
        newBuf = realloc(buffer_, size + 1);  // �����һ���ֽ����� c_str()!

        if (newBuf)
        {
            if (initZero && (size > size_))
                memset(((char*)newBuf) + size_, 0, size - size_);
            buffer_ = newBuf;
            size_ = size;
            verifyPosition();
        }
        else
        {
            iseThrowMemoryException();
        }
    }
}

//-----------------------------------------------------------------------------
// ����: ���� Position
//-----------------------------------------------------------------------------
void Buffer::setPosition(int position)
{
    position_ = position;
    verifyPosition();
}

//-----------------------------------------------------------------------------
// ����: ���� C �����ַ��� (ĩ�˸��ӽ����� '\0')
//-----------------------------------------------------------------------------
char* Buffer::c_str() const
{
    if (size_ <= 0 || !buffer_)
        return (char*)"";
    else
    {
        ((char*)buffer_)[size_] = 0;
        return (char*)buffer_;
    }
}

//-----------------------------------------------------------------------------
// ����: �����������뵽������
//-----------------------------------------------------------------------------
bool Buffer::loadFromStream(Stream& stream)
{
    try
    {
        INT64 size64 = stream.getSize() - stream.getPosition();
        ISE_ASSERT(size64 <= MAXINT);
        int size = (int)size64;

        setPosition(0);
        setSize(size);
        stream.readBuffer(data(), size);
        return true;
    }
    catch (Exception&)
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
// ����: ���ļ����뵽������
//-----------------------------------------------------------------------------
bool Buffer::loadFromFile(const string& fileName)
{
    FileStream fs;
    bool result = fs.open(fileName, FM_OPEN_READ | FM_SHARE_DENY_WRITE);
    if (result)
        result = loadFromStream(fs);
    return result;
}

//-----------------------------------------------------------------------------
// ����: �����汣�浽��������
//-----------------------------------------------------------------------------
bool Buffer::saveToStream(Stream& stream)
{
    try
    {
        if (getSize() > 0)
            stream.writeBuffer(data(), getSize());
        return true;
    }
    catch (Exception&)
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
// ����: �����汣�浽�ļ���
//-----------------------------------------------------------------------------
bool Buffer::saveToFile(const string& fileName)
{
    FileStream fs;
    bool result = fs.open(fileName, FM_CREATE);
    if (result)
        result = saveToStream(fs);
    return result;
}

//-----------------------------------------------------------------------------

void Buffer::assign(const Buffer& src)
{
    setSize(src.getSize());
    setPosition(src.getPosition());
    if (size_ > 0)
        memmove(buffer_, src.buffer_, size_);
}

//-----------------------------------------------------------------------------

void Buffer::verifyPosition()
{
    if (position_ < 0) position_ = 0;
    if (position_ > size_) position_ = size_;
}

///////////////////////////////////////////////////////////////////////////////
// class DateTime

//-----------------------------------------------------------------------------
// ����: ���ص�ǰʱ�� (��1970-01-01 00:00:00 ���������, UTCʱ��)
//-----------------------------------------------------------------------------
DateTime DateTime::now()
{
    return DateTime(time(NULL));
}

//-----------------------------------------------------------------------------
// ����: �� Timestamp ת���� DateTime
//-----------------------------------------------------------------------------
DateTime& DateTime::operator = (const Timestamp& rhs)
{
    time_ = rhs.epochTime();
    return *this;
}

//-----------------------------------------------------------------------------
// ����: ���ַ���ת���� DateTime
// ע��: dateTimeStr �ĸ�ʽ����Ϊ YYYY-MM-DD HH:MM:SS
//-----------------------------------------------------------------------------
DateTime& DateTime::operator = (const string& dateTimeStr)
{
    int year, month, day, hour, minute, second;

    if (dateTimeStr.length() == 19)
    {
        year = strToInt(dateTimeStr.substr(0, 4), 0);
        month = strToInt(dateTimeStr.substr(5, 2), 0);
        day = strToInt(dateTimeStr.substr(8, 2), 0);
        hour = strToInt(dateTimeStr.substr(11, 2), 0);
        minute = strToInt(dateTimeStr.substr(14, 2), 0);
        second = strToInt(dateTimeStr.substr(17, 2), 0);

        encodeDateTime(year, month, day, hour, minute, second);
        return *this;
    }
    else
    {
        iseThrowException(SEM_INVALID_DATETIME_STR);
        return *this;
    }
}

//-----------------------------------------------------------------------------
// ����: ����ʱ����룬������ *this
//-----------------------------------------------------------------------------
void DateTime::encodeDateTime(int year, int month, int day,
    int hour, int minute, int second)
{
    struct tm tm;

    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;

    time_ = mktime(&tm);
}

//-----------------------------------------------------------------------------
// ����: ����ʱ����룬�����������
//-----------------------------------------------------------------------------
void DateTime::decodeDateTime(int *year, int *month, int *day,
    int *hour, int *minute, int *second, int *weekDay, int *yearDay) const
{
    struct tm tm;

#ifdef ISE_WINDOWS
#ifdef ISE_COMPILER_VC
#if (_MSC_VER >= 1400)  // >= VC8
    localtime_s(&tm, &time_);
#else
    struct tm *ptm = localtime(&time_);    // ��������������
    if (ptm) tm = *ptm;
#endif
#else
    struct tm *ptm = localtime(&time_);    // ��������������
    if (ptm) tm = *ptm;
#endif
#endif

#ifdef ISE_LINUX
    localtime_r(&time_, &tm);
#endif

    if (year) *year = tm.tm_year + 1900;
    if (month) *month = tm.tm_mon + 1;
    if (day) *day = tm.tm_mday;
    if (hour) *hour = tm.tm_hour;
    if (minute) *minute = tm.tm_min;
    if (second) *second = tm.tm_sec;
    if (weekDay) *weekDay = tm.tm_wday;
    if (yearDay) *yearDay = tm.tm_yday;
}

//-----------------------------------------------------------------------------
// ����: ���������ַ���
// ����:
//   dateSep - ���ڷָ���
// ��ʽ:
//   YYYY-MM-DD
//-----------------------------------------------------------------------------
string DateTime::toDateString(const string& dateSep) const
{
    string result;
    int year, month, day;

    decodeDateTime(&year, &month, &day, NULL, NULL, NULL, NULL);
    result = formatString("%04d%s%02d%s%02d",
        year, dateSep.c_str(), month, dateSep.c_str(), day);

    return result;
}

//-----------------------------------------------------------------------------
// ����: ��������ʱ���ַ���
// ����:
//   dateSep     - ���ڷָ���
//   dateTimeSep - ���ں�ʱ��֮��ķָ���
//   timeSep     - ʱ��ָ���
// ��ʽ:
//   YYYY-MM-DD HH:MM:SS
//-----------------------------------------------------------------------------
string DateTime::toDateTimeString(const string& dateSep,
    const string& dateTimeSep, const string& timeSep) const
{
    string result;
    int year, month, day, hour, minute, second;

    decodeDateTime(&year, &month, &day, &hour, &minute, &second, NULL);
    result = formatString("%04d%s%02d%s%02d%s%02d%s%02d%s%02d",
        year, dateSep.c_str(), month, dateSep.c_str(), day,
        dateTimeSep.c_str(),
        hour, timeSep.c_str(), minute, timeSep.c_str(), second);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// class Timestamp

//-----------------------------------------------------------------------------
// ����: ���ص�ǰʱ���
//-----------------------------------------------------------------------------
Timestamp Timestamp::now()
{
    Timestamp result;

#ifdef ISE_WINDOWS
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER epoch; // UNIX epoch (1970-01-01 00:00:00) expressed in Windows NT FILETIME
    epoch.LowPart  = 0xD53E8000;
    epoch.HighPart = 0x019DB1DE;

    ULARGE_INTEGER ts;
    ts.LowPart  = ft.dwLowDateTime;
    ts.HighPart = ft.dwHighDateTime;
    ts.QuadPart -= epoch.QuadPart;
    result.value_ = ts.QuadPart/10000;
#endif
#ifdef ISE_LINUX
    struct timeval tv;
    gettimeofday(&tv, NULL);
    result.value_ = TimeVal(tv.tv_sec) * MILLISECS_PER_SECOND + tv.tv_usec;
#endif

    return result;
}

//-----------------------------------------------------------------------------
// ����: ����Ϊ��ǰʱ��
//-----------------------------------------------------------------------------
void Timestamp::update()
{
    value_ = Timestamp::now().value_;
}

//-----------------------------------------------------------------------------
// ����: �� time_t ����ʱ���
//-----------------------------------------------------------------------------
void Timestamp::setEpochTime(time_t value)
{
    value_ = value * MILLISECS_PER_SECOND;
}

//-----------------------------------------------------------------------------
// ����: �� time_t ��ʽ���� (since 1970-01-01 00:00:00, in seconds)
//-----------------------------------------------------------------------------
time_t Timestamp::epochTime() const
{
    return (time_t)(value_ / MILLISECS_PER_SECOND);
}

//-----------------------------------------------------------------------------
// ����: �Ժ��뾫�ȷ��� epoch ʱ�� (since 1970-01-01 00:00:00)
//-----------------------------------------------------------------------------
Timestamp::TimeVal Timestamp::epochMilliseconds() const
{
    return value_;
}

//-----------------------------------------------------------------------------
// ����: ת�����ַ���
//-----------------------------------------------------------------------------
string Timestamp::toString(const string& dateSep, const string& dateTimeSep,
    const string& timeSep, const string& microSecSep) const
{
    time_t seconds = static_cast<time_t>(value_ / MILLISECS_PER_SECOND);
    int milliseconds = static_cast<int>(value_ % MILLISECS_PER_SECOND);

    string result = DateTime(seconds).toDateTimeString(dateSep, dateTimeSep, timeSep);
    result += formatString("%s%03d", microSecSep.c_str(), milliseconds);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// class Mutex

Mutex::Mutex()
{
#ifdef ISE_WINDOWS
    InitializeCriticalSection(&critiSect_);
#endif
#ifdef ISE_LINUX
    pthread_mutexattr_t attr;

    // ������˵��:
    // PTHREAD_MUTEX_TIMED_NP:
    //   ��ͨ����ͬһ�߳��ڱ���ɶԵ��� Lock �� Unlock�������������ö�� Lock�������������
    // PTHREAD_MUTEX_RECURSIVE_NP:
    //   Ƕ�������߳��ڿ���Ƕ�׵��� Lock����һ����Ч��֮����������ͬ������ Unlock ���ɽ�����
    // PTHREAD_MUTEX_ERRORCHECK_NP:
    //   ����������ͬһ�߳�Ƕ�׵��� Lock ���������
    // PTHREAD_MUTEX_ADAPTIVE_NP:
    //   ��Ӧ����
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&mutex_, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
}

//-----------------------------------------------------------------------------

Mutex::~Mutex()
{
#ifdef ISE_WINDOWS
    DeleteCriticalSection(&critiSect_);
#endif
#ifdef ISE_LINUX
    // �����δ����������� destroy���˺����᷵�ش��� EBUSY��
    // �� linux �£���ʹ�˺������ش���Ҳ��������Դй©��
    pthread_mutex_destroy(&mutex_);
#endif
}

//-----------------------------------------------------------------------------
// ����: ����
//-----------------------------------------------------------------------------
void Mutex::lock()
{
#ifdef ISE_WINDOWS
    EnterCriticalSection(&critiSect_);
#endif
#ifdef ISE_LINUX
    pthread_mutex_lock(&mutex_);
#endif
}

//-----------------------------------------------------------------------------
// ����: ����
//-----------------------------------------------------------------------------
void Mutex::unlock()
{
#ifdef ISE_WINDOWS
    LeaveCriticalSection(&critiSect_);
#endif
#ifdef ISE_LINUX
    pthread_mutex_unlock(&mutex_);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// class Semaphore

Semaphore::Semaphore(UINT initValue)
{
    initValue_ = initValue;
    doCreateSem();
}

//-----------------------------------------------------------------------------

Semaphore::~Semaphore()
{
    doDestroySem();
}

//-----------------------------------------------------------------------------
// ����: �����ֵԭ�ӵؼ�1����ʾ����һ���ɷ��ʵ���Դ
//-----------------------------------------------------------------------------
void Semaphore::increase()
{
#ifdef ISE_WINDOWS
    ReleaseSemaphore(sem_, 1, NULL);
#endif
#ifdef ISE_LINUX
    sem_post(&sem_);
#endif
}

//-----------------------------------------------------------------------------
// ����: �ȴ����(ֱ�����ֵ����0)��Ȼ�����ֵԭ�ӵؼ�1
//-----------------------------------------------------------------------------
void Semaphore::wait()
{
#ifdef ISE_WINDOWS
    WaitForSingleObject(sem_, INFINITE);
#endif
#ifdef ISE_LINUX
    int ret;
    do
    {
        ret = sem_wait(&sem_);
    }
    while (ret == -1 && (errno == EINTR || errno == EAGAIN));
#endif
}

//-----------------------------------------------------------------------------
// ����: ��������ã������ֵ��Ϊ��ʼ״̬
//-----------------------------------------------------------------------------
void Semaphore::reset()
{
    doDestroySem();
    doCreateSem();
}

//-----------------------------------------------------------------------------

void Semaphore::doCreateSem()
{
#ifdef ISE_WINDOWS
    sem_ = CreateSemaphore(NULL, initValue_, 0x7FFFFFFF, NULL);
    if (!sem_)
        iseThrowException(SEM_SEM_INIT_ERROR);
#endif
#ifdef ISE_LINUX
    if (sem_init(&sem_, 0, initValue_) != 0)
        iseThrowException(SEM_SEM_INIT_ERROR);
#endif
}

//-----------------------------------------------------------------------------

void Semaphore::doDestroySem()
{
#ifdef ISE_WINDOWS
    CloseHandle(sem_);
#endif
#ifdef ISE_LINUX
    sem_destroy(&sem_);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// class Condition::Mutex

Condition::Mutex::Mutex()
{
#ifdef ISE_WINDOWS
    mutex_ = ::CreateMutexA(NULL, FALSE, NULL);
    if (mutex_ == NULL)
        iseThrowException(SEM_FAIL_TO_CREATE_MUTEX);
#endif
}

Condition::Mutex::~Mutex()
{
#ifdef ISE_WINDOWS
    ::CloseHandle(mutex_);
#endif
}

void Condition::Mutex::lock()
{
#ifdef ISE_WINDOWS
    ::WaitForSingleObject(mutex_, INFINITE);
#endif
#ifdef ISE_LINUX
    mutex_.lock();
#endif
}

void Condition::Mutex::unlock()
{
#ifdef ISE_WINDOWS
    ::ReleaseMutex(mutex_);
#endif
#ifdef ISE_LINUX
    mutex_.unlock();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// class Condition

#ifdef ISE_WINDOWS

Condition::Condition(Condition::Mutex& mutex) :
    mutex_(mutex),
    waiters_(new AtomicInt())
{
    // nothing
}

Condition::~Condition()
{
    // nothing
}

void Condition::wait()
{
    waiters_->increment();
    ::SignalObjectAndWait(mutex_.mutex_, sem_.sem_, INFINITE, FALSE);
    mutex_.lock();
    waiters_->decrement();
}

void Condition::notify()
{
    if (waiters_->get() > 0)
        sem_.increase();
}

void Condition::notifyAll()
{
    int count = (int)waiters_->get();
    for (int i = 0; i < count; ++i)
        sem_.increase();
}

#endif

//-----------------------------------------------------------------------------

#ifdef ISE_LINUX

Condition::Condition(Condition::Mutex& mutex) :
    mutex_(mutex)
{
    if (pthread_cond_init(&cond_, NULL) != 0)
        iseThrowException(SEM_COND_INIT_ERROR);
}

Condition::~Condition()
{
    pthread_cond_destroy(&cond_);
}

void Condition::wait()
{
    pthread_cond_wait(&cond_, &mutex_.mutex_.mutex_);
}

void Condition::notify()
{
    pthread_cond_signal(&cond_);
}

void Condition::notifyAll()
{
    pthread_cond_broadcast(&cond_);
}

#endif

///////////////////////////////////////////////////////////////////////////////
// class SignalMasker

#ifdef ISE_LINUX

SignalMasker::SignalMasker(bool isAutoRestore) :
    isBlock_(false),
    isAutoRestore_(isAutoRestore)
{
    sigemptyset(&oldSet_);
    sigemptyset(&newSet_);
}

//-----------------------------------------------------------------------------

SignalMasker::~SignalMasker()
{
    if (isAutoRestore_) restore();
}

//-----------------------------------------------------------------------------
// ����: ���� Block/UnBlock ����������źż���
//-----------------------------------------------------------------------------
void SignalMasker::setSignals(int sigCount, va_list argList)
{
    sigemptyset(&newSet_);
    for (int i = 0; i < sigCount; i++)
        sigaddset(&newSet_, va_arg(argList, int));
}

//-----------------------------------------------------------------------------

void SignalMasker::setSignals(int sigCount, ...)
{
    va_list argList;
    va_start(argList, sigCount);
    setSignals(sigCount, argList);
    va_end(argList);
}

//-----------------------------------------------------------------------------
// ����: �ڽ��̵�ǰ�����źż������ setSignals ���õ��ź�
//-----------------------------------------------------------------------------
void SignalMasker::block()
{
    sigProcMask(SIG_BLOCK, &newSet_, &oldSet_);
    isBlock_ = true;
}

//-----------------------------------------------------------------------------
// ����: �ڽ��̵�ǰ�����źż��н�� setSignals ���õ��ź�
//-----------------------------------------------------------------------------
void SignalMasker::unBlock()
{
    sigProcMask(SIG_UNBLOCK, &newSet_, &oldSet_);
    isBlock_ = true;
}

//-----------------------------------------------------------------------------
// ����: �����������źż��ָ�Ϊ Block/UnBlock ֮ǰ��״̬
//-----------------------------------------------------------------------------
void SignalMasker::restore()
{
    if (isBlock_)
    {
        sigProcMask(SIG_SETMASK, &oldSet_, NULL);
        isBlock_ = false;
    }
}

//-----------------------------------------------------------------------------

int SignalMasker::sigProcMask(int how, const sigset_t *newSet, sigset_t *oldSet)
{
    int result;
    if ((result = sigprocmask(how, newSet, oldSet)) < 0)
        iseThrowException(strerror(errno));

    return result;
}

#endif

///////////////////////////////////////////////////////////////////////////////
// class SeqNumberAlloc

//-----------------------------------------------------------------------------
// ����: ���캯��
// ����:
//   startId - ��ʼ���к�
//-----------------------------------------------------------------------------
SeqNumberAlloc::SeqNumberAlloc(UINT64 startId)
{
    currentId_ = startId;
}

//-----------------------------------------------------------------------------
// ����: ����һ���·����ID
//-----------------------------------------------------------------------------
UINT64 SeqNumberAlloc::allocId()
{
    AutoLocker locker(mutex_);
    return currentId_++;
}

///////////////////////////////////////////////////////////////////////////////
// class Stream

void Stream::readBuffer(void *buffer, int count)
{
    if (count != 0 && read(buffer, count) != count)
        iseThrowStreamException(SEM_STREAM_READ_ERROR);
}

//-----------------------------------------------------------------------------

void Stream::writeBuffer(const void *buffer, int count)
{
    if (count != 0 && write(buffer, count) != count)
        iseThrowStreamException(SEM_STREAM_WRITE_ERROR);
}

//-----------------------------------------------------------------------------

INT64 Stream::getSize()
{
    INT64 pos, result;

    pos = seek(0, SO_CURRENT);
    result = seek(0, SO_END);
    seek(pos, SO_BEGINNING);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// class MemoryStream

//-----------------------------------------------------------------------------
// ����: ���캯��
// ����:
//   memoryDelta - �ڴ��������� (�ֽ����������� 2 �� N �η�)
//-----------------------------------------------------------------------------
MemoryStream::MemoryStream(int memoryDelta)
{
    init();
}

//-----------------------------------------------------------------------------

MemoryStream::MemoryStream(const MemoryStream& src)
{
    init();
    assign(src);
}

//-----------------------------------------------------------------------------

MemoryStream::~MemoryStream()
{
    clear();
}

//-----------------------------------------------------------------------------

void MemoryStream::init()
{
    memory_ = NULL;
    capacity_ = 0;
    size_ = 0;
    position_ = 0;
    setMemoryDelta(DEFAULT_MEMORY_DELTA);
}

//-----------------------------------------------------------------------------

MemoryStream& MemoryStream::operator = (const MemoryStream& rhs)
{
    if (this != &rhs)
        assign(rhs);
    return *this;
}

//-----------------------------------------------------------------------------
// ����: ���ڴ���
//-----------------------------------------------------------------------------
int MemoryStream::read(void *buffer, int count)
{
    int result = 0;

    if (position_ >= 0 && count >= 0)
    {
        result = size_ - position_;
        if (result > 0)
        {
            if (result > count) result = count;
            memmove(buffer, memory_ + (UINT)position_, result);
            position_ += result;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: д�ڴ���
//-----------------------------------------------------------------------------
int MemoryStream::write(const void *buffer, int count)
{
    int result = 0;
    int pos;

    if (position_ >= 0 && count >= 0)
    {
        pos = position_ + count;
        if (pos > 0)
        {
            if (pos > size_)
            {
                if (pos > capacity_)
                    setCapacity(pos);
                size_ = pos;
            }
            memmove(memory_ + (UINT)position_, buffer, count);
            position_ = pos;
            result = count;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: �ڴ���ָ�붨λ
//-----------------------------------------------------------------------------
INT64 MemoryStream::seek(INT64 offset, SEEK_ORIGIN seekOrigin)
{
    switch (seekOrigin)
    {
    case SO_BEGINNING:
        position_ = (int)offset;
        break;
    case SO_CURRENT:
        position_ += (int)offset;
        break;
    case SO_END:
        position_ = size_ + (int)offset;
        break;
    }

    return position_;
}

//-----------------------------------------------------------------------------
// ����: �����ڴ�����С
//-----------------------------------------------------------------------------
void MemoryStream::setSize(INT64 size)
{
    ISE_ASSERT(size <= MAXINT);

    int oldPos = position_;

    setCapacity((int)size);
    size_ = (int)size;
    if (oldPos > size) seek(0, SO_END);
}

//-----------------------------------------------------------------------------
// ����: �����������뵽�ڴ�����
//-----------------------------------------------------------------------------
bool MemoryStream::loadFromStream(Stream& stream)
{
    try
    {
        INT64 count = stream.getSize();
        ISE_ASSERT(count <= MAXINT);

        stream.setPosition(0);
        setSize(count);
        if (count != 0)
            stream.readBuffer(memory_, (int)count);
        return true;
    }
    catch (Exception&)
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
// ����: ���ļ����뵽�ڴ�����
//-----------------------------------------------------------------------------
bool MemoryStream::loadFromFile(const string& fileName)
{
    FileStream fs;
    bool result = fs.open(fileName, FM_OPEN_READ | FM_SHARE_DENY_WRITE);
    if (result)
        result = loadFromStream(fs);
    return result;
}

//-----------------------------------------------------------------------------
// ����: ���ڴ������浽��������
//-----------------------------------------------------------------------------
bool MemoryStream::saveToStream(Stream& stream)
{
    try
    {
        if (size_ != 0)
            stream.writeBuffer(memory_, size_);
        return true;
    }
    catch (Exception&)
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
// ����: ���ڴ������浽�ļ���
//-----------------------------------------------------------------------------
bool MemoryStream::saveToFile(const string& fileName)
{
    FileStream fs;
    bool result = fs.open(fileName, FM_CREATE);
    if (result)
        result = saveToStream(fs);
    return result;
}

//-----------------------------------------------------------------------------
// ����: ����ڴ���
//-----------------------------------------------------------------------------
void MemoryStream::clear()
{
    setCapacity(0);
    size_ = 0;
    position_ = 0;
}

//-----------------------------------------------------------------------------

void MemoryStream::assign(const MemoryStream& src)
{
    setMemoryDelta(src.memoryDelta_);
    setSize(src.size_);
    setPosition(src.position_);
    if (size_ > 0)
        memmove(memory_, src.memory_, size_);
}

//-----------------------------------------------------------------------------

void MemoryStream::setMemoryDelta(int newMemoryDelta)
{
    if (newMemoryDelta != DEFAULT_MEMORY_DELTA)
    {
        if (newMemoryDelta < MIN_MEMORY_DELTA)
            newMemoryDelta = MIN_MEMORY_DELTA;

        // ��֤ newMemoryDelta ��2��N�η�
        for (int i = sizeof(int) * 8 - 1; i >= 0; i--)
            if (((1 << i) & newMemoryDelta) != 0)
            {
                newMemoryDelta &= (1 << i);
                break;
            }
    }

    memoryDelta_ = newMemoryDelta;
}

//-----------------------------------------------------------------------------

void MemoryStream::setPointer(char *memory, int size)
{
    memory_ = memory;
    size_ = size;
}

//-----------------------------------------------------------------------------

void MemoryStream::setCapacity(int newCapacity)
{
    setPointer(realloc(newCapacity), size_);
    capacity_ = newCapacity;
}

//-----------------------------------------------------------------------------

char* MemoryStream::realloc(int& newCapacity)
{
    char* result;

    if (newCapacity > 0 && newCapacity != size_)
        newCapacity = (newCapacity + (memoryDelta_ - 1)) & ~(memoryDelta_ - 1);

    result = memory_;
    if (newCapacity != capacity_)
    {
        if (newCapacity == 0)
        {
            free(memory_);
            result = NULL;
        }
        else
        {
            if (capacity_ == 0)
                result = (char*)malloc(newCapacity);
            else
                result = (char*)::realloc(memory_, newCapacity);

            if (!result)
                iseThrowMemoryException();
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// class FileStream

//-----------------------------------------------------------------------------
// ����: ȱʡ���캯��
//-----------------------------------------------------------------------------
FileStream::FileStream()
{
    init();
}

//-----------------------------------------------------------------------------
// ����: ���캯��
// ����:
//   fileName   - �ļ���
//   openMode   - �ļ����򿪷�ʽ
//   rights     - �ļ���ȡȨ��
//-----------------------------------------------------------------------------
FileStream::FileStream(const string& fileName, UINT openMode, UINT rights)
{
    init();

    FileException e;
    if (!open(fileName, openMode, rights, &e))
        throw FileException(e);
}

//-----------------------------------------------------------------------------
// ����: ��������
//-----------------------------------------------------------------------------
FileStream::~FileStream()
{
    close();
}

//-----------------------------------------------------------------------------
// ����: ���ļ�
// ����:
//   fileName - �ļ���
//   openMode   - �ļ����򿪷�ʽ
//   rights     - �ļ���ȡȨ��
//   exception  - ��������쳣���򴫻ظ��쳣
//-----------------------------------------------------------------------------
bool FileStream::open(const string& fileName, UINT openMode, UINT rights,
    FileException* exception)
{
    close();

    if (openMode == FM_CREATE)
        handle_ = fileCreate(fileName, rights);
    else
        handle_ = fileOpen(fileName, openMode);

    bool result = (handle_ != INVALID_HANDLE_VALUE);

    if (!result && exception != NULL)
    {
        if (openMode == FM_CREATE)
            *exception = FileException(fileName.c_str(), getLastSysError(),
                formatString(SEM_CANNOT_CREATE_FILE, fileName.c_str(),
                sysErrorMessage(getLastSysError()).c_str()).c_str());
        else
            *exception = FileException(fileName.c_str(), getLastSysError(),
                formatString(SEM_CANNOT_OPEN_FILE, fileName.c_str(),
                sysErrorMessage(getLastSysError()).c_str()).c_str());
    }

    fileName_ = fileName;
    return result;
}

//-----------------------------------------------------------------------------
// ����: �ر��ļ�
//-----------------------------------------------------------------------------
void FileStream::close()
{
    if (handle_ != INVALID_HANDLE_VALUE)
    {
        fileClose(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
    fileName_.clear();
}

//-----------------------------------------------------------------------------
// ����: ���ļ���
//-----------------------------------------------------------------------------
int FileStream::read(void *buffer, int count)
{
    int result;

    result = fileRead(handle_, buffer, count);
    if (result == -1) result = 0;

    return result;
}

//-----------------------------------------------------------------------------
// ����: д�ļ���
//-----------------------------------------------------------------------------
int FileStream::write(const void *buffer, int count)
{
    int result;

    result = fileWrite(handle_, buffer, count);
    if (result == -1) result = 0;

    return result;
}

//-----------------------------------------------------------------------------
// ����: �ļ���ָ�붨λ
//-----------------------------------------------------------------------------
INT64 FileStream::seek(INT64 offset, SEEK_ORIGIN seekOrigin)
{
    return fileSeek(handle_, offset, seekOrigin);
}

//-----------------------------------------------------------------------------
// ����: �����ļ�����С
//-----------------------------------------------------------------------------
void FileStream::setSize(INT64 size)
{
    bool success;
    seek(size, SO_BEGINNING);

#ifdef ISE_WINDOWS
    success = ::SetEndOfFile(handle_) != 0;
#endif
#ifdef ISE_LINUX
    success = (::ftruncate(handle_, static_cast<__off_t>(getPosition())) == 0);
#endif

    if (!success)
        iseThrowFileException(fileName_.c_str(), getLastSysError(), SEM_SET_FILE_STREAM_SIZE_ERR);
}

//-----------------------------------------------------------------------------
// ����: �ж��ļ�����ǰ�Ƿ��״̬
//-----------------------------------------------------------------------------
bool FileStream::isOpen() const
{
    return (handle_ != INVALID_HANDLE_VALUE);
}

//-----------------------------------------------------------------------------
// ����: ��ʼ��
//-----------------------------------------------------------------------------
void FileStream::init()
{
    fileName_.clear();
    handle_ = INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
// ����: �����ļ�
//-----------------------------------------------------------------------------
HANDLE FileStream::fileCreate(const string& fileName, UINT rights)
{
#ifdef ISE_WINDOWS
    return ::CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#endif
#ifdef ISE_LINUX
    umask(0);  // ��ֹ rights �� umask ֵ ����
    return ::open(fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, rights);
#endif
}

//-----------------------------------------------------------------------------
// ����: ���ļ�
//-----------------------------------------------------------------------------
HANDLE FileStream::fileOpen(const string& fileName, UINT openMode)
{
#ifdef ISE_WINDOWS
    UINT accessModes[3] = {
        GENERIC_READ,
        GENERIC_WRITE,
        GENERIC_READ | GENERIC_WRITE
    };
    UINT shareModes[5] = {
        0,
        0,
        FILE_SHARE_READ,
        FILE_SHARE_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE
    };

    HANDLE fileHandle = INVALID_HANDLE_VALUE;

    if ((openMode & 3) <= FM_OPEN_READ_WRITE &&
        (openMode & 0xF0) <= FM_SHARE_DENY_NONE)
        fileHandle = ::CreateFileA(fileName.c_str(), accessModes[openMode & 3],
            shareModes[(openMode & 0xF0) >> 4], NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    return fileHandle;
#endif
#ifdef ISE_LINUX
    BYTE shareModes[4] = {
        0,          // none
        F_WRLCK,    // FM_SHARE_EXCLUSIVE
        F_RDLCK,    // FM_SHARE_DENY_WRITE
        0           // FM_SHARE_DENY_NONE
    };

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    BYTE shareMode;

    if (fileExists(fileName) &&
        (openMode & 0x03) <= FM_OPEN_READ_WRITE &&
        (openMode & 0xF0) <= FM_SHARE_DENY_NONE)
    {
        umask(0);  // ��ֹ openMode �� umask ֵ����
        fileHandle = ::open(fileName.c_str(), (openMode & 0x03), DEFAULT_FILE_ACCESS_RIGHTS);
        if (fileHandle != INVALID_HANDLE_VALUE)
        {
            shareMode = static_cast<BYTE>((openMode & 0xF0) >> 4);
            if (shareModes[shareMode] != 0)
            {
                struct flock flk;

                flk.l_type = shareModes[shareMode];
                flk.l_whence = SEEK_SET;
                flk.l_start = 0;
                flk.l_len = 0;

                if (fcntl(fileHandle, F_SETLK, &flk) < 0)
                {
                    fileClose(fileHandle);
                    fileHandle = INVALID_HANDLE_VALUE;
                }
            }
        }
    }

    return fileHandle;
#endif
}

//-----------------------------------------------------------------------------
// ����: �ر��ļ�
//-----------------------------------------------------------------------------
void FileStream::fileClose(HANDLE handle)
{
#ifdef ISE_WINDOWS
    ::CloseHandle(handle);
#endif
#ifdef ISE_LINUX
    ::close(handle);
#endif
}

//-----------------------------------------------------------------------------
// ����: ���ļ�
//-----------------------------------------------------------------------------
int FileStream::fileRead(HANDLE handle, void *buffer, int count)
{
#ifdef ISE_WINDOWS
    unsigned long result;
    if (!::ReadFile(handle, buffer, count, &result, NULL))
        result = -1;
    return result;
#endif
#ifdef ISE_LINUX
    return ::read(handle, buffer, count);
#endif
}

//-----------------------------------------------------------------------------
// ����: д�ļ�
//-----------------------------------------------------------------------------
int FileStream::fileWrite(HANDLE handle, const void *buffer, int count)
{
#ifdef ISE_WINDOWS
    unsigned long result;
    if (!::WriteFile(handle, buffer, count, &result, NULL))
        result = -1;
    return result;
#endif
#ifdef ISE_LINUX
    return ::write(handle, buffer, count);
#endif
}

//-----------------------------------------------------------------------------
// ����: �ļ�ָ�붨λ
//-----------------------------------------------------------------------------
INT64 FileStream::fileSeek(HANDLE handle, INT64 offset, SEEK_ORIGIN seekOrigin)
{
#ifdef ISE_WINDOWS
    INT64 result = offset;
    ((Int64Rec*)&result)->ints.lo = ::SetFilePointer(
        handle, ((Int64Rec*)&result)->ints.lo,
        (PLONG)&(((Int64Rec*)&result)->ints.hi), seekOrigin);
    if (((Int64Rec*)&result)->ints.lo == -1 && GetLastError() != 0)
        ((Int64Rec*)&result)->ints.hi = -1;
    return result;
#endif
#ifdef ISE_LINUX
    INT64 result = ::lseek(handle, static_cast<__off_t>(offset), seekOrigin);
    return result;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// class PointerList

PointerList::PointerList() :
    list_(NULL),
    count_(0),
    capacity_(0)
{
    // nothing
}

//-----------------------------------------------------------------------------

PointerList::~PointerList()
{
    clear();
}

//-----------------------------------------------------------------------------
// ����: ���б������Ԫ��
//-----------------------------------------------------------------------------
int PointerList::add(POINTER item)
{
    if (count_ == capacity_) grow();
    list_[count_] = item;
    count_++;

    return count_ - 1;
}

//-----------------------------------------------------------------------------
// ����: ���б��в���Ԫ��
// ����:
//   index - ����λ���±��(0-based)
//-----------------------------------------------------------------------------
void PointerList::insert(int index, POINTER item)
{
    if (index < 0 || index > count_)
        iseThrowException(SEM_INDEX_ERROR);

    if (count_ == capacity_) grow();
    if (index < count_)
        memmove(&list_[index + 1], &list_[index], (count_ - index) * sizeof(POINTER));
    list_[index] = item;
    count_++;
}

//-----------------------------------------------------------------------------
// ����: ���б���ɾ��Ԫ��
// ����:
//   index - �±��(0-based)
//-----------------------------------------------------------------------------
void PointerList::del(int index)
{
    if (index < 0 || index >= count_)
        iseThrowException(SEM_INDEX_ERROR);

    count_--;
    if (index < count_)
        memmove(&list_[index], &list_[index + 1], (count_ - index) * sizeof(POINTER));
}

//-----------------------------------------------------------------------------
// ����: ���б���ɾ��Ԫ��
// ����: ��ɾ��Ԫ�����б��е��±��(0-based)����δ�ҵ����򷵻� -1.
//-----------------------------------------------------------------------------
int PointerList::remove(POINTER item)
{
    int result;

    result = indexOf(item);
    if (result >= 0)
        del(result);

    return result;
}

//-----------------------------------------------------------------------------
// ����: ���б���ɾ��Ԫ��
// ����: ��ɾ����Ԫ��ֵ����δ�ҵ����򷵻� NULL.
//-----------------------------------------------------------------------------
POINTER PointerList::extract(POINTER item)
{
    int i;
    POINTER result = NULL;

    i = indexOf(item);
    if (i >= 0)
    {
        result = item;
        list_[i] = NULL;
        del(i);
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: �ƶ�һ��Ԫ�ص��µ�λ��
//-----------------------------------------------------------------------------
void PointerList::move(int curIndex, int newIndex)
{
    POINTER item;

    if (curIndex != newIndex)
    {
        if (newIndex < 0 || newIndex >= count_)
            iseThrowException(SEM_INDEX_ERROR);

        item = get(curIndex);
        list_[curIndex] = NULL;
        del(curIndex);
        insert(newIndex, NULL);
        list_[newIndex] = item;
    }
}

//-----------------------------------------------------------------------------
// ����: �ı��б�Ĵ�С
//-----------------------------------------------------------------------------
void PointerList::resize(int count)
{
    setCount(count);
}

//-----------------------------------------------------------------------------
// ����: ����б�
//-----------------------------------------------------------------------------
void PointerList::clear()
{
    setCount(0);
    setCapacity(0);
}

//-----------------------------------------------------------------------------
// ����: �����б��е��׸�Ԫ�� (���б�Ϊ�����׳��쳣)
//-----------------------------------------------------------------------------
POINTER PointerList::first() const
{
    return get(0);
}

//-----------------------------------------------------------------------------
// ����: �����б��е����Ԫ�� (���б�Ϊ�����׳��쳣)
//-----------------------------------------------------------------------------
POINTER PointerList::last() const
{
    return get(count_ - 1);
}

//-----------------------------------------------------------------------------
// ����: ����Ԫ�����б��е��±�� (��δ�ҵ��򷵻� -1)
//-----------------------------------------------------------------------------
int PointerList::indexOf(POINTER item) const
{
    int result = 0;

    while (result < count_ && list_[result] != item) result++;
    if (result == count_)
        result = -1;

    return result;
}

//-----------------------------------------------------------------------------
// ����: �����б���Ԫ������
//-----------------------------------------------------------------------------
int PointerList::getCount() const
{
    return count_;
}

//-----------------------------------------------------------------------------
// ����: ��ֵ������
//-----------------------------------------------------------------------------
PointerList& PointerList::operator = (const PointerList& rhs)
{
    if (this == &rhs) return *this;

    clear();
    setCapacity(rhs.capacity_);
    for (int i = 0; i < rhs.getCount(); i++)
        add(rhs[i]);
    return *this;
}

//-----------------------------------------------------------------------------
// ����: ��ȡ�б�������Ԫ��
//-----------------------------------------------------------------------------
const POINTER& PointerList::operator[] (int index) const
{
    if (index < 0 || index >= count_)
        iseThrowException(SEM_INDEX_ERROR);

    return list_[index];
}

//-----------------------------------------------------------------------------
// ����: ��ȡ�б�������Ԫ��
//-----------------------------------------------------------------------------
POINTER& PointerList::operator[] (int index)
{
    return
        const_cast<POINTER&>(
            ((const PointerList&)(*this))[index]
        );
}

//-----------------------------------------------------------------------------

void PointerList::grow()
{
    int delta;

    if (capacity_ > 64)
        delta = capacity_ / 4;
    else if (capacity_ > 8)
        delta = 16;
    else
        delta = 4;

    setCapacity(capacity_ + delta);
}

//-----------------------------------------------------------------------------

POINTER PointerList::get(int index) const
{
    if (index < 0 || index >= count_)
        iseThrowException(SEM_INDEX_ERROR);

    return list_[index];
}

//-----------------------------------------------------------------------------

void PointerList::put(int index, POINTER item)
{
    if (index < 0 || index >= count_)
        iseThrowException(SEM_INDEX_ERROR);

    list_[index] = item;
}

//-----------------------------------------------------------------------------

void PointerList::setCapacity(int newCapacity)
{
    if (newCapacity < count_)
        iseThrowException(SEM_LIST_CAPACITY_ERROR);

    if (newCapacity != capacity_)
    {
        if (newCapacity > 0)
        {
            POINTER *p = (POINTER*)realloc(list_, newCapacity * sizeof(PVOID));
            if (p)
                list_ = p;
            else
                iseThrowMemoryException();
        }
        else
        {
            if (list_)
            {
                free(list_);
                list_ = NULL;
            }
        }

        capacity_ = newCapacity;
    }
}

//-----------------------------------------------------------------------------

void PointerList::setCount(int newCount)
{
    if (newCount < 0)
        iseThrowException(SEM_LIST_COUNT_ERROR);

    if (newCount > capacity_)
        setCapacity(newCount);
    if (newCount > count_)
        memset(&list_[count_], 0, (newCount - count_) * sizeof(POINTER));
    else
        for (int i = count_ - 1; i >= newCount; i--) del(i);

    count_ = newCount;
}

///////////////////////////////////////////////////////////////////////////////
// class PropertyList

PropertyList::PropertyList()
{
    // nothing
}

PropertyList::PropertyList(const PropertyList& src)
{
    assign(src);
}

PropertyList::~PropertyList()
{
    clear();
}

//-----------------------------------------------------------------------------
// ����: ���б������Ԫ��
//-----------------------------------------------------------------------------
void PropertyList::add(const string& name, const string& value)
{
    if (name.find(NAME_VALUE_SEP, 0) != string::npos)
        iseThrowException(SEM_PROPLIST_NAME_ERROR);

    PropertyItem *item = find(name);
    if (!item)
    {
        item = new PropertyItem(name, value);
        items_.add(item);
    }
    else
        item->value = value;
}

//-----------------------------------------------------------------------------
// ����: ���б���ɾ��Ԫ��
// ����:
//   true  - �ɹ�
//   false - ʧ��(δ�ҵ�)
//-----------------------------------------------------------------------------
bool PropertyList::remove(const string& name)
{
    int i = indexOf(name);
    bool result = (i >= 0);

    if (result)
    {
        delete (PropertyItem*)items_[i];
        items_.del(i);
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: ����б�
//-----------------------------------------------------------------------------
void PropertyList::clear()
{
    for (int i = 0; i < items_.getCount(); i++)
        delete (PropertyItem*)items_[i];
    items_.clear();
}

//-----------------------------------------------------------------------------
// ����: ����ĳ���������б��е�λ��(0-based)
//-----------------------------------------------------------------------------
int PropertyList::indexOf(const string& name) const
{
    int result = -1;

    for (int i = 0; i < items_.getCount(); i++)
        if (sameText(name, ((PropertyItem*)items_[i])->name))
        {
            result = i;
            break;
        }

    return result;
}

//-----------------------------------------------------------------------------
// ����: �ж�ĳ�������Ƿ����
//-----------------------------------------------------------------------------
bool PropertyList::nameExists(const string& name) const
{
    return (indexOf(name) >= 0);
}

//-----------------------------------------------------------------------------
// ����: ���� Name ���б���ȡ�� Value
// ����: ���б��в����ڸ������ԣ��򷵻� False��
//-----------------------------------------------------------------------------
bool PropertyList::getValue(const string& name, string& value) const
{
    int i = indexOf(name);
    bool result = (i >= 0);

    if (result)
        value = ((PropertyItem*)items_[i])->value;

    return result;
}

//-----------------------------------------------------------------------------
// ����: �����±��(0-based)ȡ��������Ŀ
//-----------------------------------------------------------------------------
const PropertyList::PropertyItem& PropertyList::getItem(int index) const
{
    if (index < 0 || index >= getCount())
        iseThrowException(SEM_INDEX_ERROR);

    return *((PropertyItem*)items_[index]);
}

//-----------------------------------------------------------------------------
// ����: �������б�ת���� PropString ������
// ʾ��:
//   [<abc,123>, <def,456>]   ->  abc=123,def=456
//   [<abc,123>, <,456>]      ->  abc=123,=456
//   [<abc,123>, <",456>]     ->  abc=123,"""=456"
//   [<abc,123>, <',456>]     ->  abc=123,'=456
//   [<abc,123>, <def,">]     ->  abc=123,"def="""
//   [<abc,123>, < def,456>]  ->  abc=123," def=456"
//   [<abc,123>, <def,=>]     ->  abc=123,def==
//   [<abc,123>, <=,456>]     ->  �׳��쳣(Name�в�������ڵȺ�"=")
//-----------------------------------------------------------------------------
string PropertyList::getPropString() const
{
    string result;
    string itemStr;

    for (int index = 0; index < getCount(); index++)
    {
        const PropertyItem& item = getItem(index);

        itemStr = item.name + (char)NAME_VALUE_SEP + item.value;
        if (hasReservedChar(itemStr))
            itemStr = makeQuotedStr(itemStr);

        result += itemStr;
        if (index < getCount() - 1) result += (char)PROP_ITEM_SEP;
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: �� PropString ת���������б�
//-----------------------------------------------------------------------------
void PropertyList::setPropString(const string& propString)
{
    char *strPtr = const_cast<char*>(propString.c_str());
    string itemStr;

    clear();
    while (*strPtr)
    {
        if (*strPtr == PROP_ITEM_QUOT)
            itemStr = extractQuotedStr(strPtr);
        else
        {
            char *p = strPtr;
            strPtr = scanStr(strPtr, PROP_ITEM_SEP);
            itemStr.assign(p, strPtr - p);
            if (*strPtr == PROP_ITEM_SEP) strPtr++;
        }

        string::size_type i = itemStr.find(NAME_VALUE_SEP, 0);
        if (i != string::npos)
            add(itemStr.substr(0, i), itemStr.substr(i + 1));
    }
}

//-----------------------------------------------------------------------------
// ����: ��ֵ������
//-----------------------------------------------------------------------------
PropertyList& PropertyList::operator = (const PropertyList& rhs)
{
    if (this != &rhs)
        assign(rhs);
    return *this;
}

//-----------------------------------------------------------------------------
// ����: ��ȡ�б�������Ԫ��
// ע��: ���ô˺���ʱ���� name �����ڣ���������ӵ��б��У�
//-----------------------------------------------------------------------------
string& PropertyList::operator[] (const string& name)
{
    int i = indexOf(name);
    if (i < 0)
    {
        add(name, "");
        i = getCount() - 1;
    }

    return ((PropertyItem*)items_[i])->value;
}

//-----------------------------------------------------------------------------

void PropertyList::assign(const PropertyList& src)
{
    clear();
    for (int i = 0; i < src.getCount(); i++)
        add(src.getItem(i).name, src.getItem(i).value);
}

//-----------------------------------------------------------------------------
// ����: ����ĳ��������Ŀ��û�ҵ��򷵻� NULL
//-----------------------------------------------------------------------------
PropertyList::PropertyItem* PropertyList::find(const string& name)
{
    int i = indexOf(name);
    PropertyItem *result = (i >= 0? (PropertyItem*)items_[i] : NULL);
    return result;
}

//-----------------------------------------------------------------------------

bool PropertyList::isReservedChar(char ch)
{
    return
        ((ch >= 0) && (ch <= 32)) ||
        (ch == (char)PROP_ITEM_SEP) ||
        (ch == (char)PROP_ITEM_QUOT);
}

//-----------------------------------------------------------------------------

bool PropertyList::hasReservedChar(const string& str)
{
    for (UINT i = 0; i < str.length(); i++)
        if (isReservedChar(str[i])) return true;
    return false;
}

//-----------------------------------------------------------------------------

char* PropertyList::scanStr(char *str, char ch)
{
    char *result = str;
    while ((*result != ch) && (*result != 0)) result++;
    return result;
}

//-----------------------------------------------------------------------------
// ����: ���ַ��� str ������(")������
// ʾ��:
//   abcdefg   ->  "abcdefg"
//   abc"efg   ->  "abc""efg"
//   abc""fg   ->  "abc""""fg"
//-----------------------------------------------------------------------------
string PropertyList::makeQuotedStr(const string& str)
{
    const char QUOT_CHAR = (char)PROP_ITEM_QUOT;
    string result;

    for (UINT i = 0; i < str.length(); i++)
    {
        if (str[i] == QUOT_CHAR)
            result += string(2, QUOT_CHAR);
        else
            result += str[i];
    }
    result = "\"" + result + "\"";

    return result;
}

//-----------------------------------------------------------------------------
// ����: �� MakeQuotedStr ���ɵ��ַ�����ԭ
// ����:
//   strPtr - ��������ַ���ָ�룬������󴫻ش����ֹλ��
// ����:
//   ��ԭ����ַ���
// ʾ��:
//   "abcde"xyz   ->  abcde       ��������ʱ strPtr ָ�� x
//   "ab""cd"     ->  ab"cd       ��������ʱ strPtr ָ�� '\0'
//   "ab""""cd"   ->  ab""cd      ��������ʱ strPtr ָ�� '\0'
//-----------------------------------------------------------------------------
string PropertyList::extractQuotedStr(char*& strPtr)
{
    const char QUOT_CHAR = (char)PROP_ITEM_QUOT;
    string result;
    int dropCount;
    char *p;

    if (strPtr == NULL || *strPtr != QUOT_CHAR) return result;

    strPtr++;
    dropCount = 1;
    p = strPtr;
    strPtr = scanStr(strPtr, QUOT_CHAR);
    while (*strPtr)
    {
        strPtr++;
        if (*strPtr != QUOT_CHAR) break;
        strPtr++;
        dropCount++;
        strPtr = scanStr(strPtr, QUOT_CHAR);
    }

    if (((strPtr - p) <= 1) || ((strPtr - p - dropCount) == 0)) return result;

    if (dropCount == 1)
        result.assign(p, strPtr - p - 1);
    else
    {
        result.resize(strPtr - p - dropCount);
        char *dest = const_cast<char*>(result.c_str());
        while (p < strPtr)
        {
            if ((*p == QUOT_CHAR) && (p < strPtr - 1) && (*(p+1) == QUOT_CHAR)) p++;
            *dest++ = *p++;
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// class Strings

Strings::Strings()
{
    init();
}

//-----------------------------------------------------------------------------

int Strings::add(const char* str)
{
    int result = getCount();
    insert(result, str);
    return result;
}

//-----------------------------------------------------------------------------

int Strings::add(const char* str, POINTER data)
{
    int result = add(str);
    setData(result, data);
    return result;
}

//-----------------------------------------------------------------------------

void Strings::addStrings(const Strings& strings)
{
    AutoUpdater autoUpdater(*this);

    for (int i = 0; i < strings.getCount(); i++)
        add(strings.getString(i).c_str(), strings.getData(i));
}

//-----------------------------------------------------------------------------

void Strings::insert(int index, const char* str, POINTER data)
{
    insert(index, str);
    setData(index, data);
}

//-----------------------------------------------------------------------------

bool Strings::equals(const Strings& strings)
{
    bool result;
    int count = getCount();

    result = (count == strings.getCount());
    if (result)
    {
        for (int i = 0; i < count; i++)
            if (getString(i) != strings.getString(i))
            {
                result = false;
                break;
            }
    }

    return result;
}

//-----------------------------------------------------------------------------

void Strings::exchange(int index1, int index2)
{
    AutoUpdater autoUpdater(*this);

    POINTER tempData;
    string tempStr;

    tempStr = getString(index1);
    tempData = getData(index1);
    setString(index1, getString(index2).c_str());
    setData(index1, getData(index2));
    setString(index2, tempStr.c_str());
    setData(index2, tempData);
}

//-----------------------------------------------------------------------------

void Strings::move(int curIndex, int newIndex)
{
    if (curIndex != newIndex)
    {
        AutoUpdater autoUpdater(*this);

        POINTER tempData;
        string tempStr;

        tempStr = getString(curIndex);
        tempData = getData(curIndex);
        del(curIndex);
        insert(newIndex, tempStr.c_str(), tempData);
    }
}

//-----------------------------------------------------------------------------

bool Strings::exists(const char* str) const
{
    return (indexOf(str) >= 0);
}

//-----------------------------------------------------------------------------

int Strings::indexOf(const char* str) const
{
    int result = -1;

    for (int i = 0; i < getCount(); i++)
        if (compareStrings(getString(i).c_str(), str) == 0)
        {
            result = i;
            break;
        }

    return result;
}

//-----------------------------------------------------------------------------

int Strings::indexOfName(const char* name) const
{
    int result = -1;

    for (int i = 0; i < getCount(); i++)
        if (compareStrings((extractName(getString(i).c_str()).c_str()), name) == 0)
        {
            result = i;
            break;
        }

    return result;
}

//-----------------------------------------------------------------------------

int Strings::indexOfData(POINTER data) const
{
    int result = -1;

    for (int i = 0; i < getCount(); i++)
        if (getData(i) == data)
        {
            result = i;
            break;
        }

    return result;
}

//-----------------------------------------------------------------------------

bool Strings::loadFromStream(Stream& stream)
{
    try
    {
        AutoUpdater autoUpdater(*this);

        const int BLOCK_SIZE = 1024*64;
        Buffer block(BLOCK_SIZE);
        MemoryStream memStream;

        while (true)
        {
            int bytesRead = stream.read(block.data(), block.getSize());
            if (bytesRead > 0)
                memStream.write(block.data(), bytesRead);

            if (bytesRead < block.getSize())
                break;
        }

        char endChar = 0;
        memStream.write(&endChar, sizeof(char));

        setText(memStream.getMemory());
        return true;
    }
    catch (Exception&)
    {
        return false;
    }
}

//-----------------------------------------------------------------------------

bool Strings::loadFromFile(const char* fileName)
{
    FileStream fs;
    bool result = fs.open(fileName, FM_OPEN_READ | FM_SHARE_DENY_WRITE);
    if (result)
        result = loadFromStream(fs);
    return result;
}

//-----------------------------------------------------------------------------

bool Strings::saveToStream(Stream& stream) const
{
    try
    {
        string text = getText();
        int len = (int)text.length();
        stream.writeBuffer((char*)text.c_str(), len * sizeof(char));
        return true;
    }
    catch (Exception&)
    {
        return false;
    }
}

//-----------------------------------------------------------------------------

bool Strings::saveToFile(const char* fileName) const
{
    FileStream fs;
    bool result = fs.open(fileName, FM_CREATE);
    if (result)
        result = saveToStream(fs);
    return result;
}

//-----------------------------------------------------------------------------

char Strings::getDelimiter() const
{
    if ((defined_ & SD_DELIMITER) == 0)
        const_cast<Strings&>(*this).setDelimiter(DEFAULT_DELIMITER);
    return delimiter_;
}

//-----------------------------------------------------------------------------

void Strings::setDelimiter(char value)
{
    if (delimiter_ != value || (defined_ & SD_DELIMITER) == 0)
    {
        defined_ |= SD_DELIMITER;
        delimiter_ = value;
    }
}

//-----------------------------------------------------------------------------

string Strings::getLineBreak() const
{
    if ((defined_ & SD_LINE_BREAK) == 0)
        const_cast<Strings&>(*this).setLineBreak(DEFAULT_LINE_BREAK);
    return lineBreak_;
}

//-----------------------------------------------------------------------------

void Strings::setLineBreak(const char* value)
{
    if (lineBreak_ != string(value) || (defined_ & SD_LINE_BREAK) == 0)
    {
        defined_ |= SD_LINE_BREAK;
        lineBreak_ = value;
    }
}

//-----------------------------------------------------------------------------

char Strings::getQuoteChar() const
{
    if ((defined_ & SD_QUOTE_CHAR) == 0)
        const_cast<Strings&>(*this).setQuoteChar(DEFAULT_QUOTE_CHAR);
    return quoteChar_;
}

//-----------------------------------------------------------------------------

void Strings::setQuoteChar(char value)
{
    if (quoteChar_ != value || (defined_ & SD_QUOTE_CHAR) == 0)
    {
        defined_ |= SD_QUOTE_CHAR;
        quoteChar_ = value;
    }
}

//-----------------------------------------------------------------------------

char Strings::getNameValueSeparator() const
{
    if ((defined_ & SD_NAME_VALUE_SEP) == 0)
        const_cast<Strings&>(*this).setNameValueSeparator(DEFAULT_NAME_VALUE_SEP);
    return nameValueSeparator_;
}

//-----------------------------------------------------------------------------

void Strings::setNameValueSeparator(char value)
{
    if (nameValueSeparator_ != value || (defined_ & SD_NAME_VALUE_SEP) == 0)
    {
        defined_ |= SD_NAME_VALUE_SEP;
        nameValueSeparator_ = value;
    }
}

//-----------------------------------------------------------------------------

string Strings::combineNameValue(const char* name, const char* value) const
{
    string nameStr(name);
    char nameValueSep = getNameValueSeparator();

    if (nameStr.find(nameValueSep) != string::npos)
        error(SEM_STRINGS_NAME_ERROR, 0);

    return nameStr + nameValueSep + value;
}

//-----------------------------------------------------------------------------

string Strings::getName(int index) const
{
    return extractName(getString(index).c_str());
}

//-----------------------------------------------------------------------------

string Strings::getValue(const char* name) const
{
    string nameStr(name);
    int i = indexOfName(nameStr.c_str());
    if (i >= 0)
        return getString(i).substr(nameStr.length() + 1);
    else
        return "";
}

//-----------------------------------------------------------------------------

string Strings::getValue(int index) const
{
    if (index >= 0)
    {
        string name = getName(index);
        if (!name.empty())
            return getString(index).substr(name.length() + 1);
        else
        {
            string strItem = getString(index);
            if (!strItem.empty() && strItem[0] == getNameValueSeparator())
                return strItem.substr(1);
            else
                return "";
        }
    }
    else
        return "";
}

//-----------------------------------------------------------------------------

void Strings::setValue(const char* name, const char* value)
{
    string nameStr(name);
    string valueStr(value);

    int i = indexOfName(nameStr.c_str());
    if (valueStr.empty())
    {
        if (i >= 0) del(i);
    }
    else
    {
        if (i < 0)
            i = add("");
        setString(i, (nameStr + getNameValueSeparator() + valueStr).c_str());
    }
}

//-----------------------------------------------------------------------------

void Strings::setValue(int index, const char* value)
{
    string valueStr(value);

    if (valueStr.empty())
    {
        if (index >= 0) del(index);
    }
    else
    {
        if (index < 0)
            index = add("");
        setString(index, (getName(index) + getNameValueSeparator() + valueStr).c_str());
    }
}

//-----------------------------------------------------------------------------

string Strings::getText() const
{
    string result;
    int count = getCount();
    string lineBreak = getLineBreak();

    for (int i = 0; i < count; i++)
    {
        result += getString(i);
        result += lineBreak;
    }

    return result;
}

//-----------------------------------------------------------------------------

void Strings::setText(const char* value)
{
    AutoUpdater autoUpdater(*this);

    string valueStr(value);
    string lineBreak = getLineBreak();
    int start = 0;

    clear();
    while (true)
    {
        string::size_type pos = valueStr.find(lineBreak, start);
        if (pos != string::npos)
        {
            add(valueStr.substr(start, pos - start).c_str());
            start = (int)(pos + lineBreak.length());
        }
        else
        {
            string str = valueStr.substr(start);
            if (!str.empty())
                add(str.c_str());
            break;
        }
    }
}

//-----------------------------------------------------------------------------

string Strings::getCommaText() const
{
    UINT bakDefined = defined_;
    char bakDelimiter = delimiter_;
    char bakQuoteChar = quoteChar_;

    const_cast<Strings&>(*this).setDelimiter(DEFAULT_DELIMITER);
    const_cast<Strings&>(*this).setQuoteChar(DEFAULT_QUOTE_CHAR);

    string result = getDelimitedText();  // �������׳��쳣

    const_cast<Strings&>(*this).defined_ = bakDefined;
    const_cast<Strings&>(*this).delimiter_ = bakDelimiter;
    const_cast<Strings&>(*this).quoteChar_ = bakQuoteChar;

    return result;
}

//-----------------------------------------------------------------------------

void Strings::setCommaText(const char* value)
{
    setDelimiter(DEFAULT_DELIMITER);
    setQuoteChar(DEFAULT_QUOTE_CHAR);

    setDelimitedText(value);
}

//-----------------------------------------------------------------------------

string Strings::getDelimitedText() const
{
    string result;
    string line;
    int count = getCount();
    char quoteChar = getQuoteChar();
    char delimiter = getDelimiter();

    if (count == 1 && getString(0).empty())
        return string(getQuoteChar(), 2);

    string delimiters;
    for (int i = 1; i <= 32; i++)
        delimiters += (char)i;
    delimiters += quoteChar;
    delimiters += delimiter;

    for (int i = 0; i < count; i++)
    {
        line = getString(i);
        if (line.find_first_of(delimiters) != string::npos)
            line = getQuotedStr((char*)line.c_str(), quoteChar);

        if (i > 0) result += delimiter;
        result += line;
    }

    return result;
}

//-----------------------------------------------------------------------------

void Strings::setDelimitedText(const char* value)
{
    AutoUpdater autoUpdater(*this);

    char quoteChar = getQuoteChar();
    char delimiter = getDelimiter();
    const char* curPtr = value;
    string line;

    clear();
    while (*curPtr >= 1 && *curPtr <= 32)
        curPtr++;

    while (*curPtr != '\0')
    {
        if (*curPtr == quoteChar)
            line = extractQuotedStr(curPtr, quoteChar);
        else
        {
            const char* p = curPtr;
            while (*curPtr > 32 && *curPtr != delimiter)
                curPtr++;
            line.assign(p, curPtr - p);
        }

        add(line.c_str());

        while (*curPtr >= 1 && *curPtr <= 32)
            curPtr++;

        if (*curPtr == delimiter)
        {
            const char* p = curPtr;
            p++;
            if (*p == '\0')
                add("");

            do curPtr++; while (*curPtr >= 1 && *curPtr <= 32);
        }
    }
}

//-----------------------------------------------------------------------------

void Strings::setString(int index, const char* value)
{
    POINTER tempData = getData(index);
    del(index);
    insert(index, value, tempData);
}

//-----------------------------------------------------------------------------

void Strings::beginUpdate()
{
    if (updateCount_ == 0)
        setUpdateState(true);
    updateCount_++;
}

//-----------------------------------------------------------------------------

void Strings::endUpdate()
{
    updateCount_--;
    if (updateCount_ == 0)
        setUpdateState(false);
}

//-----------------------------------------------------------------------------

Strings& Strings::operator = (const Strings& rhs)
{
    if (this != &rhs)
        assign(rhs);
    return *this;
}

//-----------------------------------------------------------------------------

int Strings::compareStrings(const char* str1, const char* str2) const
{
    int r = compareText(str1, str2);

    if (r > 0) r = 1;
    else if (r < 0) r = -1;

    return r;
}

//-----------------------------------------------------------------------------

void Strings::init()
{
    defined_ = 0;
    delimiter_ = 0;
    quoteChar_ = 0;
    nameValueSeparator_ = 0;
    updateCount_ = 0;
}

//-----------------------------------------------------------------------------

void Strings::error(const char* msg, int data) const
{
    iseThrowException(formatString(msg, data).c_str());
}

//-----------------------------------------------------------------------------

string Strings::extractName(const char* str) const
{
    string result(str);

    string::size_type i = result.find(getNameValueSeparator());
    if (i != string::npos)
        result = result.substr(0, i);
    else
        result.clear();

    return result;
}

//-----------------------------------------------------------------------------

void Strings::assign(const Strings& src)
{
    AutoUpdater autoUpdater(*this);
    clear();

    defined_ = src.defined_;
    delimiter_ = src.delimiter_;
    lineBreak_ = src.lineBreak_;
    quoteChar_ = src.quoteChar_;
    nameValueSeparator_ = src.nameValueSeparator_;

    addStrings(src);
}

///////////////////////////////////////////////////////////////////////////////
// class StrList

//-----------------------------------------------------------------------------

StrList::StrList()
{
    init();
}

//-----------------------------------------------------------------------------

StrList::StrList(const StrList& src)
{
    init();
    assign(src);
}

//-----------------------------------------------------------------------------

StrList::~StrList()
{
    internalClear();
}

//-----------------------------------------------------------------------------

void StrList::assign(const StrList& src)
{
    Strings::operator=(src);
}

//-----------------------------------------------------------------------------

int StrList::add(const char* str)
{
    return add(str, NULL);
}

//-----------------------------------------------------------------------------

int StrList::add(const char* str, POINTER data)
{
    int result;

    if (!isSorted_)
        result = count_;
    else
    {
        if (find(str, result))
            switch (dupMode_)
            {
            case DM_IGNORE:
                return result;
            case DM_ERROR:
                error(SEM_DUPLICATE_STRING, 0);
            default:
                break;
            }
    }

    insertItem(result, str, data);
    return result;
}

//-----------------------------------------------------------------------------

void StrList::clear()
{
    internalClear();
}

//-----------------------------------------------------------------------------

void StrList::del(int index)
{
    if (index < 0 || index >= count_)
        error(SEM_LIST_INDEX_ERROR, index);

    onChanging();

    delete list_[index].str;
    list_[index].str = NULL;

    count_--;
    if (index < count_)
    {
        memmove(list_ + index, list_ + index + 1, (count_ - index) * sizeof(StringItem));
        list_[count_].str = NULL;
    }

    onChanged();
}

//-----------------------------------------------------------------------------

void StrList::exchange(int index1, int index2)
{
    if (index1 < 0 || index1 >= count_)
        error(SEM_LIST_INDEX_ERROR, index1);
    if (index2 < 0 || index2 >= count_)
        error(SEM_LIST_INDEX_ERROR, index2);

    onChanging();
    exchangeItems(index1, index2);
    onChanged();
}

//-----------------------------------------------------------------------------

int StrList::indexOf(const char* str) const
{
    int result;

    if (!isSorted_)
        result = Strings::indexOf(str);
    else if (!find(str, result))
        result = -1;

    return result;
}

//-----------------------------------------------------------------------------

void StrList::insert(int index, const char* str)
{
    insert(index, str, NULL);
}

//-----------------------------------------------------------------------------

void StrList::insert(int index, const char* str, POINTER data)
{
    if (isSorted_)
        error(SEM_SORTED_LIST_ERROR, 0);
    if (index < 0 || index > count_)
        error(SEM_LIST_INDEX_ERROR, index);

    insertItem(index, str, data);
}

//-----------------------------------------------------------------------------

void StrList::setCapacity(int value)
{
    if (value < 0) value = 0;

    for (int i = value; i < capacity_; i++)
        delete list_[i].str;

    if (value > 0)
    {
        StringItem *p = (StringItem*)realloc(list_, value * sizeof(StringItem));
        if (p)
            list_ = p;
        else
            iseThrowMemoryException();
    }
    else
    {
        if (list_)
        {
            free(list_);
            list_ = NULL;
        }
    }

    if (list_ != NULL)
    {
        for (int i = capacity_; i < value; i++)
        {
            list_[i].str = NULL;   // new string object when needed
            list_[i].data = NULL;
        }
    }

    capacity_ = value;
    if (count_ > capacity_)
        count_ = capacity_;
}

//-----------------------------------------------------------------------------

POINTER StrList::getData(int index) const
{
    if (index < 0 || index >= count_)
        error(SEM_LIST_INDEX_ERROR, index);

    return list_[index].data;
}

//-----------------------------------------------------------------------------

void StrList::setData(int index, POINTER data)
{
    if (index < 0 || index >= count_)
        error(SEM_LIST_INDEX_ERROR, index);

    onChanging();
    list_[index].data = data;
    onChanged();
}

//-----------------------------------------------------------------------------

const string& StrList::getString(int index) const
{
    if (index < 0 || index >= count_)
        error(SEM_LIST_INDEX_ERROR, index);

    return stringObjectNeeded(index);
}

//-----------------------------------------------------------------------------

void StrList::setString(int index, const char* value)
{
    if (isSorted_)
        error(SEM_SORTED_LIST_ERROR, 0);
    if (index < 0 || index >= count_)
        error(SEM_LIST_INDEX_ERROR, index);

    onChanging();
    stringObjectNeeded(index) = value;
    onChanged();
}

//-----------------------------------------------------------------------------

bool StrList::find(const char* str, int& index) const
{
    if (!isSorted_)
    {
        index = indexOf(str);
        return (index >= 0);
    }

    bool result = false;
    int l, h, i, c;

    l = 0;
    h = count_ - 1;
    while (l <= h)
    {
        i = (l + h) >> 1;
        c = compareStrings(stringObjectNeeded(i).c_str(), str);
        if (c < 0)
            l = i + 1;
        else
        {
            h = i - 1;
            if (c == 0)
            {
                result = true;
                if (dupMode_ != DM_ACCEPT)
                    l = i;
            }
        }
    }

    index = l;
    return result;
}

//-----------------------------------------------------------------------------

int stringListCompareProc(const StrList& stringList, int index1, int index2)
{
    return stringList.compareStrings(
        stringList.stringObjectNeeded(index1).c_str(),
        stringList.stringObjectNeeded(index2).c_str());
}

//-----------------------------------------------------------------------------

void StrList::sort()
{
    sort(stringListCompareProc);
}

//-----------------------------------------------------------------------------

void StrList::sort(StringListCompareProc compareProc)
{
    if (!isSorted_ && count_ > 1)
    {
        onChanging();
        quickSort(0, count_ - 1, compareProc);
        onChanged();
    }
}

//-----------------------------------------------------------------------------

void StrList::setSorted(bool value)
{
    if (value != isSorted_)
    {
        if (value) sort();
        isSorted_ = value;
    }
}

//-----------------------------------------------------------------------------

void StrList::setCaseSensitive(bool value)
{
    if (value != isCaseSensitive_)
    {
        isCaseSensitive_ = value;
        if (isSorted_) sort();
    }
}

//-----------------------------------------------------------------------------

StrList& StrList::operator = (const StrList& rhs)
{
    if (this != &rhs)
        assign(rhs);
    return *this;
}

//-----------------------------------------------------------------------------

void StrList::setUpdateState(bool isUpdating)
{
    if (isUpdating)
        onChanging();
    else
        onChanged();
}

//-----------------------------------------------------------------------------

int StrList::compareStrings(const char* str1, const char* str2) const
{
    if (isCaseSensitive_)
        return strcmp(str1, str2);
    else
        return compareText(str1, str2);
}

//-----------------------------------------------------------------------------

void StrList::insertItem(int index, const char* str, POINTER data)
{
    onChanging();
    if (count_ == capacity_) grow();
    if (index < count_)
    {
        memmove(list_ + index + 1, list_ + index, (count_ - index) * sizeof(StringItem));
        list_[index].str = NULL;
    }

    stringObjectNeeded(index) = str;
    list_[index].data = data;

    count_++;
    onChanged();
}

//-----------------------------------------------------------------------------

void StrList::init()
{
    list_ = NULL;
    count_ = 0;
    capacity_ = 0;
    dupMode_ = DM_IGNORE;
    isSorted_ = false;
    isCaseSensitive_ = false;
}

//-----------------------------------------------------------------------------

void StrList::internalClear()
{
    setCapacity(0);
}

//-----------------------------------------------------------------------------

string& StrList::stringObjectNeeded(int index) const
{
    if (list_[index].str == NULL)
        list_[index].str = new string();
    return *(list_[index].str);
}

//-----------------------------------------------------------------------------

void StrList::exchangeItems(int index1, int index2)
{
    StringItem temp;

    temp = list_[index1];
    list_[index1] = list_[index2];
    list_[index2] = temp;
}

//-----------------------------------------------------------------------------

void StrList::grow()
{
    int delta;

    if (capacity_ > 64)
        delta = capacity_ / 4;
    else if (capacity_ > 8)
        delta = 16;
    else
        delta = 4;

    setCapacity(capacity_ + delta);
}

//-----------------------------------------------------------------------------

void StrList::quickSort(int l, int r, StringListCompareProc compareProc)
{
    int i, j, p;

    do
    {
        i = l;
        j = r;
        p = (l + r) >> 1;
        do
        {
            while (compareProc(*this, i, p) < 0) i++;
            while (compareProc(*this, j, p) > 0) j--;
            if (i <= j)
            {
                exchangeItems(i, j);
                if (p == i)
                    p = j;
                else if (p == j)
                    p = i;
                i++;
                j--;
            }
        }
        while (i <= j);

        if (l < j)
            quickSort(l, j, compareProc);
        l = i;
    }
    while (i < r);
}

///////////////////////////////////////////////////////////////////////////////
// class Url

Url::Url(const string& url)
{
    setUrl(url);
}

Url::Url(const Url& src)
{
    (*this) = src;
}

//-----------------------------------------------------------------------------

void Url::clear()
{
    protocol_.clear();
    host_.clear();
    port_.clear();
    path_.clear();
    fileName_.clear();
    bookmark_.clear();
    userName_.clear();
    password_.clear();
    params_.clear();
}

//-----------------------------------------------------------------------------

Url& Url::operator = (const Url& rhs)
{
    if (this == &rhs) return *this;

    protocol_ = rhs.protocol_;
    host_ = rhs.host_;
    port_ = rhs.port_;
    path_ = rhs.path_;
    fileName_ = rhs.fileName_;
    bookmark_ = rhs.bookmark_;
    userName_ = rhs.userName_;
    password_ = rhs.password_;
    params_ = rhs.params_;

    return *this;
}

//-----------------------------------------------------------------------------

string Url::getUrl() const
{
    const char SEP_CHAR = '/';
    string result;

    if (!protocol_.empty())
        result = protocol_ + "://";

    if (!userName_.empty())
    {
        result += userName_;
        if (!password_.empty())
            result = result + ":" + password_;
        result += "@";
    }

    result += host_;

    if (!port_.empty())
    {
        if (sameText(protocol_, "HTTP"))
        {
            if (port_ != "80")
                result = result + ":" + port_;
        }
        else if (sameText(protocol_, "HTTPS"))
        {
            if (port_ != "443")
                result = result + ":" + port_;
        }
        else if (sameText(protocol_, "FTP"))
        {
            if (port_ != "21")
                result = result + ":" + port_;
        }
    }

    // path and filename
    string str = path_;
    if (!str.empty() && str[str.length()-1] != SEP_CHAR)
        str += SEP_CHAR;
    str += fileName_;

    if (!str.empty())
    {
        if (!result.empty() && str[0] == SEP_CHAR) str.erase(0, 1);
        if (!host_.empty() && result[result.length()-1] != SEP_CHAR)
            result += SEP_CHAR;
        result += str;
    }

    if (!params_.empty())
        result = result + "?" + params_;

    if (!bookmark_.empty())
        result = result + "#" + bookmark_;

    return result;
}

//-----------------------------------------------------------------------------

string Url::getUrl(UINT parts)
{
    Url url(*this);

    if (!(parts & URL_PROTOCOL)) url.setProtocol("");
    if (!(parts & URL_HOST)) url.setHost("");
    if (!(parts & URL_PORT)) url.setPort("");
    if (!(parts & URL_PATH)) url.setPath("");
    if (!(parts & URL_FILENAME)) url.setFileName("");
    if (!(parts & URL_BOOKMARK)) url.setBookmark("");
    if (!(parts & URL_USERNAME)) url.setUserName("");
    if (!(parts & URL_PASSWORD)) url.setPassword("");
    if (!(parts & URL_PARAMS)) url.setParams("");

    return url.getUrl();
}

//-----------------------------------------------------------------------------

void Url::setUrl(const string& value)
{
    clear();

    string url(value);
    if (url.empty()) return;

    // get the bookmark
    string::size_type pos = url.rfind('#');
    if (pos != string::npos)
    {
        bookmark_ = url.substr(pos + 1);
        url.erase(pos);
    }

    // get the parameters
    pos = url.find('?');
    if (pos != string::npos)
    {
        params_ = url.substr(pos + 1);
        url = url.substr(0, pos);
    }

    string buffer;
    pos = url.find("://");
    if (pos != string::npos)
    {
        protocol_ = url.substr(0, pos);
        url.erase(0, pos + 3);
        // get the user name, password, host and the port number
        buffer = fetchStr(url, '/', true);
        // get username and password
        pos = buffer.find('@');
        if (pos != string::npos)
        {
            password_ = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            userName_ = fetchStr(password_, ':');
            if (userName_.empty())
                password_.clear();
        }
        // get the host and the port number
        string::size_type p1, p2;
        if ((p1 = buffer.find('[')) != string::npos &&
            (p2 = buffer.find(']')) != string::npos &&
            p2 > p1)
        {
            // this is for IPv6 Hosts
            host_ = fetchStr(buffer, ']');
            fetchStr(host_, '[');
            fetchStr(buffer, ':');
        }
        else
            host_ = fetchStr(buffer, ':', true);
        port_ = buffer;
        // get the path
        pos = url.rfind('/');
        if (pos != string::npos)
        {
            path_ = "/" + url.substr(0, pos + 1);
            url.erase(0, pos + 1);
        }
        else
            path_ = "/";
    }
    else
    {
        // get the path
        pos = url.rfind('/');
        if (pos != string::npos)
        {
            path_ = url.substr(0, pos + 1);
            url.erase(0, pos + 1);
        }
    }

    // get the filename
    fileName_ = url;
}

///////////////////////////////////////////////////////////////////////////////
// class Packet

Packet::Packet()
{
    init();
}

//-----------------------------------------------------------------------------

Packet::~Packet()
{
    clear();
}

//-----------------------------------------------------------------------------
// ����: ���ݴ��
// ����:
//   true  - �ɹ�
//   false - ʧ��
// ��ע:
//   ���������ݿ��� Packet.getBuffer �� Packet.getSize ȡ����
//-----------------------------------------------------------------------------
bool Packet::pack()
{
    bool result;

    try
    {
        delete stream_;
        stream_ = new MemoryStream(DEFAULT_MEMORY_DELTA);
        doPack();
        doAfterPack();
        doCompress();
        doEncrypt();
        isAvailable_ = true;
        isPacked_ = true;
        result = true;
    }
    catch (Exception&)
    {
        result = false;
    }

    return result;
}

//-----------------------------------------------------------------------------
// ����: ���ݽ��
// ����:
//   true  - �ɹ�
//   false - ʧ�� (���ݲ��㡢��ʽ�����)
//-----------------------------------------------------------------------------
bool Packet::unpack(void *buffer, int bytes)
{
    bool result;

    try
    {
        delete stream_;
        stream_ = new MemoryStream(DEFAULT_MEMORY_DELTA);
        stream_->setSize(bytes);
        memmove(stream_->getMemory(), buffer, bytes);
        doDecrypt();
        doDecompress();
        doUnpack();
        isAvailable_ = true;
        isPacked_ = false;
        result = true;
    }
    catch (Exception&)
    {
        result = false;
        clear();
    }

    delete stream_;
    stream_ = NULL;

    return result;
}

//-----------------------------------------------------------------------------
// ����: ���ݽ��
//-----------------------------------------------------------------------------
bool Packet::unpack(const Buffer& buffer)
{
    return unpack(buffer.data(), buffer.getSize());
}

//-----------------------------------------------------------------------------
// ����: ������ݰ�����
//-----------------------------------------------------------------------------
void Packet::clear()
{
    delete stream_;
    stream_ = NULL;
    isAvailable_ = false;
    isPacked_ = false;
}

//-----------------------------------------------------------------------------
// ����: ȷ�������Ѵ��
//-----------------------------------------------------------------------------
void Packet::ensurePacked()
{
    if (!IsPacked()) pack();
}

//-----------------------------------------------------------------------------

void Packet::throwUnpackError()
{
    iseThrowException(SEM_PACKET_UNPACK_ERROR);
}

//-----------------------------------------------------------------------------

void Packet::throwPackError()
{
    iseThrowException(SEM_PACKET_PACK_ERROR);
}

//-----------------------------------------------------------------------------

void Packet::checkUnsafeSize(int value)
{
    const int MAX_UNSAFE_SIZE = 1024*1024*8;  // 8M

    if (value < 0 || value > MAX_UNSAFE_SIZE)
        iseThrowException(SEM_UNSAFE_VALUE_IN_PACKET);
}

//-----------------------------------------------------------------------------

void Packet::readBuffer(void *buffer, int bytes)
{
    if (stream_->read(buffer, bytes) != bytes)
        throwUnpackError();
}

//-----------------------------------------------------------------------------

void Packet::readString(string& str)
{
    DWORD size;

    str.clear();
    if (stream_->read(&size, sizeof(DWORD)) == sizeof(DWORD))
    {
        checkUnsafeSize(size);
        if (size > 0)
        {
            str.resize(size);
            if (stream_->read((void*)str.data(), size) != static_cast<int>(size))
                throwUnpackError();
        }
    }
    else
    {
        throwUnpackError();
    }
}

//-----------------------------------------------------------------------------

void Packet::readBlob(string& str)
{
    readString(str);
}

//-----------------------------------------------------------------------------

void Packet::readBlob(Stream& stream)
{
    string str;

    readBlob(str);
    stream.setSize(0);
    if (!str.empty())
        stream.write((void*)str.c_str(), (int)str.length());
}

//-----------------------------------------------------------------------------

void Packet::readBlob(Buffer& buffer)
{
    string str;

    readBlob(str);
    buffer.setSize(0);
    if (!str.empty())
        buffer.assign((void*)str.c_str(), (int)str.length());
}

//-----------------------------------------------------------------------------

void Packet::writeBuffer(const void *buffer, int bytes)
{
    ISE_ASSERT(buffer && bytes >= 0);
    stream_->write(buffer, bytes);
}

//-----------------------------------------------------------------------------

void Packet::writeString(const string& str)
{
    DWORD size;

    size = (DWORD)str.length();
    stream_->write(&size, sizeof(DWORD));
    if (size > 0)
        stream_->write((void*)str.c_str(), size);
}

//-----------------------------------------------------------------------------

void Packet::writeBlob(void *buffer, int bytes)
{
    DWORD size = 0;

    if (buffer && bytes >= 0)
        size = (DWORD)bytes;
    stream_->write(&size, sizeof(DWORD));
    if (size > 0)
        stream_->write(buffer, size);
}

//-----------------------------------------------------------------------------

void Packet::writeBlob(const Buffer& buffer)
{
    writeBlob(buffer.data(), buffer.getSize());
}

//-----------------------------------------------------------------------------
// ����: �̶��ַ����ĳ���
//-----------------------------------------------------------------------------
void Packet::fixStrLength(string& str, int length)
{
    if ((int)str.length() != length)
        str.resize(length, 0);
}

//-----------------------------------------------------------------------------
// ����: �����ַ�������󳤶�
//-----------------------------------------------------------------------------
void Packet::truncString(string& str, int maxLength)
{
    if ((int)str.length() > maxLength)
        str.resize(maxLength);
}

//-----------------------------------------------------------------------------

void Packet::init()
{
    stream_ = NULL;
    isAvailable_ = false;
    isPacked_ = false;
}

///////////////////////////////////////////////////////////////////////////////
// class Logger

Logger::Logger() :
    isNewFileDaily_(false)
{
    // nothing
}

//-----------------------------------------------------------------------------
// ����: ������־�ļ���
// ����:
//   fileName   - ��־�ļ��� (��·��)
//   isNewFileDaily - ���Ϊtrue�������Զ����ļ�����(��׺��ǰ)���ϵ��������
//-----------------------------------------------------------------------------
void Logger::setFileName(const string& fileName, bool isNewFileDaily)
{
    fileName_ = fileName;
    isNewFileDaily_ = isNewFileDaily;
}

//-----------------------------------------------------------------------------
// ����: ���ı�д����־
//-----------------------------------------------------------------------------
void Logger::writeStr(const char *str)
{
    string text;
    UINT processId, threadId;

#ifdef ISE_WINDOWS
    processId = GetCurrentProcessId();
    threadId = GetCurrentThreadId();
#endif
#ifdef ISE_LINUX
    processId = getpid();
    threadId = pthread_self();
#endif

    text = formatString("[%s](%05d|%05u) %s%s",
        DateTime::now().toDateTimeString().c_str(),
        processId, threadId, str, S_CRLF);

    writeToFile(text);
}

//-----------------------------------------------------------------------------
// ����: ���ı�д����־
//-----------------------------------------------------------------------------
void Logger::writeFmt(const char *format, ...)
{
    string text;

    va_list argList;
    va_start(argList, format);
    formatStringV(text, format, argList);
    va_end(argList);

    writeStr(text.c_str());
}

//-----------------------------------------------------------------------------
// ����: ���쳣��Ϣд����־
//-----------------------------------------------------------------------------
void Logger::writeException(const Exception& e)
{
    writeStr(e.makeLogStr().c_str());
}

//-----------------------------------------------------------------------------

string Logger::getLogFileName()
{
    string result = fileName_;

    if (result.empty())
        result = getAppPath() + "log.txt";

    if (isNewFileDaily_)
    {
        string fileExt = extractFileExt(result);
        result = result.substr(0, result.length() - fileExt.length()) + ".";
        result += DateTime::now().toDateString("");
        result += fileExt;
    }

    return result;
}

//-----------------------------------------------------------------------------

bool Logger::openFile(FileStream& fileStream, const string& fileName)
{
    return
        fileStream.open(fileName, FM_OPEN_WRITE | FM_SHARE_DENY_NONE) ||
        fileStream.open(fileName, FM_CREATE | FM_SHARE_DENY_NONE);
}

//-----------------------------------------------------------------------------
// ����: ���ַ���д���ļ�
//-----------------------------------------------------------------------------
void Logger::writeToFile(const string& str)
{
    AutoLocker locker(mutex_);

    string fileName = getLogFileName();
    FileStream fs;
    if (!openFile(fs, fileName))
    {
        string path = extractFilePath(fileName);
        if (!path.empty())
        {
            forceDirectories(path);
            openFile(fs, fileName);
        }
    }

    if (fs.isOpen())
    {
        fs.seek(0, SO_END);
        fs.write(str.c_str(), (int)str.length());
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ise
