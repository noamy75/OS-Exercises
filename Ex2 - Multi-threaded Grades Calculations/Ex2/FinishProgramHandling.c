/*
Description: This module includes functions that are used for finishing the program, such as closing handles and
terminating threads. 
*/

/*Includes*/

#include <HardCodedData.h>
#include <FinishProgramHandling.h>
#include <Main.h>
#include <MemAllocation.h>

/*Functions */

/*This function closes file handles for a specific school
Parameters: A handles_t struct containing file handles*/
void close_struct_idx_handles(handles_t* h_struct)
{
	if (h_struct->active_handles_bitmask & BIT(REAL)) // handle of file 'real' is open
	{
		printf("Closing real handle\n");
		h_struct->active_handles_bitmask &= ~(BIT(REAL)); // turn off bit of 'real'
		if (FALSE == CloseHandle(h_struct->h_real))// close handle of 'real' file
		{
			printf("ERROR: CloseHandle of real handle failed!!\n");
			return;
		}
	}
	if (h_struct->active_handles_bitmask & BIT(HUMAN)) // handle of file 'human' is open
	{
		printf("Closing human handle\n");
		h_struct->active_handles_bitmask &= ~(BIT(HUMAN)); // turn off bit of 'human'
		if (FALSE == CloseHandle(h_struct->h_human))// close handle of 'human' file
		{
			printf("ERROR: CloseHandle of human handle failed!!\n");
			return;
		}
	}
	if (h_struct->active_handles_bitmask & BIT(ENG)) // handle of file 'eng' is open
	{
		printf("Closing eng handle\n");
		h_struct->active_handles_bitmask &= ~(BIT(ENG)); // turn off bit of 'eng'
		if (FALSE == CloseHandle(h_struct->h_eng)) // close handle of 'eng' file
		{
			printf("ERROR: CloseHandle of eng handle failed!!\n");
			return;
		}
	}
	if (h_struct->active_handles_bitmask & BIT(EVAL)) // handle of file 'eval' is open
	{
		printf("Closing eval handle\n");
		h_struct->active_handles_bitmask &= ~(BIT(EVAL)); // turn off bit of 'eval'
		if (FALSE == CloseHandle(h_struct->h_eval)) // close handle of 'eval' file
		{
			printf("ERROR: CloseHandle of eval handle failed!!\n");
			return;
		}
	}
	if (h_struct->active_handles_bitmask & BIT(RESULTS)) // handle of file 'results' is open
	{
		printf("Closing results handle\n");
		h_struct->active_handles_bitmask &= ~(BIT(RESULTS)); // turn off bit of 'results'
		if (FALSE == CloseHandle(h_struct->h_results)) // close handle of 'results' file
		{
			printf("ERROR: CloseHandle of results handle failed!!\n");
			return;
		}
	}
}

/* The function terminates all active threads
* Parameters: thread_handles, first active thread, last active thread
*/
void threads_termination(HANDLE* p_thread_handles, int end_idx)
{
	DWORD wait_code = WAIT_TIMEOUT;
	for (int idx = 0; idx < end_idx; idx++)
	{ //For every active thread - terminate
		if (TerminateThread(p_thread_handles[idx], BRUTAL_TERMINATION_CODE))
		{ //if termination process succeed
			wait_code = WaitForSingleObject(p_thread_handles[idx], TIMEOUT_IN_MILLISECONDS); //wait for thread to be signaled
			print_multiple_object(wait_code); //print wait_code
			if (wait_code != WAIT_OBJECT_0)
			{// if wait_code is not 0
				printf("ERROR: wait for single object failed\n");
				return;
			}
		}
		else
		{
			printf("ERROR: TerminateThread failed!!\n");
			return;
		}
	}
}


/*This function closes all file handles for all schools
Parameters: Array of structs of file handles*/
void close_struct_handles(handles_t* p_struct_arr)
{
	for (int idx = 0; idx < g_num_of_schools; idx++) // For each school
	{
		close_struct_idx_handles(&p_struct_arr[idx]); // Close handles
		printf("All struct handles of thread %d are closed!!\n", idx);
	}
	printf("All struct handles are closed!!\n");
}

/*This function closes mutex handle*/
void close_mutex_handle()
{
	if (NULL != g_school_idx_mutex_handle) // Mutex handle is open
	{
		if (FALSE == CloseHandle(g_school_idx_mutex_handle)) // Close mutex handle
		{
			printf("ERROR: CloseHandle of mutex handle failed!!\n");
			return;
		}
	}
	printf("Mutex handle is closed!!\n");
}

/*This function closes all handles and frees all allocated memory.
Parameters: array of structs of file handles, array of threads handles and number of active handles*/
void close_handles_and_free_memory(handles_t* p_struct_arr, HANDLE* p_thread_handles, int num_active_handles)
{
	printf("close thread handles\n");
	close_thread_handles(p_thread_handles, num_active_handles);
	printf("close struct handles\n");
	close_struct_handles(p_struct_arr); // Close all file handles of all schools
	printf("free all memory\n");
	free_all_memory(p_struct_arr); // Free all allocated memory
	printf("close mutex handle\n");
	close_mutex_handle(); // Close mutex handle
}

/*This function is called when an error occured. It calls to another function that closes all handles and frees
all allocated memory.
Parameters: array of structs of file handles, array of threads handles and num of active handles*/
void error_handling(handles_t* p_struct_arr, HANDLE* p_thread_handles, int num_active_handles)
{
	printf("Error Handling\n");
	// Close all handles and free all allocated memory
	close_handles_and_free_memory(p_struct_arr, p_thread_handles, num_active_handles);
	printf("exitting now!!!!\n");
	exit(1);
}

/* The function closes all thread handles
* Parameter: thread handles and number of active handles
*/
void close_thread_handles(HANDLE* p_thread_handles, int num_active_handles)
{
	for (int num_school_idx = 0; num_school_idx < num_active_handles; num_school_idx++)
	{ //For each thread
		if (FALSE == CloseHandle(p_thread_handles[num_school_idx])) //close thread handle
		{
			printf("ERROR: CloseHandle failed!!\n");
			return;
		}
		printf("Thread handle %d is closed!!!\n", num_school_idx);
	}
	printf("All thread handles are closed!!\n");
}

