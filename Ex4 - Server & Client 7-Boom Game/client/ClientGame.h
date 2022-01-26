
#include "HardCodedData.h"

/* Global parameters */

extern char g_user_input[MAX_INPUT_LENGTH];
extern message_type_e g_client_state;
extern char gp_username[MAX_USERNAME_LENGTH];
extern BOOL g_no_errors;
extern char* gp_server_address;
extern int g_server_port;

/* Functions */

BOOL execute_operation_client(int* timeout, char* par1, char* par2, char* par3);
void change_client_state(char* msg_type_buffer);
BOOL parse_message_client(char* accepted_str, char** par1, char** par2, char** par3);
void show_main_menu();
BOOL is_zero();
BOOL is_not_valid_move();
void get_player_valid_move();