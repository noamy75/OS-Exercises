/*
Description: Declarations of functions that are used in Timing.c, imports of relevant variables from HardCodedData.h
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
void free_times_list();
void sort_time_list(time_object_t* finish_time_object);
void create_time_object(int finish_time);
void update_object(int old_finish_time, int new_finish_time);
void free_one_object(int time);
BOOL create_start_time_object(int time);