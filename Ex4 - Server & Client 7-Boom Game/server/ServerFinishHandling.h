
#include <HardCodedData.h>
#include "main.h"

extern BOOL g_no_errors;
extern BOOL g_server_active_flag;
extern HANDLE g_client_input_mutex;
extern HANDLE g_thread_handles[THREAD_MAX];
extern char* gp_client_input;
extern HANDLE g_thread_logs[MAX_NUM_CONNECTED_PLAYERS];
extern SOCKET g_main_socket;
extern SOCKET g_accept_socket[MAX_NUM_CONNECTED_PLAYERS];

int wait_for_exit();
void close_server_handles();
void finish_server_program_routine();
void finish_program_service_thread(char** send_str, thread_input_t* thread_input, char** par, char** accepted_str);
BOOL close_socket_server(SOCKET* sock);
BOOL grace_disconnect_from_client(SOCKET* socket_input);