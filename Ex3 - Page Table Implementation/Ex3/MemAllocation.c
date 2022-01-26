/*
Description: This module includes the relevant functions for dynamic memory allocation and the free of it.
*/

/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>
#include <main.h>
#include <FileHandling.h>
#include <MemAllocation.h>
#include <Timing.h>

/*Functions*/

/*This function creates a frame table via dynamic memory allocation
Returns TRUE if creation succeeds, else returns FALSE
*/
BOOL make_frame_table_alloc()
{
	pg_frame_table = NULL;
	pg_frame_table = (frame_object_t*)malloc(g_num_frames * sizeof(frame_object_t)); // memory allocation
	if (pg_frame_table == NULL) // allocation failed
	{
		printf("ERROR make_frame_table_alloc g_frame_table\n");
		return FALSE;
	}
	memset(pg_frame_table, 0, g_num_frames * sizeof(frame_object_t)); // Reset memory
	return TRUE;
}

/*This function creates the page table via dynamic memory allocation
Returns TRUE if creation succeeds, else returns FALSE
*/
BOOL make_page_table_alloc()
{
	g_page_table.page_array = NULL;
	g_page_table.page_array = (page_object_t*)malloc(g_num_pages * sizeof(page_object_t)); // memory allocation
	if (g_page_table.page_array == NULL) // allocation failed
	{
		printf("ERROR make_page_table_alloc page_array\n");
		return FALSE;
	}
	memset(g_page_table.page_array, 0, g_num_pages * sizeof(page_object_t)); // Reset memory
	//printf("g_page_table.page_array[0].valid = %d\n", g_page_table.page_array[0].valid);
	return TRUE;
}

/*This function creates an array of thread handles
Returns a pointer to the handles array
*/
HANDLE* make_handle_alloc(int num_of_lines)
{
	HANDLE* p_handle = NULL;
	p_handle = (HANDLE*)malloc(num_of_lines * sizeof(HANDLE)); // Memory allocation
	if (p_handle == NULL) // allocation failed
	{
		printf("ERROR: make_handle_alloc\n");
		return NULL;
	}
	return p_handle;
}


/*This function creates an array of thread IDs (of type DWORD)
Parameter: num_of_lines - the number of lines which is also the number of threads
Return: A pointer to the thread IDs array
*/
DWORD* make_dword_alloc(int num_of_lines)
{
	DWORD* p_dword = NULL;
	p_dword = (DWORD*)malloc(num_of_lines * sizeof(DWORD)); // Memory allocation
	if (p_dword == NULL) // allocation failed
	{
		printf("ERROR: make_dword_alloc\n");
		return NULL;
	}
	return p_dword;
}


/*This function creates an array of thread IDs (of type DWORD)
Parameter: num_of_lines - the number of lines which is also the number of threads
Return: A pointer to the thread IDs array
*/
thread_input_t* make_thread_input_alloc(int num_of_lines)
{
	thread_input_t* p_thread_input = NULL;
	p_thread_input = (thread_input_t*)malloc(num_of_lines * sizeof(thread_input_t)); // Memory allocation
	if (p_thread_input == NULL) // allocation failed
	{
		printf("ERROR: make_thread_input_alloc\n");
		return NULL;
	}

	return p_thread_input;
}

/*This function frees all the memory that was dynamically allocated in the program
Parameters: p_thread_handles - thread handles array, p_thread_ids - thread IDs array, p_thread_input - thread input args array
*/
void free_all_memory(HANDLE* p_thread_handles, DWORD* p_thread_ids, thread_input_t* p_thread_input)
{
	if (NULL != p_thread_handles)
	{
		free(p_thread_handles);
		printf("free_all_memory: p_thread_handles was freed\n");
	}
	if (NULL != p_thread_ids)
	{
		free(p_thread_ids);
		printf("free_all_memory: p_thread_ids was freed\n");
	}
	if (NULL != p_thread_input)
	{
		free(p_thread_input);
		printf("free_all_memory: thread_input was freed\n");
	}
	if (NULL != g_page_table.page_array)
	{
		free(g_page_table.page_array);
		printf("free_all_memory: g_page_table.page_array was freed\n");
	}
	if (NULL != pg_frame_table)
	{
		free(pg_frame_table);
		printf("free_all_memory: pg_frame_table was freed\n");
	}
	if (0 != g_num_elements_in_list)
	{
		free_times_list();
		printf("free_all_memory: pg_finish_time_list was freed\n");

	}
}

