/* Authors:
Ron Lewinsohn: 313291759
Noam Yakar: 318418027

Project: server

Description: This module handles new connections and disconnections of clients and manages a game of 7-boom.
*/

#include "main.h"
#include "HardCodedData.h"
#include "ServerFinishHandling.h"
#include "ServerCommandHandling.h"


char g_players_name[MAX_NUM_CONNECTED_PLAYERS][MAX_USERNAME_LENGTH] = { 0 }; // players name
char* gp_client_input = NULL; //the client response to a move request
HANDLE g_client_input_mutex = NULL; // client input mutex
int g_match_count = 0; // match count
int g_client_input_counter = 0; //client input counter - used for free the client input dynamic allocated memory after GAME_VIEW was sent to both players
int g_num_of_connected_players = 0; // number of connected players
int g_num_of_active_players = 0; // number of active players
BOOL g_server_active_flag = TRUE; // turns to FALSE when server is getting an exit from the user
BOOL g_socket_accepted_flag = FALSE; // turns TRUE when a client got connected to server
BOOL g_valid_move = TRUE; // turns FALSE when a player make a invalid move
BOOL g_player_turn[MAX_NUM_CONNECTED_PLAYERS] = { 0 }; // identifier of whose turn is
BOOL g_player_made_move; // turns TRUE when a player made a move
BOOL g_no_errors = TRUE; // turns FALSE when an unexpected error occurred
HANDLE g_thread_handles[THREAD_MAX] = { NULL }; // thread handles
SOCKET g_main_socket = INVALID_SOCKET; // main socket
BOOL g_player_wants_to_play[MAX_NUM_CONNECTED_PLAYERS] = { FALSE }; // TURNS true when a player returns CLIENT_VERSUS
HANDLE g_thread_logs[MAX_NUM_CONNECTED_PLAYERS] = { NULL };
SOCKET g_accept_socket[MAX_NUM_CONNECTED_PLAYERS]; // accepted socket
int g_socket_counter = 0; // socket counter
int g_num_of_active_threads = 0;
message_type_e g_state_per_client[MAX_NUM_CONNECTED_PLAYERS]; //array of state identifier for each client

/* The function creates a mutex handle
* Returns TRUE if creation succeed, else returns FALSE
*/
BOOL create_mutex(HANDLE* mutex)
{
	/* Create the mutex that will be used to synchronize access to the page table */
	*mutex = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == *mutex)
	{ //mutex creation failed
		printf("ERROR: Error when creating mutex: %d\n", GetLastError());
		g_no_errors = FALSE;
		return FALSE;
	}
	return TRUE;
}

/* This function locks the mutex
Parameter: Pointer to the mutex
returns TRUE if lock succeed, else FALSE
*/
BOOL lock_mutex(HANDLE* mutex)
{
	int wait_code = WaitForSingleObject(*mutex, INFINITE);
	if (WAIT_OBJECT_0 != wait_code)
	{ //If locking procedure failed
		printf("ERROR lock_mutex\n");
		g_no_errors = FALSE;
		return FALSE;
	}
	return TRUE;
}

/* This function releases the mutex
Parameter: Pointer to the mutex
returns TRUE if release succeed, else FALSE
*/
BOOL release_mutex(HANDLE* mutex)
{
	BOOL ret_val = ReleaseMutex(*mutex);
	if (FALSE == ret_val)
	{ //if the mutex releasing procedure failed
		printf("ERROR: Error when releasing mutex\n");
		g_no_errors = FALSE;
		return FALSE;
	}
	return TRUE;
}

/* This function finds the first unused thread
* Returns the first available thread idx
*/
static int find_first_available_client()
{
	int idx;
	for (idx = THREAD_PLAYER_OFFSET; idx < THREAD_MAX; idx++)
	{ // go over the thread and check which thread is not NULL and finished running
		if (g_thread_handles[idx] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(g_thread_handles[idx], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				close_one_handle(&g_thread_handles[idx], &g_no_errors);
				g_thread_handles[idx] = NULL;
				break;
			}
		}
	}
	g_num_of_active_threads--;
	return idx;
}

