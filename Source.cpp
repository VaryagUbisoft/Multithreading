#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>


using namespace std;



#define SYNCHRONIZE_THREADS


class ThreadInfo
{
public:

    ThreadInfo()
    {
#ifdef SYNCHRONIZE_THREADS
        InitializeCriticalSection(&m_criticalSection);
#endif
    }

    ~ThreadInfo()
    {
#ifdef SYNCHRONIZE_THREADS
        DeleteCriticalSection(&m_criticalSection);
#endif
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
        delete m_threadName;
            m_threadName = new char[strlen(threadName)];
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

#ifdef SYNCHRONIZE_THREADS
    CRITICAL_SECTION m_criticalSection;
#endif

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

        cout << "Thread " << m_threadInfo->GetThreadThreadName() << " is running" << " with Id " << m_threadInfo->GetThreadId() << "\n\n";
    }

    ~WriterThread()
    {
        cout << "Thread " << m_threadInfo->GetThreadThreadName() << " with Id " << m_threadInfo->GetThreadId() << "terminated" <<"\n\n";
    }

private:

    ThreadInfo* m_threadInfo;

    int m_threadId;
    char* m_threadName;

};


class ReaderThread
{
public:

    ReaderThread(ThreadInfo* threadInfo) : m_threadInfo(threadInfo) {}
    ReaderThread(ThreadInfo* threadInfo, const std::vector<HANDLE> threadsToWaitOn) : m_threadInfo(threadInfo), m_threadsToWaitOn(threadsToWaitOn) {}

    static unsigned __stdcall StaticEntryPoint(void* pObj)
    {
        ReaderThread* thread = static_cast<ReaderThread*>(pObj);
        thread->EntryPoint();

        return 1;
    }

    void EntryPoint()
    {
        if (!m_threadsToWaitOn.empty())
        {
            for (auto it_threadHndl = m_threadsToWaitOn.begin(); it_threadHndl != m_threadsToWaitOn.end(); ++it_threadHndl)
            {
                WaitForSingleObject(*it_threadHndl, INFINITE);
                m_threadsToWaitOn.erase(it_threadHndl);
            }
        }


        cout << "Thread " << m_threadInfo->GetThreadThreadName() << " is running" << " with Id " << m_threadInfo->GetThreadId() << "\n\n";
    }

private:

    ThreadInfo* m_threadInfo;
    std::vector<HANDLE> m_threadsToWaitOn;

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

    /*
    ReaderThread* t3 = new ReaderThread(threadInfo, {t1H, t2H});
    uintptr_t t3Id = 0;
    HANDLE t3H = (HANDLE)_beginthreadex(0, 0, &ReaderThread::StaticEntryPoint, t3, CREATE_SUSPENDED, &t3Id);
    ResumeThread(t3H);
    WaitForSingleObject(t3H, INFINITE);
    */

    // Let's start our suspended threads

    ResumeThread(t1H);
    ResumeThread(t2H);    
    
    Sleep(100);

    // In order for the main thread not to terminate causing termination of a process before the other two threads started
    // we force it to wait upon t1 and t2
    
    //WaitForSingleObject(t1H, INFINITE);
   // WaitForSingleObject(t2H, INFINITE);
    

    // _beginthreadex doesn't automatically close thread's handle after terminating
    // It must be done manually
    CloseHandle(t1H);
    CloseHandle(t2H);
    //CloseHandle(t3H);

}







