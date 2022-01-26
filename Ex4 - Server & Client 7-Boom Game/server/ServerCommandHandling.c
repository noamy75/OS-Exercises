
#include "ServerCommandHandling.h"
#include "ServerFinishHandling.h"

BOOL g_sync_flag = FALSE;

/* This function  implements the execution of the operation according to the client state
* Parameters: thread input and a parameter from the client program
*/
BOOL execute_operation(thread_input_t* thread_input, char* par)
{
	switch (g_state_per_client[thread_input->num_player])
	{
	case CLIENT_REQUEST:
		if (g_num_of_connected_players < 2)
		{ //if the client state is CLIENT_REQUEST and there are less than 2 players - server send an approvement to the client.
			memcpy(g_players_name[thread_input->num_player], par, strlen(par) + 1); // save the player name on a global variable
			g_state_per_client[thread_input->num_player] = SERVER_APPROVED; // change the client state to SERVER_APPROVED

			if (g_num_of_connected_players == 0)
			{// the first player to connect will play the first turn
				g_player_turn[thread_input->num_player] = TRUE;
			}
		}
		else
		{ // if there are already 2 players connected to the server - it will response SERVER_DENIED
			g_state_per_client[thread_input->num_player] = SERVER_DENIED;
		}
		g_num_of_connected_players++; // increment the number of connected players
		break;

	case CLIENT_VERSUS:
		g_player_wants_to_play[thread_input->num_player] = TRUE; //server gets a response that player wants to play
		g_num_of_active_players++; // increment the number of active players
		int i = 0;
		while (g_num_of_active_players == 1 && i < 15000)
		{
			Sleep(100);
			i += 100;
		}
		if (g_num_of_active_players == 1)
		{ // there is no other player to play with.
			g_state_per_client[thread_input->num_player] = SERVER_NO_OPPONENTS;
		}
		else
		{ // the second player wants to play - the game starts
			g_state_per_client[thread_input->num_player] = GAME_STARTED;
		}
		break;

	case CLIENT_PLAYER_MOVE: // gets a player move
		g_match_count++; // increment the match count
		gp_client_input = make_char_alloc(strlen(par) + 1, &g_no_errors);
		if (gp_client_input == NULL)
		{
			printf("ERROR: dynamic memory allocation failed\n");
			write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: dynamic memory allocation failed\n", FALSE, FALSE);
			return FALSE;
		}
		memcpy(gp_client_input, par, strlen(par) + 1); // get the client input
		g_valid_move = check_player_move_validity(par); // check if the move was valid
		if (!g_valid_move)
		{ // if the player made a invalid move - he lost
			thread_input->is_lost = TRUE;
		}
		if (g_player_turn[thread_input->num_player])
		{ // the players turn is switching
			g_player_turn[thread_input->num_player] = FALSE;
			g_player_turn[thread_input->num_other_player] = TRUE;
		}
		g_state_per_client[thread_input->num_player] = GAME_VIEW; // client state changes to GAME_VIEW
		g_player_made_move = TRUE; // the player made a move
		g_sync_flag = TRUE;
		break;

	case CLIENT_DISCONNECT:
		g_player_wants_to_play[thread_input->num_player] = FALSE; // client disconnect from server
		break;

	}
	return TRUE;
}