/* This function is the thread listening routine, this routine is used for listening on the socket for a new client,
	accepting if its valid and let the main thread to create a new thread for the new client
* Returns 0 if the function finished properly
*/
int thread_listening()
{
	SOCKET socket_accepted = INVALID_SOCKET;
	int ListenRes;
	while (g_no_errors && g_server_active_flag)
	{ // while the server is still active and no errors occurred on program
		// Listen on the Socket.
		ListenRes = listen(g_main_socket, SOMAXCONN);
		if (ListenRes == SOCKET_ERROR)
		{ // listening failed
			printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
			g_no_errors = FALSE;
			ExitThread(1);
		}
		// accept
		socket_accepted = accept(g_main_socket, NULL, NULL);
		if (socket_accepted == INVALID_SOCKET)
		{ // accept failed
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			g_no_errors = FALSE;
			ExitThread(1);
		}
		for (int idx = 0; idx < MAX_NUM_CONNECTED_PLAYERS; idx++)
		{
			if (g_accept_socket[idx] == INVALID_SOCKET)
			{
				g_accept_socket[idx] = socket_accepted;
				break;
			}
		}
		// turn the accepted socket flag to TRUE
		g_socket_accepted_flag = TRUE;
	}
	return 0;
}

/* The function checks if a boom is expected in the following move
* Returns TRUE if boom is expected, else FALSE
*/
BOOL is_boom_expected()
{
	if (g_match_count % 7 == 0)
	{ // check if the expected move is divisible by 7 with no remainder
		return TRUE;
	}
	int temp = g_match_count;
	// check if the expected move contains the digit '7'
	while (temp > 0)
	{
		if (temp % 10 == 7)
		{
			return TRUE;
		}
		temp /= 10;
	}
	return FALSE;
}

/* This function checks if the player move was a valid move
* Parameter: player move string
* Returns TRUE if the move was valid
*/
BOOL check_player_move_validity(char* player_move)
{
	BOOL is_boom = is_boom_expected(); // check if the next move suppose to be boom
	if (is_boom)
	{ // boom is expected
		if (STRINGS_ARE_EQUAL(player_move, "boom"))
		{ // check if the user input was "boom"
			return TRUE; // return TRUE
		}
		return FALSE; // boom was expected but the user didn't write "boom"
	}
	else if (atoi(player_move) == g_match_count) // boom is not expected - check if the user inserted the expected move
	{
		return TRUE;
	}
	return FALSE;
}
/* This function opens the thread log file according to the user name.
* Parameters: thread_input , username
* Returns TRUE if the function succeed, else FALSE
*/
BOOL file_opening_server(thread_input_t* thread_input, char* username)
{
	char file_name[MAX_FILE_NAME_LENGTH] = "Thread_log_"; // the log file name starts with "Thread_log_<username>"
	strcat(file_name, username); // concatenate the user name
	strcat(file_name, ".txt"); // concatenate .txt
	g_thread_logs[thread_input->num_player] = file_opening(file_name, GENERIC_WRITE, OPEN_ALWAYS); // open the log file for writting
	if (g_thread_logs[thread_input->num_player] == NULL)
	{ // if the file opening failed
		printf("ERROR: file_opening_server");
		g_no_errors = FALSE;
		return FALSE;
	}
	return TRUE;
}


