#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "HardCodedData.h"

// Defines

#define FINAL_TRANSMISSION_LENGTH 18
#define CONNECTED_TO_SERVER_LEN 26
#define FAILED_CONNECTING_TO_SERVER_LEN 34
#define IP_LEN 10
#define PORT_LEN 6

// Declarations

typedef enum
{
	SEND_THREAD = 0,
	RECV_THREAD,
	NUM_THREADS
}thread_client_e;

typedef enum
{
	CONNECT_SUCCESS = 0,
	RECONNECT,
	EXIT,
	EXIT_BRUTALLY
}connection_status_e;

BOOL close_socket();
connection_status_e choose_reconnect_or_exit();
connection_status_e connect_to_server();
TransferResult_t trans_zero_or_error(int bytes);
BOOL shutdown_client();
void free_all_memory_recv(char** accepted_str, char** par1, char** par2, char** par3);
BOOL reconnect_or_exit_handling();
static DWORD recv_data_thread(void);
void construct_message_client(char* msg);
static DWORD send_data_thread(void);
void close_handles(HANDLE* thread_handles);
void finish_program_routine(HANDLE* thread_handles);
void main_client(char* argv[]);
int main(int argc, char* argv[]);

#endif // CLIENT_SOCKET_H