/*
Authors: Ron Lewinsohn 313291759, Noam Yakar 318418027
Project: Ex2
Description: This is the main module of the project. It includes the main thread function and other high-level 
functions that are relevant to the main thread.
*/
/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>
#include <MemAllocation.h>
#include <Main.h>
#include <FinishProgramHandling.h>

/*Functions */

/* Entry function - calls to the main function
* Parameters: command line arguments
* Returns 0 if program completed
*/
int main(int argc, char* argv[])
{
	main_function(argv);
	printf("PROGRAM FINISHED!!\n");
	return 0;
}

/* Main function
* Parameters: command line arguments
*/
void main_function(char* argv[])
{
	populate_command_args_and_globals(argv); // Put command line arguments and in variables and populate globals
	if (!create_results_directory()) // Create a Results directory
	{ //If results directory creating failed -> finish program
		printf("Exitting program!!\n");
		exit(1);
	}
	handles_t* p_struct_arr = make_handle_struct_alloc(); // Create array of structs of type handles_t
	HANDLE p_thread_handles[10]; // Create array of handles
	DWORD p_thread_ids[10]; // Create array of thread Ids
	int finished_threads_counter = 0;
	int num_threads_per_session = 0;

	// if memory allocation failed, free all memory and close all handles
	if (p_struct_arr == NULL)
	{
		error_handling(p_struct_arr, p_thread_handles, 0);
	}

	// if mutex creation fails, close all handles and free all memory
	if (!(create_mutex()))
	{
		error_handling(p_struct_arr, p_thread_handles, 0);
	}

	// Create thread which calculates the student average grade for each student and write it in the appropriate results file
	// A thread will execute for each school
	// 10 threads can run simultanously
	while (finished_threads_counter < g_num_of_schools)
	{
		// Calculate the num of threads that will run in the following session: the maximum value between 
		// 10 and the number of schools left
		num_threads_per_session = calculate_num_threads_per_session(finished_threads_counter);

		// Threads creation routine
		create_threads(p_struct_arr, p_thread_handles, p_thread_ids, num_threads_per_session, finished_threads_counter);

		//Wait for all active threads to finish their progress
		if (!wait_for_multiple_objects(finished_threads_counter, num_threads_per_session, p_thread_handles))
		{ //if ERROR detected - need to terminate all threads, close handles and free memory
			threads_termination(p_thread_handles, num_threads_per_session);
			error_handling(p_struct_arr, p_thread_handles, num_threads_per_session);
		}

		//Get the exit code per thread
		if (FALSE == get_exit_code_per_thread(num_threads_per_session, finished_threads_counter, p_thread_handles))
		{ 
			printf("ERROR: get_exit_code_per_thread haven't finished properly!\n");
			error_handling(p_struct_arr, p_thread_handles, num_threads_per_session);
		}
		for (int idx = 0; idx < num_threads_per_session; idx++)
		{ // close handles
			printf("Closing handle %d\n", idx);
			if (FALSE == CloseHandle(p_thread_handles[idx])) // close thread handle
			{
				printf("ERROR: CloseHandle of results handle failed!!\n");
				error_handling(p_struct_arr, p_thread_handles, num_threads_per_session);
			}
		}

		// Increase the amount of finished threads
		finished_threads_counter += num_threads_per_session;
	}

	// Close all open handles and free memory allocation
	close_handles_and_free_memory(p_struct_arr, p_thread_handles, 0);
}

/*This function puts the command line arguments into variables and populates global variables
Parameters: The array containing the program arguments*/
void populate_command_args_and_globals(char* argv[])
{
	printf("Populating global parameters\n");
	g_memory_allocation_bitmask = 0;
	g_school_idx_mutex_handle = NULL;
	g_thread_idx = 0;
	printf("Populating command line arguments\n");
	g_num_of_schools = atoi(argv[1]);
	g_weight_real = atoi(argv[2]);
	g_weight_human = atoi(argv[3]);
	g_weight_eng = atoi(argv[4]);
	g_weight_eval = atoi(argv[5]);
	printf("g_number of schools = %d, g_weight_real = %d, g_weight_human = %d, g_weight_eng = %d, g_weight_eval = %d\n",
		g_num_of_schools, g_weight_real, g_weight_human, g_weight_eng, g_weight_eval);
}

