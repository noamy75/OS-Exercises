/*
Description: Declarations of functions that are used in ThreadMain.c, imports of relevant variables from HardCodedData.h
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

void wait_for_event(thread_input_t thread_input, int time);
void lock_mutex(int num_thread_idx);
void update_page_object_and_frame_table(page_object_t* page_object, int finish_time, int frame_number, thread_input_t thread_input);
void update_finish_time_check(page_object_t* page_object, int finish_time, thread_input_t thread_input);
void get_minimum_start_time_and_empty_frame_flag(BOOL* empty_frame_exists, unsigned int* minimum_start_time);
void page_not_active_handling(page_object_t* page_object, thread_input_t thread_input, BOOL is_stuck);
void finish_page_access(page_object_t* page_object, thread_input_t thread_input);
void release_mutex(int num_thread_idx);
void down_semaphore(int num_thread_idx);
void up_semaphore(int num_thread_idx);
int thread_main(LPVOID lpParam);
