/*
Description: Declarations of functions that are used in main.c, imports of relevant variables from HardCodedData.h
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


HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, thread_input_t* thread_input);
void close_handle(HANDLE handle);
void close_thread_handles(HANDLE* p_thread_handles, int num_of_threads);
void close_all_handles(HANDLE h_input, HANDLE* p_thread_handles, int num_of_threads);
void error_handling(HANDLE h_input, HANDLE* p_thread_handles, DWORD* p_thread_ids, thread_input_t* thread_input, int num_of_threads);
void print_multiple_object(DWORD wait_code);
BOOL wait_for_multiple_objects(int threads_amount, HANDLE* p_thread_handles);
void threads_termination(HANDLE* p_thread_handles, int threads_amount);
int find_end_time();
void clean_frame_table(int end_time);
void main_function(char* argv[]);
int main(int argc, char* argv[]);