/*
Description: Declarations of functions that are used in main.c, imports of relevant variables from HardCodedData.h
*/
/*Global parameters*/

extern int g_weight_real, g_weight_human, g_weight_eng, g_weight_eval, g_num_of_schools;
extern int g_memory_allocation_bitmask, g_thread_idx; // memory allocation bitmask
extern HANDLE g_school_idx_mutex_handle; //mutex handle once

/*Functions */

void populate_command_args_and_globals(char* argv[]);
BOOL create_mutex();
HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, handles_t* p_struct_arr);
void print_multiple_object(DWORD wait_code);
BOOL create_results_directory();
int calculate_num_threads_per_session(int finished_threads_counter);
void create_threads(handles_t* p_struct_arr, HANDLE* p_thread_handles,
	DWORD* p_thread_ids, int num_threads_per_session, int finished_threads_counter);
BOOL wait_for_multiple_objects(int finished_threads_counter, int num_threads_per_session, HANDLE* p_thread_handles);
BOOL get_exit_code_per_thread(int num_threads_per_session, int finished_threads_counter, HANDLE* p_thread_handles);
void main_function(char* argv[]);
int main(int argc, char* argv[]);