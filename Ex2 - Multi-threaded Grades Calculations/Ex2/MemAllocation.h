/*
Description: Declarations of functions that are used in Memallocation.c, imports of relevant variables from HardCodedData.h
*/
/*Global parameters*/

extern int g_num_of_schools, g_memory_allocation_bitmask;

/*Functions */

handles_t * make_handle_struct_alloc();
void free_all_memory(handles_t* p_struct_arr);
