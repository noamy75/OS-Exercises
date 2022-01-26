/*
Description: This module includes the thread's main function and other functions relevant to its operation such as
file handling, handles opening, mutex locking/releasing and grades calculations.
*/

/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>
#include <main.h>
#include <FileHandling.h>
#include <Timing.h>

/*Functions */

/* The function updates the page object and frame object parameters
* Parameters: page_object, finish_time, frame_number, thread_input
*/
void update_page_object_and_frame_table(page_object_t* page_object, int finish_time, int frame_number, thread_input_t thread_input)
{
	page_object->is_active = TRUE;
	page_object->valid = TRUE;
	pg_frame_table[frame_number].start_time = thread_input.input_args.time;
	pg_frame_table[frame_number].is_active = TRUE;
	pg_frame_table[frame_number].mapped_page = thread_input.input_args.page_number;
	page_object->finish_time = finish_time;
	page_object->frame_number = frame_number;
}

/* The function checks if a update of the page's finish time is needed, if so, it will update the page object and frame object parameters
* and free the start time object from the linked list.
* Parameters: page_object, finish_time, thread_input
*/
void update_finish_time_check(page_object_t* page_object, int finish_time, thread_input_t thread_input)
{
	if (page_object->finish_time <= finish_time) { // need to update page finish time
		update_object(page_object->finish_time, finish_time); // update time in linked list
		update_page_object_and_frame_table(page_object, finish_time, page_object->frame_number, thread_input);
	}
	free_one_object(thread_input.input_args.time);
}

/* The function gets the minimum start time of a non active page
* This method will help us evict the corresponding page from the frame table in LRU eviction method
* The function will also turn on the empty_frame_exists if there is an empty frame.
* Parameters: pointer to empty_frame_exists and a pointer to minimum_start_time
*/
void get_minimum_start_time_and_empty_frame_flag(BOOL* empty_frame_exists, unsigned int* minimum_start_time)
{
	for (int frame_idx = 0; frame_idx < g_num_frames; frame_idx++) { //go over all frames and find the first one that finishes
		if (!pg_frame_table[frame_idx].is_active) { //find the frame with the minimum finish time for LRU eviction policy
			*minimum_start_time = min(*minimum_start_time, (unsigned int)(pg_frame_table[frame_idx].start_time));
		}
		if (pg_frame_table[frame_idx].mapped_page == INVALID_MAPPING) {
			*empty_frame_exists = TRUE; //There is one non-active frame that can be evicted
		}
	}
}

/* The function handles the eviction of page from a frame and set new page in the frame
* The eviction method is LRU
* Parameters: page_object, thread input, is_stuck flag
*/
void page_not_active_handling(page_object_t* page_object, thread_input_t thread_input, BOOL is_stuck)
{
	int finish_time = thread_input.input_args.time + thread_input.input_args.time_of_use; // current expected finish time

	lock_mutex(thread_input.num_thread);
	if (page_object->valid)
	{ //if page is mapped to frame table, update its' finish time
		update_page_object_and_frame_table(page_object, finish_time, page_object->frame_number, thread_input);
	}
	else
	{
		unsigned int minimum_start_time = 0xFFFFFFFF;
		BOOL empty_frame_exists = FALSE;
		get_minimum_start_time_and_empty_frame_flag(&empty_frame_exists, &minimum_start_time);
		for (int frame_idx = 0; frame_idx < g_num_frames; frame_idx++)
		{ //go over all frames, find the first empty frame or an LRU frame to be evicted
			if (empty_frame_exists)
			{ // There exists an empty frame
				if (pg_frame_table[frame_idx].mapped_page == INVALID_MAPPING)
				{ // reached the first empty one
					update_page_object_and_frame_table(page_object, finish_time, frame_idx, thread_input);
				}
				else
				{ // continue loop until finding the first empty frame
					continue;
				}
			}
			else if (pg_frame_table[frame_idx].is_active || minimum_start_time != pg_frame_table[frame_idx].start_time)
			{ //the page is still active or not suppose to be evicted by LRU eviction method
				continue;
			}
			else
			{ //arrived to the frame that suppose to be evicted
				page_object_t* evicted_page_object = &g_page_table.page_array[pg_frame_table[frame_idx].mapped_page];
				evicted_page_object->finish_time = max(evicted_page_object->finish_time, thread_input.input_args.time);
				evicted_page_object->valid = FALSE;
				write_to_file(g_h_output, evicted_page_object->finish_time, pg_frame_table[frame_idx].mapped_page, frame_idx, "E");
				printf("%d %d %d E\n", evicted_page_object->finish_time, pg_frame_table[frame_idx].mapped_page, frame_idx);
				thread_input.input_args.time = evicted_page_object->finish_time;
				update_page_object_and_frame_table(page_object, thread_input.input_args.time + thread_input.input_args.time_of_use, frame_idx, thread_input);
			}
			printf("%d %d %d P\n", thread_input.input_args.time, pg_frame_table[frame_idx].mapped_page, frame_idx);
			write_to_file(g_h_output, thread_input.input_args.time, pg_frame_table[frame_idx].mapped_page, frame_idx, "P");
			break;
		}
	}
	create_time_object(page_object->finish_time);
	if (!is_stuck) {
		free_one_object(thread_input.input_args.time);
	}
	g_arrived_semaphore--;
	release_mutex(thread_input.num_thread);

}

/* The function handles the finish page access routine like deactivate the page
* Paramters: page_object, thread_input
*/
void finish_page_access(page_object_t* page_object, thread_input_t thread_input)
{
	lock_mutex(thread_input.num_thread);
	pg_frame_table[page_object->frame_number].is_active = FALSE;
	page_object->is_active = FALSE;
	g_arrived_semaphore++;
	free_one_object(page_object->finish_time);
	release_mutex(thread_input.num_thread);
}

