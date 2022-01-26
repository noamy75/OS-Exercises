
#include "ClientGame.h"
#include "ClientFile.h"
#include "main.h"



/* This function changes the client state according to the message received from server.
* Parameter: message type string
*/
void change_client_state(char* msg_type_buffer)
{
	// if message type is SERVER_APPROVED
	if (STRINGS_ARE_EQUAL(msg_type_buffer, "SERVER_APPROVED")) {
		g_client_state = SERVER_APPROVED;
	}

	// if message type is SERVER_DENIED
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "SERVER_DENIED")) {
		g_client_state = SERVER_DENIED;
	}

	// if message type is SERVER_MAIN_MENU
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "SERVER_MAIN_MENU")) {
		g_client_state = SERVER_MAIN_MENU;
	}

	// if message type is GAME_STARTED
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "GAME_STARTED")) {
		g_client_state = GAME_STARTED;
	}

	// if message type is TURN_SWITCH
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "TURN_SWITCH")) {
		g_client_state = TURN_SWITCH;
	}

	// if message type is SERVER_MOVE_REQUEST
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "SERVER_MOVE_REQUEST")) {
		g_client_state = SERVER_MOVE_REQUEST;
	}

	// if message type is GAME_ENDED
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "GAME_ENDED")) {
		g_client_state = GAME_ENDED;
	}

	// if message type is SERVER_NO_OPPONENTS
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "SERVER_NO_OPPONENTS")) {
		g_client_state = SERVER_NO_OPPONENTS;
	}

	// if message type is GAME_VIEW
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "GAME_VIEW")) {
		g_client_state = GAME_VIEW;
	}

	// if message type is SERVER_OPPONENT_QUIT
	else if (STRINGS_ARE_EQUAL(msg_type_buffer, "SERVER_OPPONENT_QUIT")) {
		g_client_state = SERVER_OPPONENT_QUIT;
	}
}

/* This function parses the message received from server
* Paramters: pointers to the accepted string, and pointers to 3 parameters
* Returns FALSE if there was a failure in the function, else TRUE
*/
BOOL parse_message_client(char* accepted_str, char** par1, char** par2, char** par3)
{
	char* delimiter = strchr(accepted_str, ':'); //find the first occurance of ':'
	BOOL colon_flag = TRUE; //flag that note if there are parameters
	if (NULL == delimiter)
	{ //The message doesn't contain ':' -> it doesn't contain parameters
		colon_flag = FALSE; // there are no parameters
		delimiter = strchr(accepted_str, '\n'); // get the pointer to the finish of accepted_str
	}

	char msg_type_buffer[MAX_MSG_TYPE_LENGTH] = { 0 };
	int size = delimiter - accepted_str; // get the size of the message type
	memcpy(msg_type_buffer, accepted_str, size); // copy the message type to the message type buffer
	change_client_state(msg_type_buffer); // convert the client state according to the message type

	delimiter++;
	if (colon_flag && (g_client_state == GAME_VIEW))
	{ // if there are parameters and the client state is GAME VIEW -> there are 3 parameters
		char* semicolon_ptr = strchr(accepted_str, ';'); //get the pointer to the first occurance of ';'
		size = semicolon_ptr - delimiter + 1; // get parameter size
		*par1 = make_char_alloc(size, &g_no_errors); // make buffer dynamic allocation
		if (*par1 == NULL)
		{ // dynamic allocation failed
			printf("ERROR: make_char_alloc par1\n");
			write_to_client_log_file("ERROR: memory allocation failed\n", FALSE, FALSE);
			return FALSE;
		}
		memcpy(*par1, delimiter, size - 1); // copy the first parameter to par1 buffer
		(*par1)[size - 1] = '\0';
		delimiter = ++semicolon_ptr;
		char* semicolon_ptr2 = strrchr(accepted_str, ';'); // get the pointer to the last occurance of ';'
		size = semicolon_ptr2 - delimiter + 1; // get parameter size
		*par2 = make_char_alloc(size, &g_no_errors); // make buffer dynamic allocation
		if (*par2 == NULL)
		{ //dynamic allocation failed
			printf("ERROR: make_char_alloc par2\n");
			write_to_client_log_file("ERROR: memory allocation failed\n", FALSE, FALSE);
			return FALSE;
		}
		memcpy(*par2, delimiter, size - 1); // copy the second parameter to par2 buffer
		(*par2)[size - 1] = '\0';
		delimiter = ++semicolon_ptr2;
		char* end_ptr = strchr(delimiter, '\n'); // get the pointer to the finish of received buffer
		size = end_ptr - delimiter + 1; // get third parameter size
		*par3 = make_char_alloc(size, &g_no_errors); // make buffer dynamic allocation
		if (*par3 == NULL)
		{ // dynamic allocation failed
			printf("ERROR: make_char_alloc par3\n");
			write_to_client_log_file("ERROR: memory allocation failed\n", FALSE, FALSE);
			return FALSE;
		}
		memcpy(*par3, delimiter, size - 1); // copy the third parameter to par3 buffer
		(*par3)[size - 1] = '\0';
	}

	else if (colon_flag)
	{ // the received buffer contains parameters and the client state is not GAME_VIEW -> there is only 1 parameter
		char* end_ptr = strchr(accepted_str, '\n'); // get the pointer to the finish of received buffer
		size = end_ptr - delimiter + 1; // get parameter size
		*par1 = make_char_alloc(size, &g_no_errors); // make buffer dynamic allocation
		if (*par1 == NULL)
		{ // dynamic allocation failed
			printf("ERROR: make_char_alloc par1\n");
			write_to_client_log_file("ERROR: memory allocation failed\n", FALSE, FALSE);
			return FALSE;
		}
		memcpy(*par1, delimiter, size - 1); // copy the parameter to par3 buffer
		(*par1)[size - 1] = '\0';
	}
	return TRUE; // parse client message finished properly
}