/*This function writes to the output file
Paramters: log file handle, message and identifier if it sent to client and identifier of if it a RECEIVED/SENT message or another message (can be error message)
Returns TRUE if the function succeed, else FALSE
*/
BOOL write_to_log_file(HANDLE thread_log, char* message, BOOL is_sent_to_client, BOOL network_message)
{
	char* line_buffer = NULL;
	line_buffer = make_char_alloc(strlen(message) + strlen("received from client-") + 1, &g_no_errors); //make memory allocation to line_buffer
	if (NULL == line_buffer)
	{ // memory allocation failed
		printf("write_to_log_file - make_char_alloc() returned NULL, exitting thread");
		g_no_errors = FALSE;
		return FALSE;

	}
	if (network_message)
	{
		if (is_sent_to_client) //check if it sent from server or sent to server
		{
			strcpy(line_buffer, "sent to client-");
		}
		else
		{
			strcpy(line_buffer, "received from client-");
		}
		strcat(line_buffer, message); // concatenate the message
	}
	else
	{
		strcpy(line_buffer, message);
	}
	if (thread_log != NULL)
	{
		if (!WriteFile(thread_log, line_buffer, strlen(line_buffer), NULL, NULL))
		{// if writting failed
			printf("ERROR: Unable to write to file\n");
			free_str(&line_buffer);
			g_no_errors = FALSE;
			return FALSE;
		}
	}
	free_str(&line_buffer); // free allocated memory
	return TRUE;
}

/* The main function of a client thread
* Parameter: thread input
* Returns 0 if the thread finished without errors
*/
int thread_service(thread_input_t* thread_input)
{
	TransferResult_t trans_res;
	char* send_str = NULL;
	char* accepted_str = NULL;
	char* par = NULL;
	int timeout = FIFTEEN_SECONDS; // the default timeout is 15 seconds
	// while the client process is active, there are no errors and the server is active
	while (thread_input->client_is_active && g_no_errors && g_server_active_flag)
	{
		if (waiting_for_data_from_client(thread_input, &timeout)) // check if the server expect to receive data from client
		{
			if (g_state_per_client[thread_input->num_player] == TURN_SWITCH) // continue to next loop on TURN_SWITCH
			{
				continue;
			}
			if (!wait_for_receive_data(&accepted_str, thread_input, timeout)) break; // receive data from client, break if it fails
			if (thread_input->client_is_active == FALSE || g_server_active_flag == FALSE) continue; // if the client is not active anymore break or server is not active anymore
			if (g_state_per_client[thread_input->num_player] != SERVER_OPPONENT_QUIT)
			{
				if (!parse_command(accepted_str, thread_input, &par)) break; // parse the input from client, break if it fails
				if (g_num_of_connected_players < 2 && g_state_per_client[thread_input->num_player] == CLIENT_REQUEST) // open log file on client request
				{
					if (!file_opening_server(thread_input, par)) break; // open log file
				}
				if (g_thread_logs[thread_input->num_player] != NULL)
				{
					if (!write_to_log_file(g_thread_logs[thread_input->num_player], accepted_str, FALSE, TRUE)) break; // write to log file
				}
				if (!execute_operation(thread_input, par)) break; // execute the operation
			}
		}
		if (thread_input->client_is_active == FALSE) continue; // if the client is not active anymore break
		// make dynamic memory allocation with a size that changes according to the state
		if (g_state_per_client[thread_input->num_player] == GAME_VIEW) 
			send_str = make_char_alloc(strlen(gp_client_input) + strlen("CONITNUE") + MAX_MSG_TYPE_LENGTH + MAX_USERNAME_LENGTH + 4, &g_no_errors);
		else
			send_str = make_char_alloc(MAX_MSG_TYPE_LENGTH + MAX_USERNAME_LENGTH + 3, &g_no_errors);
		if (send_str == NULL)
		{
			write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: dynamic memory allocation failed\n", FALSE, FALSE);
			break; // if the memory allocation fails
		}
		if (!construct_message(thread_input, send_str)) break; // construct message to send to client, break if it fails
		
		if (g_state_per_client[thread_input->num_player] != CLIENT_DISCONNECT)
		{
			trans_res = SendString(send_str, g_accept_socket[thread_input->num_player]); // send the string
			if (trans_res == TRNS_FAILED)
			{ //transfer fails
				printf("Service socket error while writing, closing thread.\n");
				write_to_log_file(g_thread_logs[thread_input->num_player], "ERROR: failed to transfer to string\n", FALSE, FALSE);
				g_no_errors = FALSE;
				break;
			}
			else if (trans_res == TRNS_SUCCEEDED)
			{
				if (g_thread_logs[thread_input->num_player] != NULL)
				{
					if (!write_to_log_file(g_thread_logs[thread_input->num_player], send_str, TRUE, TRUE)) break; // write to log file
				}
			}
		}
		free_str(&send_str); // free send_str
		free_str(&par); // free par
		free_str(&accepted_str); // free accepted_str
	} // finish while loop
	g_num_of_connected_players--; // decrease number of connected players 
	if (g_server_active_flag && g_state_per_client[thread_input->num_player] != SERVER_DENIED)
	{
		printf("Player disconnected. Exiting.\n");
		write_to_log_file(g_thread_logs[thread_input->num_player], "Player disconnected. Exiting.\n", FALSE, FALSE);
	}
	finish_program_service_thread(&send_str, thread_input, &par, &accepted_str); // finish service thread handling
	return 0;
}