/* Thread main function
* Parameters: input args
* Returns 0 if Thread finished his process
*/
int thread_main(LPVOID lpParam)
{
	thread_input_t thread_input; // Prepare the array of handles_t structs
	if (NULL == lpParam) { // If the thread parameter is NULL
		printf("ERROR: Thread parameters are NULL\n");
		ExitThread(1);
	}
	memcpy(&thread_input, (thread_input_t*)lpParam, sizeof(thread_input_t));
	page_object_t* page_object = &g_page_table.page_array[thread_input.input_args.page_number]; // get the relevant page
	BOOL is_stuck = FALSE;
	int finish_time = thread_input.input_args.time + thread_input.input_args.time_of_use; // current expected finish time
	wait_for_event(thread_input, finish_time); // Wait until the thread can approach the page table
	lock_mutex(thread_input.num_thread); // approach page table

	if (page_object->is_active)
	{ // page is active - check if finish time update is needed
		update_finish_time_check(page_object, finish_time, thread_input);
		release_mutex(thread_input.num_thread);
	}

	else
	{ //page is not active 
		if (g_arrived_semaphore <= 0)

		{ // if thread is going to wait on semaphore, is_stuck = TRUE and remove the time object from linked list
			free_one_object(thread_input.input_args.time);
			is_stuck = TRUE;
			if (page_object->waiting_for_access)
			{
				if (thread_input.input_args.time_of_use > page_object->future_time_of_use)
				{
					page_object->future_time_of_use = thread_input.input_args.time_of_use;
				}
				release_mutex(thread_input.num_thread);
				return 0;
			}
			else
			{
				page_object->waiting_for_access = TRUE;
				page_object->future_time_of_use = thread_input.input_args.time_of_use;
			}
		}
		release_mutex(thread_input.num_thread);

		down_semaphore(thread_input.num_thread); // lock semaphore

		if (page_object->waiting_for_access)
		{
			page_object->waiting_for_access = FALSE;
			thread_input.input_args.time_of_use = page_object->future_time_of_use;
		}

		page_not_active_handling(page_object, thread_input, is_stuck);

		while (pg_time_list->time != page_object->finish_time); // wait for finish time

		// finish page access routine
		finish_page_access(page_object, thread_input);

		// If there are several elements in the time list with the same finish time, take them all out before letting a page to take a frame.
		// This is necessary for the LRU eviction policy.
		while (g_num_elements_in_list != 0)
		{
			if (pg_time_list->time != page_object->finish_time)
			{
				break;
			}
		}

		// up semaphore
		up_semaphore(thread_input.num_thread);
	}
	// Thread finished
	return 0;
}

/*This function implements the action DOWN of the semaphore: g_active_frames_semaphore
Parameter: The thread index that performs the DOWN action
*/
void down_semaphore(int num_thread_idx)
{
	int wait_res = WaitForSingleObject(g_active_frames_semaphore, TIMEOUT_IN_MILLISECONDS);
	if (wait_res != WAIT_OBJECT_0)
	{
		printf("ERROR down_semaphore for thread %d\n", num_thread_idx);
		printf("THREAD %d CRASHED\n", num_thread_idx);
		ExitThread(1);
	}
}

/*This function implements the action UP of the semaphore: g_active_frames_semaphore
Parameter: The thread index that performs the UP action
*/
void up_semaphore(int num_thread_idx)
{
	LONG previous_count;
	BOOL release_res = ReleaseSemaphore(
		g_active_frames_semaphore,
		1, 		/* Signal that exactly one cell was emptied */
		&previous_count);
	if (FALSE == release_res)
	{ //if the mutex releasing procedure failed
		printf("ERROR: up_semaphore for thread %d\n", num_thread_idx);
		printf("THREAD %d CRASHED\n", num_thread_idx);
		ExitThread(1);
	}
}

/* This function locks the mutex of the page table: g_page_table_mutex
Parameter: The thread index that performs the locking
*/
void lock_mutex(int num_thread_idx)
{
	int wait_code = WaitForSingleObject(g_page_table_mutex, TIMEOUT_IN_MILLISECONDS);
	if (WAIT_OBJECT_0 != wait_code)
	{ //If locking procedure failed
		printf("ERROR lock_mutex for thread %d\n", num_thread_idx);
		printf("THREAD %d CRASHED\n", num_thread_idx);
		ExitThread(1);
	}
}

/* This function releases the mutex of the page table: g_page_table_mutex
Parameter: The thread index that performs the release
*/
void release_mutex(int num_thread_idx)
{
	BOOL ret_val = ReleaseMutex(g_page_table_mutex);
	if (FALSE == ret_val)
	{ //if the mutex releasing procedure failed
		printf("ERROR: Error when releasing mutex\n");
		printf("THREAD %d CRASHED\n", num_thread_idx);
		ExitThread(1);
	}
}

/*This function implements a waiting procedure for a thread until it's time for it to approach the page table
Parameters: thread_input - thread args including memory call parameters, finish_time
*/
void wait_for_event(thread_input_t thread_input, int finish_time)
{
	/* A thread shall approach the page table if one of the following hold:
	 * - His start time meets the finish time of the first node in the list, meaning there's a frame available for it
	 * - The page is mapped and the call time is smaller than the finish time of the page that's in the frame
	*/
	while (thread_input.input_args.time != pg_time_list->time &&    // It's not his start time
		(!(g_page_table.page_array[thread_input.input_args.page_number].valid) || // Page isn't mapped
			thread_input.input_args.time > g_page_table.page_array[thread_input.input_args.page_number].finish_time)); // call time is bigger
}

