#include <process.h>		// for _beginthreadex
#include <windows.h>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#define MAXTHREADCOUNT 100

DWORD WINAPI threadWork(LPVOID);

int enableMutex = false;	//Command line parameter
int count = 0;	//Actual count after update by threads
unsigned long numberIter = 0;	//Command line parameter

//Global Handles
HANDLE countMutex;
HANDLE outputMutex;

const std::string title = "Sam Justice - CSIS 443, Fall 2016 - Final Project";

int main(int argc, char * argv[]) {
	//Local variables
	char exit;
	int theoreticalCount = 0;
	DWORD waitResult;
	DWORD exitCode;
	HANDLE threads[MAXTHREADCOUNT];
	enum threadExitCodes { success = 0, waitFailure = -1, releaseMutexFailure = -2 };

	//Thread Parameters
	LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
	DWORD stackSize = 0;
	int theArg;
	DWORD dwCreationFlags = 0;
	DWORD targetThreadID;

	//Output Header
	std::cout << std::endl << "Running Final Project . . . " << std::endl;
	std::cout << title << std::endl << std::endl << std::endl;

	//Get the command line parameters and verify they are in the proper bounds
	int numberOfThreads = atoi(argv[1]);
	if (numberOfThreads < 0 || numberOfThreads > 100)
	{
		std::cout << "Number of threads must be between 0 and 100 (inclusive). Please exit and try again." 
			<< std::endl << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return -1;
	}

	numberIter = atoi(argv[2]);
	if (numberIter < 0 || numberIter > 100000000)
	{
		std::cout << "Number of iterations must be between 0 and 100000000 (inclusive). "
			<< "Please exit and try again." << std::endl << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return -1;
	}
	
	enableMutex = atoi(argv[3]);
	if (enableMutex != 0 && enableMutex != 1)
	{
		std::cout << "Enable mutex must be a 0 or 1. Please exit and try again." << std::endl << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return -1;
	}

	//If the third command line parameter is 1, create a mutex
	if (enableMutex)
	{
		countMutex = CreateMutex(NULL, FALSE, NULL);

		if (countMutex == NULL)
		{
			std::cout << "Creation of a mutex failed. Please exit and try again." << std::endl << std::endl;

			std::cout << "Enter any key to end execution of this program   . . .   ";
			std::cin >> exit;                                             //to pause program

			return -1;
		}
	}

	//Create a mutex that will prevent more than one thread from writing to the screen at once
	outputMutex = CreateMutex(NULL, FALSE, NULL);

	if (outputMutex == NULL)
	{
		std::cout << "Creation of a mutex failed. Please exit and try again." << std::endl << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return -1;
	}

	//Create the threads and calculate the theoretical count value based on the thread IDs
	for (int i = 0; i < numberOfThreads; i++) {
		threads[i] = (HANDLE) _beginthreadex(lpThreadAttributes, stackSize, (unsigned(_stdcall *) (void *)) &threadWork, NULL, (unsigned)dwCreationFlags, (unsigned *)&targetThreadID);
		theoreticalCount = theoreticalCount + ((10 * targetThreadID) * numberIter);
		Sleep(10);	// Let the new thread run
	}

	//Use a loop to wait for each thread object to be signaled (this is necessary because WaitForMultipleObjects can 
	//only wait on a maximum of 64 threads)
	for (int i = 0; i < numberOfThreads; i++)
	{
		waitResult = WaitForSingleObject(threads[i], INFINITE);

		//Verify the WaitForSingleObject function returned successfully
		switch (waitResult)
		{
			case WAIT_FAILED:
			case WAIT_ABANDONED:
			{
				std::cout << std::endl << "ERROR: Thread " << GetThreadId(threads[i]) << " did not return successfully. "
					<< "Please exit and try again." << std::endl << std::endl;

				std::cout << "Enter any key to end execution of this program   . . .   ";
				std::cin >> exit;                                             //to pause program

				return -1;

				break;
			}
			default:
				break;
		}

		//Verify the thread executed successfully
		if (GetExitCodeThread(threads[i], &exitCode) != 0)
		{
			switch ((int) exitCode)
			{
				case waitFailure:
				{
					std::cout << std::endl << "ERROR: Thread " << GetThreadId(threads[i]) 
						<< " failed to successfully obtain the mutex. Please exit and try again." 
						<< std::endl << std::endl;

					std::cout << "Enter any key to end execution of this program   . . .   ";
					std::cin >> exit;                                             //to pause program

					return -1;
					
					break;
				}
				case releaseMutexFailure:
				{
					std::cout << std::endl << "ERROR: Thread " << GetThreadId(threads[i]) 
						<< " failed to successfully release a mutex. Please exit and try again." 
						<< std::endl << std::endl;

					std::cout << "Enter any key to end execution of this program   . . .   ";
					std::cin >> exit;                                             //to pause program

					return -1;
					
					break;
				}
				default:
					break;
			}
		}
	}

	//Output the actual value of count and the theoretical value of count
	std::cout << std::endl << "The value of count is: " << count << std::endl;
	std::cout << "The value of count should be: " << theoreticalCount << std::endl;

	//Close each thread and the mutex
	for (int i = 0; i < numberOfThreads; i++)
		CloseHandle(threads[i]);

	CloseHandle(countMutex);
	CloseHandle(outputMutex);

	//Exit the program
	std::cout << std::endl << "Enter any key to end execution of this program   . . .   ";
	std::cin >> exit;                                             //to pause program

	return 0;
}

DWORD WINAPI threadWork(LPVOID threadNo)
{
	DWORD waitResult;
	enum exitCodes { success = 0, waitFailure = -1, releaseMutexFailure = -2 };
	
	//Obtain output mutex before writing to the console
	WaitForSingleObject(outputMutex, INFINITE);

	std::cout << "A worker thread has been created with an ID of: " << GetCurrentThreadId() << std::endl;

	if (!ReleaseMutex(outputMutex))
		return releaseMutexFailure;

	//Create a CPU load by burning some cycles
	for (unsigned int i = 0; i < numberIter; i++) {

		if (enableMutex == true)
		{
			//Obtain mutex before entering critical section
			waitResult = WaitForSingleObject(countMutex, INFINITE);
			
			switch (waitResult)
			{
				case WAIT_OBJECT_0:
				{
					//Critical section 
					for (int j = 0; j < 10; j++)
						count = count + abs((long) GetCurrentThreadId());  //Update the count
					
					//Exit critical section
					if (!ReleaseMutex(countMutex))
						return releaseMutexFailure;
					
					break;
				}
				case WAIT_FAILED:
				case WAIT_ABANDONED:
				{
					return waitFailure;
				}
				default:
					break;
			}
		}
		else
		{
			//Critical section 
			for (int j = 0; j < 10; j++)
				count = count + abs((long) GetCurrentThreadId());  //Update the count
		}
	}

	//Obtain output mutex before writing to the console
	WaitForSingleObject(outputMutex, INFINITE);

	std::cout << "Thread " << GetCurrentThreadId() << " has terminated. " << std::endl;

	if (!ReleaseMutex(outputMutex))
		return releaseMutexFailure;

	//Notify main this thread is done
	return success;
}