/* This function does the bind socket routine
* Parameters: pointer to service and port number
* Returns TRUE if the function succeed, else FALSE
*/
BOOL bind_socket(SOCKADDR_IN* service, int port)
{
	unsigned long address;
	int bindRes;

	// Bind the socket.
	/*
	For a server to accept client connections, it must be bound to a network address within the system.
	The following code demonstrates how to bind a socket that has already been created to an IP address
	and port.
	Client applications use the IP address and port to connect to the host network.
	The sockaddr structure holds information regarding the address family, IP address, and port number.
	sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
	*/
	// Create a sockaddr_in object and set its values.
	// Declare variables

	address = inet_addr(SERVER_ADDRESS_STR);
	if (address == INADDR_NONE)
	{ // inet_addr failed
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		return FALSE;
	}

	service->sin_family = AF_INET;
	service->sin_addr.s_addr = address;
	service->sin_port = htons((u_short)port); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	/*
		The three lines following the declaration of sockaddr_in service are used to set up
		the sockaddr structure:
		AF_INET is the Internet address family.
		"127.0.0.1" is the local IP address to which the socket will be bound.
		the input port is the port number to which the socket will be bound.
	*/

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	bindRes = bind(g_main_socket, (SOCKADDR*)service, sizeof(*service));
	if (bindRes == SOCKET_ERROR)
	{ // bind failed
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

/* This function creates a new client thread
* Returns TRUE if the function succeed, else FALSE
*/
BOOL create_new_client_thread(thread_input_t* thread_input)
{
	int idx = find_first_available_client(); // find the first avaiable client idx
	int player_idx = idx - THREAD_PLAYER_OFFSET;
	if (g_socket_accepted_flag) // if socket was accepted - create new client
	{
		thread_input[player_idx].num_player = player_idx; // save the player idx
		thread_input[player_idx].client_is_active = TRUE; // client is active
		g_state_per_client[player_idx] = SERVER_HOLD; // initial state is SERVER_HOLD
		g_thread_handles[idx] = CreateThread( // create thread
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)thread_service,
			(&thread_input[player_idx]),
			0,
			NULL
		);
		if (g_thread_handles[idx] == NULL)
		{ // if creation failed
			g_no_errors = FALSE;
			return FALSE;
		}
		g_socket_accepted_flag = FALSE; // turn off the socket accepted flag
		//g_socket_counter = (g_socket_counter + 1) % MAX_NUM_CONNECTED_PLAYERS; // increment the socket counter	
	}
	return TRUE;
}

/* The main function of the server program
* Parameter: command line arguments
*/
void server_main(char* argv[])
{
	// init
	int port = atoi(argv[1]);
	memset(g_accept_socket, INVALID_SOCKET, MAX_NUM_CONNECTED_PLAYERS * sizeof(SOCKET));
	SOCKADDR_IN service;
	DWORD thread_ids[THREAD_MAX];
	thread_input_t thread_input[MAX_NUM_CONNECTED_PLAYERS];
	memset(thread_input, 0, MAX_NUM_CONNECTED_PLAYERS * sizeof(thread_input_t));
	// Initialize Winsock.
	WSADATA wsa_data;
	int startup_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (startup_result != NO_ERROR)
	{ //WSAStartup failed
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		exit(1);
	}
	if (!create_mutex(&g_client_input_mutex))
	{ // create_mutex failed
		printf("ERROR: create_mutex\n");
		finish_server_program_routine();
	}
	// Create a socket.    
	g_main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_main_socket == INVALID_SOCKET)
	{ // socket failed
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		g_no_errors = FALSE;
		finish_server_program_routine();
	}
	// bind socket
	if (!bind_socket(&service, port))
	{ // bind socket failed
		printf("ERROR: bind_socket\n");
		g_no_errors = FALSE;
		finish_server_program_routine();
	}
	g_thread_handles[THREAD_WAIT_FOR_EXIT] = create_thread_simple((LPTHREAD_START_ROUTINE)wait_for_exit, &thread_ids[THREAD_WAIT_FOR_EXIT]); // create wait_for_exit thread
	if (NULL == g_thread_handles[THREAD_WAIT_FOR_EXIT])
	{ // create_thread_simple failed.
		printf("ERROR: create_thread_simple - THREAD_WAIT_FOR_EXIT\n");
		g_no_errors = FALSE;
		finish_server_program_routine();
	} 
	g_num_of_active_threads++;
	g_thread_handles[THREAD_LISTENING] = create_thread_simple((LPTHREAD_START_ROUTINE)thread_listening, &thread_ids[THREAD_LISTENING]); // create thread_listening thread
	if (NULL == g_thread_handles[THREAD_LISTENING])
	{ // chreate_thread_simple failed
		printf("ERROR: create_thread_simple - THREAD_WAIT_FOR_EXIT\n");
		g_no_errors = FALSE;
		get_exit_code_per_thread(g_thread_handles, THREAD_MAX, &g_no_errors);
		finish_server_program_routine();
	}
	g_num_of_active_threads++;
	while (g_server_active_flag && g_no_errors)
	{ // while the server is active and no errors occurred
		if (g_socket_accepted_flag == FALSE) continue; // while socket hasn't accepted yet, continue to another loop
		if (!create_new_client_thread(thread_input)) // socket accepted - create new client thread
		{
			printf("ERROR: create_new_client_thread\n");
			get_exit_code_per_thread(g_thread_handles, THREAD_MAX, &g_no_errors);
			finish_server_program_routine();
		}
		g_num_of_active_threads++;
	}
	if (g_no_errors == FALSE)
	{ // the program is stopping because of an error
		get_exit_code_per_thread(g_thread_handles, THREAD_MAX, &g_no_errors);
		finish_server_program_routine(); // finish server program
	}
	//Wait for all active threads to finish their progress
	if (!wait_for_multiple_objects(g_thread_handles, g_num_of_active_threads, &g_no_errors, FIFTEEN_SECONDS))
	{ //if ERROR detected - need to terminate all threads, close handles and free memory
		printf("ERROR: wait_for_multiple_objects\n");
		get_exit_code_per_thread(g_thread_handles, THREAD_MAX, &g_no_errors);
		finish_server_program_routine();// call finish program routine
	}
	if (!get_exit_code_per_thread(g_thread_handles, THREAD_MAX, &g_no_errors))
	{ // get_exit_code_per_thread failed
		printf("ERROR: get_exit_code_per_thread\n");
	}
	finish_server_program_routine();
}

/* main function
* Parameters: command line arguments
* Returns 0 if program finished properly
*/
int main(int argc, char* argv[])
{
	server_main(argv);
	return 0;
}