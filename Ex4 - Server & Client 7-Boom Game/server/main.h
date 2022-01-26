#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "HardCodedData.h"

// Defines

#define SERVER_ADDRESS_STR "127.0.0.1"
#define MAX_NUM_CONNECTED_PLAYERS 3 // the maximum amount of connected players is 3
#define MAX_NUM_ACTIVE_PLAYERS 2 // 2 players are playing against each other

// Declarations

//enum of thread idx
typedef enum
{
	THREAD_WAIT_FOR_EXIT = 0,
	THREAD_LISTENING,
	THREAD_PLAYER_1,
	THREAD_PLAYER_2,
	THREAD_PLAYER_3,
	THREAD_MAX
}thread_idx_e;

#define THREAD_PLAYER_OFFSET THREAD_PLAYER_1

// client db
typedef struct
{
	int num_player; // num client
	int num_other_player; // num of the other client playing against
	BOOL is_lost; // is this client lost flag
	BOOL client_is_active; //client is active boolean
}thread_input_t;

BOOL create_mutex(HANDLE* mutex);
BOOL lock_mutex(HANDLE* mutex);
BOOL release_mutex(HANDLE* mutex);
static int find_first_available_client();
int thread_listening();
BOOL is_boom_expected();
BOOL check_player_move_validity(char* player_move);
BOOL file_opening_server(thread_input_t* thread_input, char* username);
BOOL write_to_log_file(HANDLE thread_log, char* message, BOOL is_sent_to_client, BOOL network_message);
int thread_service(thread_input_t* thread_input);
BOOL bind_socket(SOCKADDR_IN* service, int port);
BOOL create_new_client_thread(thread_input_t* thread_input);
void server_main(char* argv[]);
int main(int argc, char* argv[]);

#endif // SERVER_SOCKET_H