/* The function creates a mutex handle
* Returns TRUE if creating succeed, else returns FALSE
*/
BOOL create_mutex()
{
	/* Create the mutex that will be used to synchronize access to count */
	g_school_idx_mutex_handle = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == g_school_idx_mutex_handle)
	{ //mutex creation failed
		printf("ERROR: Error when creating mutex: %d\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

/* The function creates a thread
* Parameters: thread's start routine, thread id and the thread argument
* Returns a thread handle
*/
HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, handles_t* p_struct_arr)
{
	if (NULL == p_start_routine)
	{ // If thread has no start routine
		printf("ERROR: Error when creating a thread");
		printf("Received null pointer");
		return NULL;
	}

	if (NULL == p_thread_id)
	{ // If thread has no thread id
		printf("ERROR: Error when creating a thread");
		printf("Received null pointer");
		return NULL;
	}

	return CreateThread(
		NULL,            /*  default security attributes */
		0,               /*  use default stack size */
		p_start_routine, /*  thread function */
		(void*)p_struct_arr,/*  argument to thread function */
		0,               /*  use default creation flags */
		p_thread_id);    /*  returns the thread identifier */
}

/* The function prints the wait_code received from WaitForMultipleObject
* Parameters: wait_code
*/
void print_multiple_object(DWORD wait_code)
{
	printf("WaitForMultipleObject output: \n");
	switch (wait_code)
	{
	case WAIT_TIMEOUT: //the thread takes too long
		printf("WAIT_TIMEOUT\n"); break;
	case WAIT_FAILED: //WaitForMultipleObject failed
		printf("WAIT_FAILED...\n");
		break;
	default:
		printf("0x%x\n", wait_code);
	}
}

/*This function creates a results directory
Returns TRUE if Results directory created, else FALSE
*/
BOOL create_results_directory()
{
	int validation_check;
	TCHAR directory[RESULTS_LEN] = { 0 };
	_tcscpy(directory, TEXT("Results"));
	if (NULL == directory)
	{ //if directory is still NULL
		printf("ERROR: couldn't create 'Results' directory because directory is NULL\n");
		return FALSE;
	}
	else
	{ // if directory is "Results"
		validation_check = CreateDirectoryW(directory, NULL); //Create the directory
		if (!validation_check)
		{ //If directory creation failed -> return FALSE
			printf("ERROR: Unable to create directory\n");
			return FALSE;
		}
		printf("Results directory created\n");
	}
	// "Results" directory created -> return TRUE
	return TRUE;
}


/* The function calculates the num threads that will run in the following session, the num of threads
* will be the maximum value between 10 (max threads can run simultanously) and amount of remain threads
* Parameters: amount of threads finished their progress.
* Returns: amount of threads that will run in the following session
*/
int calculate_num_threads_per_session(int finished_threads_counter)
{
	int num_threads_per_session = 0;

	// If more than 10 threads left, the next session will run with 10 threads.
	if (finished_threads_counter + MAX_THREADS <= g_num_of_schools)
	{
		num_threads_per_session = MAX_THREADS;
	}
	// else, the next session will run with the amount of remain threads
	else
	{
		num_threads_per_session = g_num_of_schools - finished_threads_counter;
	}
	// Return
	return num_threads_per_session;
}

/* The function creates Threads, the amount of threads will be decided by the num_threads_per_session parameter.
* Parameters: array of structs of file handles, array of threads handles, array of thread ids,
				amount of threads will be created and the amount of threads that have finished their progress till now
*/
void create_threads(handles_t* p_struct_arr, HANDLE* p_thread_handles,DWORD* p_thread_ids,int num_threads_per_session,int finished_threads_counter)
{
	int num_thread_idx;
	// Create thread handles for 10 school or amount of schools left
	for (int num_thread_idx_per_session = 0; num_thread_idx_per_session < num_threads_per_session; num_thread_idx_per_session++)
	{
		//Calculate thread index
		num_thread_idx = finished_threads_counter + num_thread_idx_per_session;
		printf("THREAD %d started its progress\n", num_thread_idx);

		// Create Thread
		p_thread_handles[num_thread_idx_per_session] = create_thread_simple((LPTHREAD_START_ROUTINE)thread_main, &p_thread_ids[num_thread_idx_per_session], p_struct_arr);
		if (NULL == p_thread_handles[num_thread_idx_per_session])
		{ //If create thread failed, terminated all running threads and exit
			printf("ERROR: Couldn't create thread %d\n", num_thread_idx);
			threads_termination(p_thread_handles, num_thread_idx_per_session);
			error_handling(p_struct_arr, p_thread_handles, num_thread_idx_per_session);
		}
	}
}

/* The function does the wait for multiple objects routine
* Parameters: finished threads counter, num of active threads, array of thread handles
* Returns TRUE if all threads were signaled properly, else FALSE
*/
BOOL wait_for_multiple_objects(int finished_threads_counter, int num_threads_per_session, HANDLE* p_thread_handles)
{

	DWORD wait_code = WaitForMultipleObjects(num_threads_per_session, p_thread_handles, TRUE, TIMEOUT_IN_MILLISECONDS);
	printf("finished_threads_counter = %d, num_threads_per_session = %d ", finished_threads_counter, num_threads_per_session);
	print_multiple_object(wait_code); // print wait code
	if (wait_code == WAIT_FAILED)
	{ //If not all threads are signaled
		printf("ERROR: WaitForExitCode failed!\n"
			"Terminating brutally!\n");
		return FALSE;
	}
	return TRUE;
}


/* The function Gets the exit code per active thread and print it to the screen
* Parameters: finished threads counter, num of active threads, array of thread handles
* Returns FALSE if an error occured during the function progress, else FALSE
*/
BOOL get_exit_code_per_thread(int num_threads_per_session, int finished_threads_counter, HANDLE* p_thread_handles)
{
	int num_thread_idx = 0;
	DWORD exitcode, wait_code;
	
	// Get the exit code per active thread and print it to the screen
	for (int num_thread_idx_per_session = 0; num_thread_idx_per_session < num_threads_per_session; num_thread_idx_per_session++)
	{
		// Calculate the thread index
		num_thread_idx = finished_threads_counter + num_thread_idx_per_session;

		//Get thread's exit code
		GetExitCodeThread(p_thread_handles[num_thread_idx_per_session], &exitcode);
		printf("The exit code for the thread %d is 0x%x and it %s\n", num_thread_idx, exitcode, !exitcode ? "succeed" : "FAILED");

		if (exitcode == STILL_ACTIVE)
		{ // Terminate thread if it is still active
			printf("ERROR: Thread %d is still active, terminating thread!\n", num_thread_idx);
			if (TerminateThread(p_thread_handles[num_thread_idx_per_session], BRUTAL_TERMINATION_CODE))
			{ //if termination thread succeed
				wait_code = WaitForSingleObject(p_thread_handles[num_thread_idx_per_session], TIMEOUT_IN_MILLISECONDS); //wait for thread to be signaled
				print_multiple_object(wait_code); //print wait_code
				if (wait_code != WAIT_OBJECT_0)
				{// if wait_code is not 0
					printf("ERROR: wait for single object failed\n");
					return FALSE;
				}
			}
			else
			{ //TerminateThread process failed
				printf("ERROR: TerminateThread function failed!!!\n");
				return FALSE;
			}
		}
		else if (exitcode == 0)
		{
			printf("THREAD %d completed its' progress\n", num_thread_idx);
		}
		else
		{
			printf("ERROR: Thread %d hasn't completed its' progress properly\n", num_thread_idx);
		}
	}
	return TRUE;
}