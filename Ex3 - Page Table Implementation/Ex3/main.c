/*
Authors: Ron Lewinsohn 313291759, Noam Yakar 318418027
Project: Ex3
Description: This is the main module of the project. It includes the main thread function and other high-level
functions that are relevant to the main thread.

WE IMPLEMENTED LRU.

*/
/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>
#include <main.h>
#include <FileHandling.h>
#include <MemAllocation.h>
#include <Timing.h>
#include <Init.h>

/*Functions */

/* This function creates a thread
* Parameters: thread's start routine, thread id and the thread argument
* Returns a thread handle
*/
HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, thread_input_t* thread_input)
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
		(void*)thread_input,/*  argument to thread function */
		0,               /*  use default creation flags */
		p_thread_id);    /*  returns the thread identifier */
}

/*This function closes a handle
Parameter: handle - the handle to close
*/
void close_handle(HANDLE handle)
{
	if (handle != NULL) // handle is valid
	{
		if (FALSE == CloseHandle(handle)) // Close handle
		{ // closure failed
			printf("ERROR: CloseHandle failed!!\n");
			return;
		}
	}
}

/*This function closes all thread handles that were opened so far in the program
Parameters: p_thread_handles - array of threads handles, num_of_threads - number of total threads
*/
void close_thread_handles(HANDLE* p_thread_handles, int num_of_threads)
{
	for (int i = 0; i < num_of_threads; i++)
	{
		close_handle(p_thread_handles[i]);
	}
}

/*This function closes all handles opened in the program
Parameter: h_input - input file handle
*/
void close_all_handles(HANDLE h_input, HANDLE* p_thread_handles, int num_of_threads)
{
	close_handle(h_input); // close input file handle
	close_handle(g_h_output); // close output file handle
	close_handle(g_page_table_mutex); // close page table mutex handle
	close_handle(g_active_frames_semaphore); // close active frames semaphore handle
	close_thread_handles(p_thread_handles, num_of_threads);
}


/*This function is called when an error occurs, it calls functions that close all handles and free all memories
Parameters: handles to close
*/
void error_handling(HANDLE h_input, HANDLE* p_thread_handles, DWORD* p_thread_ids, thread_input_t* thread_input, int num_of_threads)
{
	close_all_handles(h_input, p_thread_handles, num_of_threads);
	free_all_memory(p_thread_handles, p_thread_ids, thread_input);
	exit(1);
}

/* The function prints the wait_code received from WaitForMultipleObject
* Parameters: wait_code
*/
void print_multiple_object(DWORD wait_code)
{
	printf("WaitForMultipleObject output: ");
	switch (wait_code)
	{
	case WAIT_TIMEOUT: // the thread takes too long
		printf("WAIT_TIMEOUT\n"); break;
	case WAIT_FAILED: // WaitForMultipleObject failed
		printf("WAIT_FAILED...\n");
		break;
	default:
		printf("0x%x\n", wait_code);
	}
}

/* The function does the wait for multiple objects routine
* Parameters: number of threads, array of thread handles
* Returns TRUE if all threads were signaled properly, else FALSE
*/
BOOL wait_for_multiple_objects(int threads_amount, HANDLE* p_thread_handles)
{
	unsigned int num_threads_mult_wait = NUM_THREADS_MULT_WAIT;
	int q = threads_amount / NUM_THREADS_MULT_WAIT; // number of "full" groups (60 threads)
	unsigned int r = threads_amount % NUM_THREADS_MULT_WAIT; // the number of threads in the smaller group (less than 60 threads)
	int num_groups = q + 1; // all threads are divided into groups of 60 or less
	if (0 == r)
	{
		num_groups--;
	}
	DWORD wait_code = 0;
	BOOL error_flag = FALSE;
	int finished_groups_counter = 0;

	for (int i = 0; i < num_groups; i++) {
		int index = i * NUM_THREADS_MULT_WAIT;
		if (i < num_groups - 1 || 0 == r) { // for full groups
			wait_code = WaitForMultipleObjects(num_threads_mult_wait, &(p_thread_handles[index]), TRUE, TIMEOUT_IN_MILLISECONDS);
			if (wait_code <= num_threads_mult_wait) { // all threads in this group are signaled
				finished_groups_counter++;
			}
			else {
				error_flag = TRUE;
				break;
			}
		}

		else { // for the smaller group
			wait_code = WaitForMultipleObjects(r, &(p_thread_handles[index]), TRUE, TIMEOUT_IN_MILLISECONDS);
			if (wait_code < r) { // all threads in this group are signaled
				finished_groups_counter++;
			}
			else {
				error_flag = TRUE;
				break;
			}
		}
	}

	if (error_flag) {
		if (wait_code == WAIT_FAILED) {
			printf("ERROR: wait_for_multiple_objects failed!\n"
				"Terminating brutally!\n");
		}
		print_multiple_object(wait_code);
		return FALSE;
	}

	if (finished_groups_counter == num_groups) { // all total threads are signaled
		printf("multiple object wait finished successfully\n");
		print_multiple_object(wait_code);
	}

	return TRUE;
}


