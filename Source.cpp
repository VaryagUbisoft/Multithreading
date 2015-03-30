#include <iostream>
#include <Windows.h>
#include <process.h>

using namespace std;

void thread_entry_point_func(void* p)
{
	cout << "Entered new thread  " << (int*)p << "\n\n";
}


void main()
{
  _beginthread(thread_entry_point_func, 0, (void*)1024);
	Sleep(100);
}

