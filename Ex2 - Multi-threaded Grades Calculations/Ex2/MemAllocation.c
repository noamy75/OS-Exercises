/*
Description: This module includes functions that handle dynamic memory allocations and the freeing of memory.
*/

/*Includes*/

#include <HardCodedData.h>
#include <MemAllocation.h>

/*Functions */

/*This function creates an array of handles_t structs. Array size is the number of schools. Each struct contains
handles to the files of a specific school and a bitmask of active handles.
Return: array of handles_t structs.
*/
handles_t* make_handle_struct_alloc()
{
	handles_t* p_handles_struct = NULL;
	if (g_num_of_schools == 0) return NULL; // If there are no schools -> return NULL
	p_handles_struct = (handles_t*)malloc((g_num_of_schools) * sizeof(handles_t)); // Memory allocation
	if (p_handles_struct == NULL) // allocation failed
	{
		printf("ERROR: Memory allocation failed, exitting...\n");
		return NULL;
	}
	for (int idx = 0; idx < g_num_of_schools; idx++) // Initialize fields in each struct
	{
		p_handles_struct[idx].active_handles_bitmask = 0;
		p_handles_struct[idx].h_real = NULL;
		p_handles_struct[idx].h_eval = NULL;
		p_handles_struct[idx].h_human = NULL;
		p_handles_struct[idx].h_results = NULL;
		p_handles_struct[idx].h_eng = NULL;
	}

	// indicate that a memory allocation occurred for the array
	g_memory_allocation_bitmask |= BIT((memory_allocation_enum)STRUCT_ARR);
	return p_handles_struct;
}
/*This function frees all allocated memory
Parameters: array of handles_t structs*/
void free_all_memory(handles_t* p_struct_arr)
{
	if (g_memory_allocation_bitmask & BIT((memory_allocation_enum)STRUCT_ARR)) // Array of structs exists
	{
		free(p_struct_arr); // free memory
		g_memory_allocation_bitmask &= ~(BIT((memory_allocation_enum)STRUCT_ARR)); // turn off bit
	}
	printf("All dynamic allocated memory is free!!\n");
}
