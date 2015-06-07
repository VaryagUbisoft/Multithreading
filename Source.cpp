#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
#include <ctime>


using namespace std;



#define SYNCHRONIZE_THREADS

// Shared data

class PolarVec
{
public:

    PolarVec() : A(0), phi(0)
    {
#ifdef SYNCHRONIZE_THREADS
        InitializeCriticalSection(&m_criticalSection);
#endif
    }

    ~PolarVec()
    {
#ifdef SYNCHRONIZE_THREADS
        DeleteCriticalSection(&m_criticalSection);
#endif
    }
    

    void SetVec(float amplitude, float angle)
    {
#ifdef SYNCHRONIZE_THREADS
        EnterCriticalSection(&m_criticalSection);
#endif
        A = amplitude;
        phi = angle;
#ifdef SYNCHRONIZE_THREADS
        LeaveCriticalSection(&m_criticalSection);
#endif
    }

    void GetVec(float& amplitude, float& angle)
    {
#ifdef SYNCHRONIZE_THREADS
        EnterCriticalSection(&m_criticalSection);
#endif
        amplitude = A;
        angle = phi;
#ifdef SYNCHRONIZE_THREADS
        LeaveCriticalSection(&m_criticalSection);
#endif
    }

private:

#ifdef SYNCHRONIZE_THREADS
    CRITICAL_SECTION m_criticalSection;
#endif


    float A;
    float phi;

};


// Generic thread for read/write purposes

class ThreadRW
{
public:

    ThreadRW(PolarVec* sharedData, bool isWriterThread) : isWriter(isWriterThread), v(sharedData){}

    static unsigned __stdcall EntryPointTrigger(void* arg)
    {
        ThreadRW* pThreadRW = static_cast<ThreadRW*>(arg);
        pThreadRW->EntryPoint();
        return 1;
    }

    void EntryPoint()
    {
        if (isWriter)
        {
            cout << "Now in WRITER THREAD\n\n";
            for (int i = 0; i < 200000000; ++i)
                v->SetVec(i, i);
        }
        else
        {
            cout << "Now in READER THREAD\n\n";
            float a = -1;
            float phi = -1;
            for (int i = 0; i < 200; ++i)
            {
                v->GetVec(a, phi);
                cout << "Amp: " << a << " Angle: " << phi << "\n\n";
                Sleep(10);
            }
        }
    }

private:

    bool isWriter;
    PolarVec* v;
};

void main()
{
    PolarVec* pv = new PolarVec();

    ThreadRW* tw = new ThreadRW(pv, true);
    ThreadRW* tr = new ThreadRW(pv, false);

    unsigned twID, trID;

    HANDLE twH, trH;

    twH = (HANDLE)_beginthreadex(NULL, 0, ThreadRW::EntryPointTrigger, tw, CREATE_SUSPENDED, &twID);
    trH = (HANDLE)_beginthreadex(NULL, 0, ThreadRW::EntryPointTrigger, tr, CREATE_SUSPENDED, &trID);

    ResumeThread(twH);
    ResumeThread(trH);

    Sleep(100);

    WaitForSingleObject(twH, INFINITE);
    WaitForSingleObject(trH, INFINITE);

    CloseHandle(twH);
    CloseHandle(trH);

    delete tw;
    tw = NULL;

    delete tr;
    tr = NULL;
}

