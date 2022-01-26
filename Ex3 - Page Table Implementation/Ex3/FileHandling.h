/*
Description: Declarations of functions that are used in FileHandling.c, imports of relevant variables from HardCodedData.h
*/

/*Global parameters*/

extern HANDLE g_h_output;

/*Functions */

HANDLE file_opening(char* filename, DWORD desired_access, DWORD creation_disposition);
BOOL read_from_file(HANDLE h_file, int* p_arg, char last_byte);
BOOL read_line(HANDLE h_input, memory_call_t* p_input_args);
BOOL set_file_offset(HANDLE h_file);
BOOL get_num_of_lines(HANDLE* h_input, int* p_counter);
void write_to_file(HANDLE h_output, int time, int page, int frame, char* tav);