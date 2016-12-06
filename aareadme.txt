Sam Justice
Dr. Poole
CSIS 443-001
6 December 2016

									Final Project
How To Run the Program:

1. Open a command prompt window in the directory containing the "Final_Project.exe" file.
2. Type the name of the program in the command prompt window followed by three parameters:
	• The first parameter specifies the number of threads to be initiated up to a maximum of 100.
	• The second parameter specifies number of iterations that each thread should execute before terminating up to a maximum of 100000000.
	• The third parameter is an integer which if 1 means enable mutex checking before entering the critical section, and if 0 means disable 
	  mutex checking.
3. Press "Enter".
4. When the program is finished executing, it will prompt you to "Enter any key to end execution of this program". Simply enter any key and press "Enter".

A sample command line execution would be "Final_Project.exe 100 100000 1".


PLEASE NOTE:

I was able to achieve race events using 40 threads and 10,000 iterations. I was not always able to achieve race events using fewer numbers of threads or
fewer numbers of iterations.
