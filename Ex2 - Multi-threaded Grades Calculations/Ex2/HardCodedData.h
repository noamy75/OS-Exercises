/*
Description: This module includes all the hard coded data used in the project - conatants definitions, variables and
structs declarations.
*/

#define _CRT_SECURE_NO_WARNINGS

/*Includes*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tchar.h>
#include <fileapi.h>

/*Definitions */

#define MAX_LEN_INT			11	// "2^32" + '\0'
#define MAX_FILE			16 //   "Results/Results" + '\0'
#define MAX_FILE_NAME_LEN	(MAX_LEN_INT + MAX_FILE + 5) //".txt" + '\0'
#define ONE_BYTE			1
#define MAX_GRADE_LEN		6				//"100" + '\r' + '\n' + '\0'
#define BIT(n)				(1<<n)
#define TIMEOUT_IN_MILLISECONDS 5000
#define BRUTAL_TERMINATION_CODE 0x55
#define MAX_THREADS			10				//max number of threads work simultanously
#define RESULTS_LEN			8	// "Results" + '\0'

/*Global parameters*/

int g_weight_real, g_weight_human, g_weight_eng, g_weight_eval, g_num_of_schools; //global parameters from the command line
int g_memory_allocation_bitmask; //global dynamic memory allocation bitmask
HANDLE g_school_idx_mutex_handle; // mutex handle
int g_thread_idx; // thread index

/*Structs and enums */

// A struct containing handles to files of a specific school and a bitmask of active handles.
typedef struct
{
	HANDLE h_real;
	HANDLE h_human;
	HANDLE h_eng;
	HANDLE h_eval;
	HANDLE h_results;
	int active_handles_bitmask;
}handles_t;


//handle identifier enum
typedef enum
{
	REAL = 0,
	HUMAN,
	ENG,
	EVAL,
	RESULTS
}handles_enum;

// Options of dynamic memory allocation
typedef enum
{
	STRUCT_ARR = 0,
	THREAD_IDS,
	THREAD_HANDLES
}memory_allocation_enum;
