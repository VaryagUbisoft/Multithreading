#include <iostream>
#include <Windows.h>
#include <process.h>


using namespace std;



#define SYNCHRONIZE_THREADS


class ThreadInfo
{
public:

    ThreadInfo()
    {
        InitializeCriticalSection(&m_criticalSection);
    }

    ~ThreadInfo()
    {
        DeleteCriticalSection(&m_criticalSection);
    }

    void SetThreadId(int threadId)
    {
#ifdef SYNCHRONIZE_THREADS
        EnterCriticalSection(&m_criticalSection);
#endif
        m_threadId = threadId;
#ifdef SYNCHRONIZE_THREADS
        LeaveCriticalSection(&m_criticalSection);
#endif

    }

    int GetThreadId()
    {
#ifdef SYNCHRONIZE_THREADS
        EnterCriticalSection(&m_criticalSection);
#endif
        return m_threadId;
#ifdef SYNCHRONIZE_THREADS
        LeaveCriticalSection(&m_criticalSection);
#endif
    }

    void SetThreadName(char* threadName)
    {
#ifdef SYNCHRONIZE_THREADS
        EnterCriticalSection(&m_criticalSection);
#endif
        m_threadName = threadName;
#ifdef SYNCHRONIZE_THREADS
        LeaveCriticalSection(&m_criticalSection);
#endif

    }

    char* GetThreadThreadName()
    {
#ifdef SYNCHRONIZE_THREADS
        EnterCriticalSection(&m_criticalSection);
#endif
        return m_threadName;
#ifdef SYNCHRONIZE_THREADS
        LeaveCriticalSection(&m_criticalSection);
#endif
    }


private:

    CRITICAL_SECTION m_criticalSection;
    int   m_threadId;
    char* m_threadName;
    
};



class WriterThread
{
public:

    WriterThread(char* threadName, int threadId, ThreadInfo* threadInfo) : m_threadName(threadName), m_threadId(threadId), m_threadInfo(threadInfo){}

    
    static unsigned __stdcall StaticEntryPoint(void* pObj)
    {
        WriterThread* thread = static_cast<WriterThread*>(pObj);
        thread->EntryPoint();

        return 1;
    }

    void EntryPoint()
    {
        m_threadInfo->SetThreadId(m_threadId);
        m_threadInfo->SetThreadName(m_threadName);
    }

private:

    ThreadInfo* m_threadInfo;

    int m_threadId;
    char* m_threadName;

};


class ReaderThread
{
public:

    ReaderThread(ThreadInfo* threadInfo) : m_threadInfo(threadInfo){}

    static unsigned __stdcall StaticEntryPoint(void* pObj)
    {
        ReaderThread* thread = static_cast<ReaderThread*>(pObj);
        thread->EntryPoint();

        return 1;
    }

    void EntryPoint()
    {
        cout << "Thread " << m_threadInfo->GetThreadThreadName() << " is running" << " with Id " << m_threadInfo->GetThreadId() << "\n\n";
    }

private:

    ThreadInfo* m_threadInfo;

};




void main()
{
    ThreadInfo* threadInfo = new ThreadInfo;

    WriterThread* t1 = new WriterThread("WriterThread1", 111, threadInfo);
    uintptr_t t1Id = 0;
    HANDLE t1H = (HANDLE)_beginthreadex(0, 0, &WriterThread::StaticEntryPoint, t1, CREATE_SUSPENDED, &t1Id);

    WriterThread* t2 = new WriterThread("WriterThread2", 222, threadInfo);
    uintptr_t t2Id = 0;
    HANDLE t2H = (HANDLE)_beginthreadex(0, 0, &WriterThread::StaticEntryPoint, t2, CREATE_SUSPENDED, &t2Id);

    ReaderThread* t3 = new ReaderThread(threadInfo);
    uintptr_t t3Id = 0;
    HANDLE t3H = (HANDLE)_beginthreadex(0, 0, &ReaderThread::StaticEntryPoint, t3, CREATE_SUSPENDED, &t3Id);


    // Let's start our suspended threads

    ResumeThread(t1H);
    ResumeThread(t2H);
    ResumeThread(t3H);


    // In order for the main thread not to terminate causing termination of a process before the other two threads started
    // we force it to wait upon t1 and t2
    
    WaitForSingleObject(t1H, INFINITE);
    WaitForSingleObject(t2H, INFINITE);
    WaitForSingleObject(t3H, INFINITE);

    // _beginthreadex doesn't automatically close thread's handle after terminating
    // It must be done manually
    CloseHandle(t1H);
    CloseHandle(t2H);
    CloseHandle(t3H);

}