/* The function checks if the server is waiting for data from client according to the client state
* Parameters: thread input, pointer to timeout
* Returns TRUE if getting data from server is expected, else FALSE
*/
BOOL waiting_for_data_from_client(thread_input_t* thread_input, int* timeout)
{
	BOOL returned_value = TRUE;
	switch (g_state_per_client[thread_input->num_player])
	{

	case TURN_SWITCH:
		while (g_sync_flag);
		// the player that it is time to play gets SERVER_MOVE_REQUEST, the other player waits for a move to be done by the other player and changes its' state to GAME_VIEW
		if (g_player_turn[thread_input->num_player])
		{
			g_state_per_client[thread_input->num_player] = SERVER_MOVE_REQUEST;
		}
		else
		{
			// wait for a move to be done. Also, finish loop if other player quit.
			while (!g_player_made_move && g_state_per_client[thread_input->num_player] != SERVER_OPPONENT_QUIT);
			if (g_state_per_client[thread_input->num_player] == SERVER_OPPONENT_QUIT)
			{
				returned_value = FALSE;
				g_sync_flag = FALSE;
				break;
			}
			g_state_per_client[thread_input->num_player] = GAME_VIEW;
			g_player_made_move = FALSE; // turn off the flag
			g_sync_flag = FALSE;
		}
		returned_value = FALSE;
		break;

	case GAME_VIEW:
		if (!g_valid_move)
		{ // if the last move was invalid so the game ended
			g_state_per_client[thread_input->num_player] = GAME_ENDED;
		}
		else
		{// if the last move was valid - the game continues
			g_state_per_client[thread_input->num_player] = TURN_SWITCH;
		}
		returned_value = FALSE;
		break;

	case GAME_STARTED:
		for (int i = 0; i < MAX_NUM_CONNECTED_PLAYERS; i++)
		{ // game started - get the idx of the other player
			if ((TRUE == g_player_wants_to_play[i]) && (i != thread_input->num_player))
			{
				thread_input->num_other_player = i;
				break;
			}
		}
		if (!g_player_turn[THREAD_PLAYER_1 - THREAD_PLAYER_OFFSET]) g_player_turn[THREAD_PLAYER_2 - THREAD_PLAYER_OFFSET] = TRUE; // if the first player to connect disconnected before the game had started let the second player to start the game.
		g_match_count = 0; // the match count is 0
		g_state_per_client[thread_input->num_player] = TURN_SWITCH;
		returned_value = FALSE;
		*timeout = TEN_MINUTES; // waiting for input from user - the timeout is 10 minutes
		g_sync_flag = FALSE;
		break;

	case GAME_ENDED: //GAME_ENDED
	case SERVER_OPPONENT_QUIT:
	case SERVER_NO_OPPONENTS:
		memset(g_player_wants_to_play, 0, MAX_NUM_CONNECTED_PLAYERS * sizeof(BOOL)); //players that wants to play another match will have to send CLIENT_VERSUS again
		g_num_of_active_players = 0; // num of active players is 0
	case SERVER_APPROVED:
		g_state_per_client[thread_input->num_player] = SERVER_MAIN_MENU; // client will have the main menu
		returned_value = FALSE;
		*timeout = TEN_MINUTES; // waiting for input from user - the timeout is 10 minutes
		break;
	default: // on all other cases we expect to get a message from client
		returned_value = TRUE;
	}
	return returned_value;
}

/* The function constructs the message that will be sent to the client
* Parameters: thread input, string that will be sent to client
* Returns TRUE if the function finished properly, else FALSE
*/
BOOL construct_message(thread_input_t* thread_input, char send_str[])
{
	switch (g_state_per_client[thread_input->num_player])
	{
		// copy the state to the string
	case SERVER_APPROVED:
		strcpy(send_str, "SERVER_APPROVED");
		break;
	case SERVER_DENIED:
		strcpy(send_str, "SERVER_DENIED");
		break;
	case SERVER_MAIN_MENU:
		strcpy(send_str, "SERVER_MAIN_MENU");
		break;
	case GAME_STARTED:
		strcpy(send_str, "GAME_STARTED");
		break;
	case TURN_SWITCH:
		strcpy(send_str, "TURN_SWITCH:");
		if (g_player_turn[thread_input->num_player])
		{ // check which name suppose to be sent to client
			strcat(send_str, g_players_name[thread_input->num_player]);
		}
		else
		{
			strcat(send_str, g_players_name[thread_input->num_other_player]);
		}
		break;
	case SERVER_MOVE_REQUEST:
		strcpy(send_str, "SERVER_MOVE_REQUEST");
		break;
	case GAME_ENDED:
		strcpy(send_str, "GAME_ENDED:");
		if (thread_input->is_lost) // if the game is ended the client will receive the winner's name
		{
			strcat(send_str, g_players_name[thread_input->num_other_player]);
		}
		else
		{
			strcat(send_str, g_players_name[thread_input->num_player]);
		}
		break;
	case SERVER_NO_OPPONENTS:
		strcpy(send_str, "SERVER_NO_OPPONENTS");
		break;
	case GAME_VIEW:
		strcpy(send_str, "GAME_VIEW:");
		if (g_player_turn[thread_input->num_player]) // the client will receive the name of the player that just made a move
		{
			strcat(send_str, g_players_name[thread_input->num_other_player]);
		}
		else
		{
			strcat(send_str, g_players_name[thread_input->num_player]);
		}
		strcat(send_str, ";");
		strcat(send_str, gp_client_input); // the client receive the last move string
		strcat(send_str, ";");
		if (g_valid_move) // if the last move was valid the client will receive CONTINUE, else END
		{
			strcat(send_str, "CONTINUE");
		}
		else
		{
			strcat(send_str, "END");
		}
		if (!lock_mutex(&g_client_input_mutex))
		{
			write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: failed to lock mutex\n", FALSE, FALSE);
			return FALSE; // lock mutex
		}
		g_client_input_counter++;
		if (g_client_input_counter == 2)
		{ // the dynamic memory allocated for the client input need to be used for both clients and need to be free only after both players saved the input.
			free_str(&gp_client_input);
			g_client_input_counter = 0;
		}
		if (!release_mutex(&g_client_input_mutex))
		{
			write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: failed to release mutex\n", FALSE, FALSE);
			return FALSE; // release mutex
		}
		break;
	case SERVER_OPPONENT_QUIT:
		strcpy(send_str, "SERVER_OPPONENT_QUIT");
		break;
	}
	strcat(send_str, "\n");
	return TRUE;
}

