/*
Description: This module includes all the hard coded data used in the project - conatants definitions, variables and
structs declarations.
*/

#define _CRT_SECURE_NO_WARNINGS

/* includes */
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <Windows.h>

/* Definitions*/

#define NUM_BITS_PER_PAGE 12 //4KB page size
#define NUM_BITS_INT 11 //2 ** 32 + '\0'
#define ONE_BYTE 1
#define TIMEOUT_IN_MILLISECONDS 5000
#define INVALID_MAPPING 0xFFFFFFFF
#define MAX_LINE_LENGTH 35 // 3 * 11 + 'P/E' + '\0'
#define BRUTAL_TERMINATION_CODE 0x55
#define NUM_THREADS_MULT_WAIT 60;

/* Structs */

typedef struct
{
	int time;
	int virtual_address;
	int time_of_use;
	int page_number;
}memory_call_t;

typedef struct
{
	int frame_number;
	BOOL valid;
	int finish_time;
	BOOL is_active;
	BOOL waiting_for_access;
	int future_time_of_use;
}page_object_t;

typedef struct
{
	page_object_t* page_array;
}page_table_t;

typedef struct
{
	memory_call_t input_args;
	int num_thread;
}thread_input_t;

typedef struct
{
	int mapped_page;
	BOOL is_active;
	int start_time;
}frame_object_t;

typedef struct time_object
{
	int time;
	struct time_object* next;
}time_object_t;

/*Global parameters*/

int g_num_pages;
int g_num_frames;
page_table_t g_page_table;
HANDLE g_page_table_mutex;
HANDLE g_active_frames_semaphore;
HANDLE g_h_output;
frame_object_t* pg_frame_table;
time_object_t* pg_time_list;
int g_arrived_semaphore;
int g_num_elements_in_list;