#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H

/*
Description: This module includes declarations for SendRecvTools.c
*/

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define BRUTAL_TERMINATION_CODE 0x55
#define TIMEOUT 0x5000

typedef enum 
{ 
	TRNS_FAILED,
	TRNS_DISCONNECTED,
	TRNS_SUCCEEDED ,
	TRNS_TIMEOUT,
	TRNS_UNEXPECTED_DISCONNECT
} TransferResult_t;

typedef enum
{
	CLIENT_REQUEST = 0,
	CLIENT_VERSUS,
	CLIENT_PLAYER_MOVE,
	CLIENT_DISCONNECT,
	SERVER_APPROVED,
	SERVER_DENIED,
	SERVER_MAIN_MENU,
	GAME_STARTED,
	TURN_SWITCH,
	SERVER_MOVE_REQUEST,
	GAME_ENDED,
	SERVER_NO_OPPONENTS,
	GAME_VIEW,
	SERVER_OPPONENT_QUIT,
	SERVER_HOLD,
	CLIENT_HOLD
}message_type_e;

TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd);

TransferResult_t SendString(const char* Str, SOCKET sd);

TransferResult_t ReceiveBuffer(char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd, int timeout);

TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd, int timeout);

HANDLE file_opening(char* filename, DWORD desired_access, DWORD creation_disposition);

char* make_char_alloc(int size, BOOL* no_errors);
void close_one_handle(HANDLE* handle, BOOL* no_errors);
void print_multiple_object(DWORD wait_code);
HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine, LPDWORD p_thread_id);
void free_str(char** str);
void wsa_cleanup(BOOL* no_errors);
BOOL wait_for_multiple_objects(HANDLE* thread_handles, int num_threads, BOOL* no_errors, int wait_time);
BOOL get_exit_code_per_thread(HANDLE* thread_handles, int num_threads, BOOL* no_errors);
#endif // SOCKET_SEND_RECV_TOOLS_H