/* This function calls the receive function and handles the receiving result
* Parameters: pointer to the string that will be filled by the data from client, thread input, receive timeout
* Return TRUE if function succeed, else FALSE
*/
BOOL wait_for_receive_data(char** str, thread_input_t* thread_input, int timeout)
{
	TransferResult_t recv_res;
	recv_res = ReceiveString(str, g_accept_socket[thread_input->num_player], timeout); // call receive string operation

	if (recv_res == TRNS_FAILED)
	{ // if the transfer failed
		printf("Service socket error while reading, closing thread.\n");
		g_no_errors = FALSE;
		write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: transfer failed\n", FALSE, FALSE);
		return FALSE;
	}
	else if (recv_res == TRNS_DISCONNECTED)
	{ // if the client is disconnected
		if (!grace_disconnect_from_client(&g_accept_socket[thread_input->num_player]))
		{
			write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: grace disconnect from client failed due to error in shutdown()\n", FALSE, FALSE);
			return FALSE;
		}
		if (thread_input->client_is_active) g_num_of_active_players = 0;
		thread_input->client_is_active = FALSE;
	}
	else if (recv_res == TRNS_TIMEOUT)
	{ // if nothing received until timeout
		if (g_num_of_connected_players == 2) g_state_per_client[thread_input->num_other_player] = SERVER_OPPONENT_QUIT;
		if (thread_input->client_is_active) g_num_of_active_players = 0;
		thread_input->client_is_active = FALSE;
	}
	else if (recv_res == TRNS_UNEXPECTED_DISCONNECT)
	{ // if an unexpected disconnection has occurred
		g_state_per_client[thread_input->num_other_player] = SERVER_OPPONENT_QUIT;
		printf("thread %d - wait_for_receive_data() recv_res = TRNS_UNEXPECTED_DISCONNECT\n", thread_input->num_player);
		if (thread_input->client_is_active) g_num_of_active_players = 0;
		thread_input->client_is_active = FALSE;
		return FALSE;
	}
	return TRUE;
}

/* This function parses the string that received from client
* Parameters: received string, thread_input, pointer to par
* Returns TRUE if function succeed, else FALSE
*/
BOOL parse_command(char* str, thread_input_t* thread_input, char** par)
{
	char* delimiter = strchr(str, ':'); // look for the occurance of ':'
	BOOL colon_flag = TRUE;
	if (NULL == delimiter)
	{ // if there is no ':' - the string contains type only
		colon_flag = FALSE;
		delimiter = strchr(str, '\n'); // look for the end of the string
	}

	char msg_type_buffer[MAX_MSG_TYPE_LENGTH] = { 0 };
	int size = delimiter - str;
	memcpy(msg_type_buffer, str, size); // copy the message type

	// change the client state according to the message type
	if (STRINGS_ARE_EQUAL(msg_type_buffer, "CLIENT_REQUEST")) {
		g_state_per_client[thread_input->num_player] = CLIENT_REQUEST;
	}

	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "CLIENT_VERSUS")) {
		g_state_per_client[thread_input->num_player] = CLIENT_VERSUS;
	}

	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "CLIENT_PLAYER_MOVE")) {
		g_state_per_client[thread_input->num_player] = CLIENT_PLAYER_MOVE;
	}

	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "CLIENT_DISCONNECT")) {
		g_state_per_client[thread_input->num_player] = CLIENT_DISCONNECT;
	}
	delimiter++;
	if (colon_flag) // if the string contains parameter
	{
		char* end_ptr = strchr(delimiter, '\n');
		size = end_ptr - delimiter;
		*par = make_char_alloc(size + 1, &g_no_errors); // make dynamic memory allocation for this parameter
		if (*par == NULL)
		{ // if the memory allocation failed
			g_no_errors = FALSE;
			write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: dynamic memory allocation failed\n", FALSE, FALSE);
			return FALSE;
		}
		memcpy(*par, delimiter, size); // copy the parameter to par
		(*par)[size] = '\0';
	}
	return TRUE;
}