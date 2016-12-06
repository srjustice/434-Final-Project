#include <process.h>		// for _beginthreadex
#include <windows.h>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#define MAXTHREADCOUNT 100
#define MAXITERATIONS 100000000
#define NUMPARAMS 4

DWORD WINAPI threadWork(LPVOID);

//Command Line Parameters
int enableMutex = false;		//Whether or not to enable the mutex
int count = 0;					//Actual count after update by threads
unsigned long numberIter = 0;	//Number of iterations each thread should perform

//Global handle to the mutex
HANDLE countMutex;

//Program Header
const std::string title = "Sam Justice - CSIS 443, Fall 2016 - Final Project";

int main(int argc, char * argv[]) 
{
	//Local variables
	char exit;
	int theoreticalCount = 0;
	DWORD waitResult;
	DWORD exitCode;
	HANDLE threads[MAXTHREADCOUNT];
	enum threadExitCodes { success = 0, waitFailure = -1, releaseMutexFailure = -2 };
	std::stringstream message;

	//Thread Parameters
	LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
	DWORD stackSize = 0;
	int theArg;
	DWORD dwCreationFlags = 0;
	DWORD targetThreadID;

	//Output Header
	std::cout << std::endl << "Running Final Project . . . " << std::endl;
	std::cout << title << std::endl << std::endl << std::endl;

	//Verify the correct number of command line parameters were passed into the program
	if (argc != NUMPARAMS)
	{
		std::cout << "Incorrect number of command line parameters passed into the program. "
			<< "This program takes three parameters." << std::endl << "Please exit and try again." 
			<< std::endl << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return -1;
	}

	//Get the command line parameters and verify they are in the proper bounds
	int numberOfThreads = atoi(argv[1]);
	if (numberOfThreads < 0 || numberOfThreads > MAXTHREADCOUNT)
	{
		std::cout << "Number of threads must be between 0 and 100 (inclusive). Please exit and try again." 
			<< std::endl << std::endl;

		std::cout << "Enter any key to end execution of this program   . . .   ";
		std::cin >> exit;                                             //to pause program

		return -1;
	}

	numberIter = atoi(argv[2]);
	if (numberIter < 0 || numberIter > MAXITERATIONS)
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

		//If creation of the mutex failed, exit the program
		if (countMutex == NULL)
		{
			std::cout << "Creation of the mutex failed. Please exit and try again." << std::endl << std::endl;

			std::cout << "Enter any key to end execution of this program   . . .   ";
			std::cin >> exit;                                             //to pause program

			return -1;
		}
	}

	//Create the threads and calculate the theoretical value of count based on the thread IDs
	for (int i = 0; i < numberOfThreads; i++) {
		threads[i] = (HANDLE) _beginthreadex(lpThreadAttributes, stackSize, (unsigned(_stdcall *) (void *)) &threadWork, NULL, (unsigned)dwCreationFlags, (unsigned *)&targetThreadID);
		theoreticalCount = theoreticalCount + ((10 * abs((long) targetThreadID)) * numberIter);
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
				message << std::endl << "ERROR: Thread " << GetThreadId(threads[i]) << " did not return successfully. "
					<< "Please exit and try again." << std::endl << std::endl;
				std::cout << message.str();

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
			switch ((long) exitCode)
			{
				case waitFailure:
				{
					message << std::endl << "ERROR: Thread " << GetThreadId(threads[i])
						<< " failed to successfully obtain the mutex. Please exit and try again."
						<< std::endl << std::endl;
					std::cout << message.str();

					std::cout << "Enter any key to end execution of this program   . . .   ";
					std::cin >> exit;                                             //to pause program

					return -1;
					
					break;
				}
				case releaseMutexFailure:
				{
					message << std::endl << "ERROR: Thread " << GetThreadId(threads[i])
						<< " failed to successfully release the mutex. Please exit and try again."
						<< std::endl << std::endl;
					std::cout << message.str();

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
	std::cout << std::endl << "The value of \"count\" is: " << count << std::endl;
	std::cout << "The value of \"count\" should be: " << theoreticalCount << std::endl;

	//Close each thread
	for (int i = 0; i < numberOfThreads; i++)
		CloseHandle(threads[i]);

	//Close the mutex
	CloseHandle(countMutex);

	//Exit the program
	std::cout << std::endl << "Enter any key to end execution of this program   . . .   ";
	std::cin >> exit;                                             //to pause program

	return 0;
}

DWORD WINAPI threadWork(LPVOID threadNo)
{
	DWORD waitResult;
	enum exitCodes { success = 0, waitFailure = -1, releaseMutexFailure = -2 };
	std::stringstream message;

	//Log a message with the thread's numeric identifier as it is created 
	message << "A worker thread has been created with an ID of: " << GetCurrentThreadId() << std::endl;
	std::cout << message.str();

	//Create a CPU load by burning some cycles
	for (unsigned int i = 0; i < numberIter; i++) {

		if (enableMutex == true)
		{
			//Obtain the mutex before entering the critical section
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

	//Log a message with the thread's numeric identifier as it terminates
	message = std::stringstream();
	message << "Thread " << GetCurrentThreadId() << " has terminated. " << std::endl;
	std::cout << message.str();
	
	//Notify main this thread is done
	return success;
}
