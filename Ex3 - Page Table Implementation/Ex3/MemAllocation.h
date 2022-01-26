/*
Description: Declarations of functions that are used in MemAllocation.c, imports of relevant variables from HardCodedData.h
*/

/*Global parameters*/

extern int g_num_pages;
extern int g_num_frames;
extern page_table_t g_page_table;
extern frame_object_t* pg_frame_table;
extern time_object_t* pg_time_list;
extern int g_num_elements_in_list;

/*Functions*/

HANDLE* make_handle_alloc(int num_of_lines);
DWORD* make_dword_alloc(int num_of_lines);
thread_input_t* make_thread_input_alloc(int num_of_lines);
BOOL make_page_table_alloc();
BOOL make_frame_table_alloc();
void free_all_memory(HANDLE* p_thread_handles, DWORD* p_thread_ids, thread_input_t* p_thread_input);