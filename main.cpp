#include <process.h>		// for _beginthreadex
#include <windows.h>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#define MAXTHREADCOUNT 100

DWORD WINAPI threadWork(LPVOID);

int enableMutex = false;	//Command line parameter
int count = 0;	//Actual count after update by threads
unsigned long numberIter = 0;	//Command line parameter

// Handles
HANDLE mutex;
HANDLE threads[MAXTHREADCOUNT];
//HANDLE sem;

const std::string title = "Sam Justice - CSIS 443, Fall 2016 - Final Project";

int main(int argc, char * argv[]) {
	// Local variables
	char exit;
	DWORD waitResult;

	// Thread Parameters
	LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
	DWORD stackSize = 0;
	int theArg;
	DWORD dwCreationFlags = 0;
	DWORD targetThreadID;

	std::cout << std::endl << "Running Final Project . . . " << std::endl;
	std::cout << title << std::endl << std::endl << std::endl;

	// Get the command line parameters

	int numberOfThreads = atoi(argv[1]);
	if (numberOfThreads < 0 || numberOfThreads > 100)
	{
		std::cout << "Number of threads must be between 0 and 100 (inclusive). Please exit and try again." << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return 0;
	}

	numberIter = atoi(argv[2]);
	if (numberIter < 0 || numberIter > 100000000)
	{
		std::cout << "Number of iterations must be between 0 and 100000000 (inclusive). Please exit and try again." << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return 0;
	}
	
	enableMutex = atoi(argv[3]);
	if (enableMutex != 0 && enableMutex != 1)
	{
		std::cout << "Enable mutex must be a 0 or 1. Please exit and try again." << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return 0;
	}

	int theoreticalCount = ((numberOfThreads * 10) * numberIter) * numberOfThreads;

	if (enableMutex)
	{
		mutex = CreateMutex(NULL, FALSE, NULL);
	}

	//When using Semaphore Method
	//sem = CreateSemaphore(NULL, 0, numberOfThreads, 0);

	// For each thread
	for (int i = 0; i < numberOfThreads; i++) {

		threads[i] = (HANDLE) _beginthreadex(lpThreadAttributes, stackSize, (unsigned(_stdcall *) (void *)) &threadWork, (void *) &numberOfThreads, dwCreationFlags, NULL);
		Sleep(10);	// Let the new thread run
	}

	// Waiting for Multiple Threads Method
	/*waitResult = WaitForMultipleObjects(numberOfThreads, threads, TRUE, INFINITE);

	switch (waitResult)
	{
		case WAIT_OBJECT_0:
		{
			//Output the actual value of count and the theoretical value
			std::cout << "The value of count is: " << count << std::endl;
			std::cout << "The theoretical value of count should be: " << theoreticalCount << std::endl;

			for (int i = 0; i < numberOfThreads; i++)
				CloseHandle(threads[i]);

			CloseHandle(mutex);

			std::cout << "Enter any key to end execution of this program   . . .   ";
			std::cin >> exit;                                             //to pause program

			return 0;
		}
		case WAIT_FAILED:
		case WAIT_ABANDONED:
		{
			return 1;
		}
		default:
			break;
	}*/

	// Loop Method
	for (int i = 0; i < numberOfThreads; i++)
	{
		waitResult = WaitForSingleObject(threads[i], INFINITE);

		switch (waitResult)
		{
			case WAIT_FAILED:
			case WAIT_ABANDONED:
			{
				std::cout << "ERROR: Thread " << GetThreadId(threads[i]) << " did not return successfully. "
					<< "Please exit and try again." << std::endl;

				std::cout << "Enter any key to end execution of this program   . . .   ";
				std::cin >> exit;                                             //to pause program

				return 1;
			}
			default:
				break;
		}
	}

	// Semaphore Method
	/*for (int i = 0; i < numberOfThreads; i++)
	{
		waitResult = WaitForSingleObject(sem, INFINITE);
		
		switch (waitResult)
		{
			case WAIT_FAILED:
			case WAIT_ABANDONED:
			{
				std::cout << "ERROR: A Thread did not properly release a semaphore. Please exit and try again." << std::endl;

				std::cout << "Enter any key to end execution of this program   . . .   ";
				std::cin >> exit;                                             //to pause program

				return 1;
			}
			default:
				break;
		}
	}*/

	//Output the actual value of count and the theoretical value
	std::cout << "The value of count is: " << count << std::endl;
	std::cout << "The theoretical value of count should be: " << theoreticalCount << std::endl;

	for (int i = 0; i < numberOfThreads; i++)
		CloseHandle(threads[i]);

	CloseHandle(mutex);

	std::cout << "Enter any key to end execution of this program   . . .   ";
	std::cin >> exit;                                             //to pause program

	return 0;
}

DWORD WINAPI threadWork(LPVOID threadNo)
{
	DWORD waitResult;
	int* intThreadNo = (int*)threadNo;
	
	std::cout << "A worker thread has been created with an ID of: " << GetCurrentThreadId() << std::endl;

	// Create a CPU load by burning some cycles. Hope to cause contention
	for (unsigned int i = 0; i < numberIter; i++) {
		//<Prepare to enter critical section.Check mutex if enableMutex is true.You provide this code.>
		if (enableMutex == true)
		{
			waitResult = WaitForSingleObject(mutex, INFINITE);

			switch (waitResult)
			{
				case WAIT_OBJECT_0:
				{
					for (int j = 0; j < 10; j++)
					{
						count = count + abs(*intThreadNo);  //Update the count
					}
					
					//<Exit critical section.You provide this code.>
					if (!ReleaseMutex(mutex))
					{
						return false;
					}
					
					break;
				}
				case WAIT_FAILED:
				case WAIT_ABANDONED:
				{
					return false;
				}
				default:
					break;
			}
		}
		else
		{
			// Critical section. The following loop is the CS. Do not change its’ code.  
			for (int j = 0; j<10; j++)
				count = count + abs(*intThreadNo);  //Update the count
		}
	}

	std::cout << "Thread " << GetCurrentThreadId() << " has terminated. " << std::endl;

	//For Semaphore Method
	/*if (!ReleaseSemaphore(sem, 1, NULL))
	{
		return false;
	}*/
	
	// notify main this thread is done
	return 0;
}
