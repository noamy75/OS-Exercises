/*
Description: Declarations of functions that are used in FinishProgramHandling.c, imports of relevant variables from HardCodedData.h
*/
/*Global parameters*/

extern int g_num_of_schools, g_memory_allocation_bitmask;
extern HANDLE g_school_idx_mutex_handle; //mutex handle

/*Functions */

void close_struct_idx_handles(handles_t* h_struct);
void threads_termination(HANDLE* p_thread_handles, int end_idx);
void close_struct_handles(handles_t* p_struct_arr);
void close_mutex_handle();
void close_handles_and_free_memory(handles_t* p_struct_arr, HANDLE* p_thread_handles, int num_active_handles);
void error_handling(handles_t* p_struct_arr, HANDLE* p_thread_handles, int num_active_handles);
void close_thread_handles(HANDLE* p_thread_handles, int num_active_handles);
