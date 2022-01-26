/*
Description: This module includes the relevant functions for the times linked list, such as sort of the list, add new object and free object
*/

/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>
#include <main.h>
#include <FileHandling.h>
#include <MemAllocation.h>
#include <Timing.h>
#include <Init.h>

/*Functions*/

/* The function creates a mutex handle
* Returns TRUE if creation succeeds, else returns FALSE
*/
BOOL create_mutex()
{
	/* Create the mutex that will be used to synchronize access to the page table */
	g_page_table_mutex = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == g_page_table_mutex)
	{ //mutex creation failed
		printf("ERROR: Error when creating mutex: %d\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

/* The function creates a semaphore handle
* Returns TRUE if creation succeeds, else returns FALSE
*/
BOOL create_semaphore()
{
	/* Create the semaphore that will be used to synchronize access to count */
	g_active_frames_semaphore = CreateSemaphore(
		NULL,				/* Default security attributes */
		g_num_frames,		/* Initial Count - all slots are empty */
		g_num_frames,		/* Maximum Count */
		NULL);				/* un-named */
	if (NULL == g_active_frames_semaphore)
	{ // semaphore creation failed
		printf("ERROR: create_semaphore: %d\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

/*This function calculates the number of pages and the number of frames
Parameter: The command line arguments array
*/
void get_num_pages_and_frames(char* argv[])
{
	int virtual_memory = atoi(argv[1]);
	int physical_memory = atoi(argv[2]);
	g_num_pages = (int)pow(2, (double)virtual_memory - NUM_BITS_PER_PAGE);
	g_num_frames = (int)pow(2, (double)physical_memory - NUM_BITS_PER_PAGE);

	printf("g_num_pages = %d , g_num_frames = %d\n", g_num_pages, g_num_frames);
}


/*This function initializes objects that are necessary for the program such as: input/output file handles, page/frame tables
thread-related arrays, mutex/semaphore handles, number of lines variable
Paramters: pointers to the objects to be initialized
Returns TRUE if all initializations succeed, else returns FALSE
*/
BOOL init_vars(char* argv[], HANDLE* h_input, int* num_of_lines, HANDLE** p_thread_handles, DWORD** p_thread_ids, thread_input_t** p_thread_input)
{
	*h_input = file_opening(argv[3], GENERIC_READ, OPEN_EXISTING); // open input file in read mode
	if (*h_input == NULL)
	{
		printf("ERROR:init_vars h_input\n ");
		return FALSE;
	}

	char output_file_name[] = "Output.txt";
	char output_path[MAX_PATH] = { 0 }; // The buffer in which the output path will be built
	strcpy(output_path, argv[3]); // output path equals input path, just for now

	char* starting_point = strrchr(output_path, '\\'); // get a pointer to the last occurance of '\'
	if (NULL == starting_point) { // no '\' in path
		starting_point = output_path;
	}
	else {
		starting_point++;
	}
	char* starting_point_copy = starting_point;
	char* end_of_string = strchr(output_path, 0); // pointer to the first '\0'
	while (starting_point_copy != end_of_string) { // fill zeros instead of input file name
		*starting_point_copy = 0;
		starting_point_copy++;
	}
	memcpy(starting_point, output_file_name, strlen(output_file_name)); // write 'Output.txt' at end of path
	g_h_output = file_opening(output_path, GENERIC_WRITE, OPEN_ALWAYS); // open output file in write mode
	if (g_h_output == NULL)
	{
		printf("ERROR:init_vars h_output\n ");
		return FALSE;
	}
	if (!get_num_of_lines(h_input, num_of_lines)) // get number of lines of input file
	{
		printf("ERROR:init_vars get_num_of_lines\n ");
		return FALSE;
	}
	if (!make_frame_table_alloc()) // Create frame table
	{
		printf("ERROR:init_vars make_frame_table_alloc\n ");
		return FALSE;
	}
	if (!make_page_table_alloc()) // Create page table
	{
		printf("ERROR:init_vars make_page_table_alloc\n ");
		return FALSE;
	}
	*p_thread_handles = make_handle_alloc(*num_of_lines); // Create array of thread handles
	if (NULL == *p_thread_handles)
	{
		printf("ERROR:init_vars make_handle_alloc\n ");
		return FALSE;
	}
	*p_thread_ids = make_dword_alloc(*num_of_lines); // Create array of thread Ids
	if (*p_thread_ids == NULL)
	{
		printf("ERROR:init_vars make_dword_alloc\n ");
		return FALSE;
	}
	*p_thread_input = make_thread_input_alloc(*num_of_lines); // Create array of thread inputs
	if (*p_thread_input == NULL)
	{
		printf("ERROR:init_vars make_thread_input_alloc\n ");
		return FALSE;
	}
	if (!create_mutex()) // create mutex for the page table
	{
		printf("ERROR:init_vars create_mutex g_page_table_mutex\n ");
		return FALSE;
	}
	if (!create_semaphore()) // create semaphore for the amount of active frames
	{
		printf("ERROR:init_vars create_semaphore g_active_frames_semaphore\n ");
		return FALSE;
	}
	return TRUE;
}