/* This function prints the main menu to the client, client need to choose 1 if he wants to play against another client or 2 if he wants to disconnect
*/
void show_main_menu()
{
	int decision = 0;
	char decision_str[MAX_INPUT_LENGTH] = { 0 };
	do { //print the main menu
		printf("Choose what to do next:\n"
			"1. Play against another client\n"
			"2. Quit\n");
		// get input from the client
		gets_s(decision_str, MAX_INPUT_LENGTH);
		if (STRINGS_ARE_EQUAL(decision_str, "1")) {
			// if client wants to play against another player
			g_client_state = CLIENT_VERSUS; // client state is changed to CLIENT_VERSUS -> will be sent to the server
			decision = atoi(decision_str); // convert the string to integer
		}
		else if (STRINGS_ARE_EQUAL(decision_str, "2")) {
			// if client wants to disconnect
			g_client_state = CLIENT_DISCONNECT; // client state is changed to CLIENT_DISCONNECT -> will be sent to the server and start disconnection routine
			decision = atoi(decision_str); // convert the string to integer
		}
		else {
			// client inserted invalid input
			printf("Error: Illegal command\n");
			write_to_client_log_file("Error: Illegal command\n", FALSE, FALSE);
		}
	} while (decision != 1 && decision != 2); // client will be able to choose again until he inserts valid input
}


/* This function checks if the user input contains only '0' chars
Returns TRUE if it is zero, else FALSE
*/
BOOL is_zero()
{
	// go over the user input string, return false if one of the chars is not '0'
	for (int i = 0; i < (int)strlen(g_user_input); i++)
	{
		if (g_user_input[i] != '0')
		{ // not '0'
			return FALSE;
		}
	}
	// all chars are '0' - return TRUE
	return TRUE;
}

/* This function checks if the user move is valid (number or "boom")
* Returns TRUE if it is not valid move, else FALSE
*/
BOOL is_not_valid_move()
{
	// atoi() == 0 when the user input is 0 or contains character that is not a digit.
	if (atoi(g_user_input) == 0 && !(STRINGS_ARE_EQUAL("boom", g_user_input)))
	{ // the user input contains charaters which are not digits and the string is not "boom"
		if (strlen(g_user_input) == 0) // if the user input is empty - illegal command
		{
			printf("Error: Illegal command\n");
			write_to_client_log_file("Error: Illegal command\n", FALSE, FALSE);
			return TRUE;
		}
		// the user input might be 0 which is valid move so need to check if "0" was the user input.
		else if (!is_zero()) // check if the user input was "0" - valid move
		{ // the user input wasnt "0"
			printf("Error: Illegal command\n");
			write_to_client_log_file("Error: Illegal command\n", FALSE, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

/* This function gets a user move, it repeat itself until the user inserted a valid move (number or "boom")
*/
void get_player_valid_move()
{
	do
	{
		printf("Enter the next number or boom:\n");
		gets_s(g_user_input, MAX_INPUT_LENGTH); // get input

	} while (is_not_valid_move()); // get another input until the move is valid.
}

/* The function executes the operation according to the client state
* Parameters: receive timeout, 3 message parameters
*/
BOOL execute_operation_client(int* timeout, char* par1, char* par2, char* par3)
{
	switch (g_client_state)
	{
	case SERVER_APPROVED: // SERVER_APPROVED - print connected to server
		printf("Connected to server on %s:%d\n", gp_server_address, g_server_port);
		if (!construct_message_to_log_file("Connected to server on ")) return FALSE;
		break;
	case SERVER_MAIN_MENU: // SERVER_MAIN_MENU - show main menu
		show_main_menu();
		*timeout = TEN_MINUTES;
		break;
	case SERVER_DENIED: // SERVER_DENIED - start gracefully disconnection
		if (!shutdown_client()) return FALSE;
		break;
	case GAME_STARTED: //GAME_STARTED - print game is on
		printf("Game is on!\n");
		break;
	case TURN_SWITCH: // TURN_SWITCH - print which player's turn is
		if (STRINGS_ARE_EQUAL(par1, gp_username)) {
			printf("Your turn!\n");
		}
		else {
			printf("%s's turn!\n", par1);
		}
		break;
	case SERVER_MOVE_REQUEST: //SERVER_MOVE_REQUEST - client is asked for a move - insert number or boom

		get_player_valid_move();
		g_client_state = CLIENT_PLAYER_MOVE; // the player has played to the client state changes to CLIENT_PLAYER_MOVE
		break;
	case GAME_ENDED: //GAME_ENDED - prints who won
		printf("%s won!\n", par1);
		break;
	case SERVER_NO_OPPONENTS: 
		break;
	case GAME_VIEW: // GAME_VIEW - prints the last move
		printf("%s move was %s\n"
			"%s\n", par1, par2, par3);
		break;
	case SERVER_OPPONENT_QUIT: // SERVER_OPPONENT_QUIT - prints if the other player has quitted
		printf("Opponent quit.\n");
		break;
	}
	return TRUE;
}