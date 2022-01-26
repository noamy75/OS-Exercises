/*
Description: Declarations of functions that are used in ThreadMain.c, imports of relevant variables from HardCodedData.h
*/
/*Global parameters*/
extern int g_weight_real, g_weight_human, g_weight_eng, g_weight_eval, g_num_of_schools;
extern int g_memory_allocation_bitmask; // memory allocation bitmask
extern HANDLE g_school_idx_mutex_handle; //mutex handle

/*Functions */

HANDLE file_opening(handles_t* p_struct_arr, int num_school_idx,
	char* filename, DWORD desired_access, DWORD creation_disposition, handles_enum active_handle);
void create_file_name(char* num_school_idx_char, char* directory, char* filename);
void open_handles(int num_school_idx, handles_t* p_struct_arr);
BOOL read_from_file(HANDLE h_file, int* p_read_grade, int num_school_idx, handles_enum handle);
void write_to_file(HANDLE h_results, int average_grade, int num_school_idx);
void avarage_calculation(int num_school_idx, handles_t* p_struct_arr);
int thread_main(LPVOID lpParam);
void lock_mutex();
void release_mutex(int num_school_idx);
void activate_handles(handles_t* p_struct_arr, int num_school_idx, handles_enum active_handle);