/* The function terminates all active threads
* Parameters: thread_handles, first active thread, last active thread
*/
void threads_termination(HANDLE* p_thread_handles, int threads_amount)
{
	DWORD wait_code = WAIT_TIMEOUT;
	for (int idx = 0; idx < threads_amount; idx++)
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

/* This function finds the time when all threads have ended and all frames need to be evicted
Returns the end time
*/
int find_end_time()
{
	int max_time = 0;
	for (int frame_idx = 0; frame_idx < g_num_frames; frame_idx++)
	{
		if (pg_frame_table[frame_idx].mapped_page != INVALID_MAPPING)
		{
			if (g_page_table.page_array[pg_frame_table[frame_idx].mapped_page].finish_time > max_time)
			{
				max_time = g_page_table.page_array[pg_frame_table[frame_idx].mapped_page].finish_time;
			}
		}
	}
	return max_time;
}

/*This function evicts all frames
Parameter: end_time, which is the time of eviction
*/
void clean_frame_table(int end_time)
{
	for (int frame_idx = 0; frame_idx < g_num_frames; frame_idx++)
	{
		if (pg_frame_table[frame_idx].mapped_page != INVALID_MAPPING) // frame contains page
		{
			printf("%d %d %d E\n", end_time, pg_frame_table[frame_idx].mapped_page, frame_idx);
			write_to_file(g_h_output, end_time, pg_frame_table[frame_idx].mapped_page, frame_idx, "E");
			pg_frame_table[frame_idx].mapped_page = INVALID_MAPPING;
			pg_frame_table[frame_idx].is_active = FALSE;
		}
		printf("write the eviction for frame idx %d\n", frame_idx);
	}
}


/* The function Gets the exit code per active thread and print it to the screen
* Parameters: finished threads counter, num of active threads, array of thread handles
* Returns FALSE if an error occured during the function progress, else FALSE
*/
BOOL get_exit_code_per_thread(int num_of_threads, HANDLE* p_thread_handles)
{
	int num_thread_idx = 0;
	DWORD exitcode, wait_code;

	// Get the exit code per active thread and print it to the screen
	for (int num_thread_idx = 0; num_thread_idx < num_of_threads; num_thread_idx++)
	{
		//Get thread's exit code
		GetExitCodeThread(p_thread_handles[num_thread_idx], &exitcode);
		printf("The exit code for the thread %d is 0x%x and it %s\n", num_thread_idx, exitcode, !exitcode ? "succeed" : "FAILED");

		if (exitcode == STILL_ACTIVE)
		{ // Terminate thread if it is still active
			printf("ERROR: Thread %d is still active, terminating thread!\n", num_thread_idx);
			if (TerminateThread(p_thread_handles[num_thread_idx], BRUTAL_TERMINATION_CODE))
			{ //if termination thread succeed
				wait_code = WaitForSingleObject(p_thread_handles[num_thread_idx], TIMEOUT_IN_MILLISECONDS); //wait for thread to be signaled
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

/* main thread function
* parameters: command line arguments
*/
void main_function(char* argv[])
{
	get_num_pages_and_frames(argv);
	HANDLE h_input; HANDLE* p_thread_handles = NULL; DWORD* p_thread_ids = NULL; thread_input_t* p_thread_input = NULL;
	g_page_table_mutex = NULL; g_active_frames_semaphore = NULL; g_h_output = NULL; pg_frame_table = NULL; pg_time_list = NULL;
	g_page_table.page_array = NULL; g_num_elements_in_list = 0;
	int num_of_lines = 0;
	if (!init_vars(argv, &h_input, &num_of_lines, &p_thread_handles, &p_thread_ids, &p_thread_input))
	{ // Init all vars, if one init has failed so finish the program.
		printf("ERROR: init_vars function haven't finished properly\n");
		error_handling(h_input, p_thread_handles, p_thread_ids, p_thread_input, 0);
	}
	if (0 == num_of_lines)
	{
		printf("File is empty\n");
		close_all_handles(h_input, p_thread_handles, num_of_lines); // close all handles
		free_all_memory(p_thread_handles, p_thread_ids, p_thread_input); // free all memory
		return;
	}
	g_arrived_semaphore = g_num_frames;
	for (int i = 0; i < g_num_frames; i++)
	{
		if (pg_frame_table != NULL)
		{ // Frame table objects init, each frame is empty so its' mapped page has invalid mapping
			pg_frame_table[i].mapped_page = INVALID_MAPPING;
		}
	}
	for (int num_thread_idx = 0; num_thread_idx < num_of_lines; num_thread_idx++)
	{ //Get the thread input from the input file and insert the time objects of each thread to the time linked list.
		p_thread_input[num_thread_idx].num_thread = num_thread_idx;
		if (!read_line(h_input, &p_thread_input[num_thread_idx].input_args))
		{
			printf("ERROR: read_line\n");
			error_handling(h_input, p_thread_handles, p_thread_ids, p_thread_input, 0);
		}
		if (!create_start_time_object(p_thread_input[num_thread_idx].input_args.time))
		{
			printf("ERROR: create_start_time_object\n");
			error_handling(h_input, p_thread_handles, p_thread_ids, p_thread_input, 0);
		}
	}
	for (int num_thread_idx = 0; num_thread_idx < num_of_lines; num_thread_idx++)
	{
		// Create Thread
		if (p_thread_handles != NULL && p_thread_ids != NULL)
		{
			p_thread_handles[num_thread_idx] = create_thread_simple((LPTHREAD_START_ROUTINE)thread_main, &p_thread_ids[num_thread_idx], &p_thread_input[num_thread_idx]);
			if (NULL == p_thread_handles[num_thread_idx])
			{ //If create thread fails, terminate all running threads and exit
				printf("ERROR: Couldn't create thread %d\n", num_thread_idx);
				threads_termination(p_thread_handles, num_of_lines); // terminate threads
				error_handling(h_input, p_thread_handles, p_thread_ids, p_thread_input, num_thread_idx);
			}
		}
	}
	if (!wait_for_multiple_objects(num_of_lines, p_thread_handles))
	{ //if ERROR detected - need to close mutex&semaphore, terminate all threads, close handles and free memory
		threads_termination(p_thread_handles, num_of_lines); // terminate threads
		error_handling(h_input, p_thread_handles, p_thread_ids, p_thread_input, num_of_lines);
	}
	//Get the exit code per thread
	if (FALSE == get_exit_code_per_thread(num_of_lines, p_thread_handles))
	{
		printf("ERROR: get_exit_code_per_thread haven't finished properly!\n");
		error_handling(h_input, p_thread_handles, p_thread_ids, p_thread_input, num_of_lines);
	}
	int end_time = find_end_time(); //find all-evict time
	clean_frame_table(end_time); // evict all frames
	close_all_handles(h_input, p_thread_handles, num_of_lines); // close all handles
	free_all_memory(p_thread_handles, p_thread_ids, p_thread_input); // free all memory
}

/* main function
* parameters: command line arguments
* returns 0
*/
int main(int argc, char* argv[])
{
	printf("Program is starting now!\n");
	main_function(argv);
	printf("Program finished!\n");
	return 0;
}