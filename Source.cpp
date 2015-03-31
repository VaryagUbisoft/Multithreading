#include <iostream>
#include <Windows.h>
#include <process.h>


using namespace std;

/*
void thread_entry_point_func(void* p)
{
	cout << "Entered new thread  " << (int*)p << "\n\n";
}


void main()
{
  _beginthread(thread_entry_point_func, 0, (void*)1024);
	Sleep(100);
}
*/


class Thread
{
public:

    Thread(int threadID = 1) : m_threadID(threadID) {}

    static unsigned __stdcall StaticEntryPoint(void* pObj)
    {
        Thread* thread = static_cast<Thread*>(pObj);
        thread->EntryPoint();

        return 1;
    }

    void EntryPoint()
    {
        cout << "Thread " << m_threadID << " is running" << "\n\n";
    }

private:

    int m_threadID;

};





void main()
{
    Thread* t1 = new Thread(1);
    uintptr_t t1Id = 0;
    uintptr_t t1H = _beginthreadex(0, 0, &Thread::StaticEntryPoint, t1, CREATE_SUSPENDED, &t1Id);

    Thread* t2 = new Thread(2);
    uintptr_t t2Id = 0;
    uintptr_t t2H = _beginthreadex(0, 0, &Thread::StaticEntryPoint, t2, CREATE_SUSPENDED, &t2Id);

    // Let's start our suspended threads

    ResumeThread((HANDLE)t1H);
    ResumeThread((HANDLE)t2H);


    // In order for the main thread not to terminate causing termination of a process before the other two threads started
    // we force it to wait upon t1 and t2
    
    WaitForSingleObject((HANDLE)t1H, INFINITE);
    WaitForSingleObject((HANDLE)t2H, INFINITE);


}







