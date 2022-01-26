/*
Description: Declarations of functions that are used in Init.c, imports of relevant variables from HardCodedData.h
*/

/*Global parameters*/

extern int g_num_pages;
extern int g_num_frames;
extern page_table_t g_page_table;
extern HANDLE g_page_table_mutex;
extern HANDLE g_active_frames_semaphore;
extern HANDLE g_h_output;
extern frame_object_t* pg_frame_table;
extern time_object_t* pg_time_list;
extern int g_arrived_semaphore;
extern int g_num_elements_in_list;

/*Functions */
BOOL create_mutex();
BOOL create_semaphore();
void get_num_pages_and_frames(char* argv[]);
BOOL init_vars(char* argv[], HANDLE* h_input, int* num_of_lines, HANDLE** p_thread_handles, DWORD** p_thread_ids, thread_input_t** p_thread_input);
