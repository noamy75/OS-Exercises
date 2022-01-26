
#include <HardCodedData.h>
#include "main.h"

extern int g_num_of_connected_players;
extern char* gp_client_input;
extern BOOL g_no_errors;
extern int g_match_count;
extern int g_num_of_active_players;
extern int g_client_input_counter;
extern BOOL g_player_turn[MAX_NUM_CONNECTED_PLAYERS];
extern BOOL g_player_wants_to_play[MAX_NUM_CONNECTED_PLAYERS];
extern char g_players_name[MAX_NUM_CONNECTED_PLAYERS][MAX_USERNAME_LENGTH];
extern message_type_e g_state_per_client[MAX_NUM_CONNECTED_PLAYERS];
extern HANDLE g_client_input_mutex;
extern HANDLE g_thread_logs[MAX_NUM_CONNECTED_PLAYERS];
extern BOOL g_valid_move;
extern BOOL g_player_made_move;

BOOL execute_operation(thread_input_t* thread_input, char* par);
BOOL waiting_for_data_from_client(thread_input_t* thread_input, int* timeout);
BOOL construct_message(thread_input_t* thread_input, char send_str[]);
BOOL wait_for_receive_data(char** str, thread_input_t* thread_input, int timeout);
BOOL parse_command(char* str, thread_input_t* thread_input, char** par);