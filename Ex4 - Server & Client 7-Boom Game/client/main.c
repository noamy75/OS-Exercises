/* Authors:
Ron Lewinsohn: 313291759
Noam Yakar: 318418027

Project: client

Description: This module connects to the server and handles the communication in the name of the player.
*/

#include "main.h"
#include "HardCodedData.h"
#include <SharedFunctions.h>
#include "ClientGame.h"
#include "ClientFile.h"

char g_user_input[MAX_INPUT_LENGTH] = { 0 };
SOCKET m_socket;
char gp_username[MAX_USERNAME_LENGTH] = { 0 };
char* gp_server_address;
int g_server_port;
message_type_e g_client_state = CLIENT_REQUEST;
BOOL g_finish_send_thread = FALSE;
BOOL g_no_errors = TRUE;
HANDLE g_client_log = NULL;
BOOL g_socket_is_closed = TRUE;
connection_status_e g_connection_status = CONNECT_SUCCESS;
char* gp_first_command[MAX_MSG_TYPE_LENGTH + MAX_USERNAME_LENGTH] = { 0 };

static DWORD recv_data_thread(void);
static DWORD send_data_thread(void);

/* This function handles he close_socket operation.
* Return TRUE if it succeed without errors
*/
BOOL close_socket()
{
	if (!g_socket_is_closed) // if socket is still open
	{
		if (0 != closesocket(m_socket)) // try to close socket
		{ // if close socket routine failed
			printf("ERROR: closesocket\n");
			write_to_client_log_file("ERROR: close_socket failed\n", FALSE, FALSE);
			g_no_errors = FALSE;
			return FALSE;
		}
	}
	return TRUE;
}

/* This function prints reconnect or exit menu, client need to choose 1 if he wants to try to reconnect or 2 if he wants to quit
* Returns the chosen connection status
*/
connection_status_e choose_reconnect_or_exit()
{
	int decision = 0;
	char decision_str[MAX_INPUT_LENGTH] = { 0 };
	do {
		// print the menu
		printf("Choose what to do next:\n"
			"1. Try to reconnect\n"
			"2. Exit\n");
		// get input from client
		gets_s(decision_str, MAX_INPUT_LENGTH);
		if (STRINGS_ARE_EQUAL(decision_str, "1")) { // client wants to try to reconnect
			decision = atoi(decision_str);
		}
		else if (STRINGS_ARE_EQUAL(decision_str, "2")) { // client wants to exit
			decision = atoi(decision_str);
		}
		else { // client inserted invalid input
			printf("Error: Illegal command\n");
			write_to_client_log_file("Error: Illegal command\n", FALSE, FALSE);
		}
	} while (decision != RECONNECT && decision != EXIT); //client will be able to choose again until he inserts valid input
	return decision;
}


/* The function handles the connect to server routine
* Returns the connection status
*/
connection_status_e connect_to_server()
{
	// Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	g_socket_is_closed = FALSE;
	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		write_to_client_log_file("ERROR: call socket function failed\n", FALSE, FALSE);
		g_no_errors = FALSE;
		return EXIT;
	}
	/*
	 The parameters passed to the socket function can be changed for different implementations.
	 Error detection is a key part of successful networking code.
	 If the socket call fails, it returns INVALID_SOCKET.
	 The if statement in the previous code is used to catch any errors that may have occurred while creating
	 the socket. WSAGetLastError returns an error number associated with the last error that occurred.
	 */
	SOCKADDR_IN clientService;
	/* Create a sockaddr_in object clientService and set values. AF_INET is the Internet address family.*/
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(gp_server_address); //Setting the IP address to connect to
	clientService.sin_port = htons(g_server_port); //Setting the port to connect to.

	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s:%d.\n", gp_server_address, g_server_port);
		if (!construct_message_to_log_file("Failed connecting to server on "))
		{
			g_no_errors = FALSE;
			return EXIT;
		}
		return choose_reconnect_or_exit(); // if the connect routine has failed - let the client choose if he wants to reconnect or exit
	}
	return CONNECT_SUCCESS; // connection succeed
}

/* This function returns the transfer result according to the num bytes received on transfer
* this function called when trying to make gracefully disconection so num bytes suppose to be 0
*/
TransferResult_t trans_zero_or_error(int bytes)
{
	if (bytes == SOCKET_ERROR)
	{ // recv() function failed
		printf("trans_zero_or_error() -> recv() failed, error %d\n", WSAGetLastError());
		g_no_errors = FALSE;
		return TRNS_FAILED;
	}
	else if (bytes == 0)
		return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.
	else
	{ //transfer succeed, but the size is not zero.
		g_no_errors = FALSE;
		return TRNS_SUCCEEDED;
	}
	
}

/* This function starts the shutdown routine, it used from gracefully disconnection.
* Returns TRUE if function succeed, else FALSE
*/
BOOL shutdown_client()
{
	if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR) { // shutdown
		// shutdown failed
		printf("shutdown_client() shutdown failed with error %d\n", WSAGetLastError());
		g_no_errors = FALSE;
		write_to_client_log_file("ERROR: shutdown client failed\n", FALSE, FALSE);
		return FALSE;
	}
	// shutdown succeed
	return TRUE;
}

/* This function frees all allocated memory on receive thread
* parameters: pointers to the potentially dynamic allocated strings: accepted_str, par1, par2, par3
*/
void free_all_memory_recv(char** accepted_str, char** par1, char** par2, char** par3)
{
	free_str(par1);
	free_str(par2);
	free_str(par3);
	free_str(accepted_str);
}

/* This function shows the reconnect or exit menu to the client, if he wants to connect it will handle the connect to server routine, else it will change the receive and send thread status to finish
* Returns TRUE if the client wants to close the program, else FALSE
*/
BOOL reconnect_or_exit_handling()
{
	g_connection_status = choose_reconnect_or_exit(); // get connection status
	while (RECONNECT == g_connection_status)
	{ // if the client wants to reconnect so connect to server routine get called - if connection succeed so connection status will be changed to CONNECT_SUCCESS
		g_connection_status = connect_to_server();
	}
	if (CONNECT_SUCCESS == g_connection_status)
	{ // connected to the server - client state will be changed to CLIENT_REQUEST
		g_socket_is_closed = FALSE;
		g_client_state = CLIENT_REQUEST;
		return FALSE;
	}
	// client chose to exit
	g_finish_send_thread = TRUE;
	return TRUE;
}

/* Receive data thread routine - receive the buffer from the server and handles the logic for each client state
* Returns 0 if the thread finished his progress properly without any errors
*/
static DWORD recv_data_thread(void)
{
	TransferResult_t RecvRes; // recv() operation result
	int timeout = FIFTEEN_SECONDS; // receive wait timeout
	BOOL finish_recv_thread = FALSE; // finish recv_thread flag
	char* par1 = NULL; //parameter 1
	char* par2 = NULL; //parameter 2
	char* par3 = NULL; //parameter 3
	char* accepted_str = NULL; // accepted string from recv() operation
	while (!finish_recv_thread && g_no_errors)
	{ //while we want to receive data from server and no errors occurred
		if (g_client_state == TURN_SWITCH || g_client_state == SERVER_MAIN_MENU) timeout = TEN_MINUTES; // on those states the program is waiting for input from the user, so the wait time is 10 minutes
		RecvRes = ReceiveString(&accepted_str, m_socket, timeout); // call receive string operation
		if(g_client_state != SERVER_MAIN_MENU)	timeout = FIFTEEN_SECONDS; // update timeout to the default timeout - 15 seconds

		if (RecvRes == TRNS_FAILED)
		{ // transfer failed
			printf("recv_data_thread - Socket error while trying to write data to socket\n");
			write_to_client_log_file("ERROR: transfer failed on recv_data_thread.\n", FALSE, FALSE);
			g_no_errors = FALSE;
			break;
		}
		else if (RecvRes == TRNS_TIMEOUT || RecvRes == TRNS_UNEXPECTED_DISCONNECT)
		{ // the server has disconnected - close socket and try to reconnect if wanted
			if (FALSE == close_socket())
			{
				break;
			}
			g_socket_is_closed = TRUE; // turn on the socket is closed flag to prevent use closesocket() twice
			printf("Failed connecting to server on %s:%d\n", gp_server_address, g_server_port);
			if (!construct_message_to_log_file("Failed connecting to server on ")) break;
			finish_recv_thread = reconnect_or_exit_handling();
			free_all_memory_recv(&accepted_str, &par1, &par2, &par3); // free all allocated memory
			continue; // finish the loop
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{ // server closed his connection
			printf("Server disconnected. Exiting.\n");
			write_to_client_log_file("Server disconnected. Exiting.\n", FALSE, FALSE);
			finish_recv_thread = TRUE;
			g_finish_send_thread = TRUE;
			break;
		}
		if (STRINGS_ARE_EQUAL(FINAL_TRANSMISSION, accepted_str))
		{ // "final transmission" is received on gracefully disconnected
			char ft_buffer[4];
			int BytesTransferredExpectZero = recv(m_socket, ft_buffer, 4, 0);
			if (trans_zero_or_error(BytesTransferredExpectZero) != TRNS_DISCONNECTED)
			{ // after receiving "final transmission" we expect to receive 0 bytes.
				write_to_client_log_file("ERROR: client_disconnect() expected zero\n", FALSE, FALSE);
				printf("error: client_disconnect() expected zero\n");
				break;
			}
			else
			{ // 0 bytes received
				if (FALSE == close_socket()) break; // close the socket
				g_socket_is_closed = TRUE; // turn on the socket is closed flag to prevent use closesocket() twice
				finish_recv_thread = TRUE; // turn on the finish recv thread  flag
				if (g_client_state == SERVER_DENIED)
				{ // if the gracefully disconnection was after that SERVER_DENIED message has been received - continue to reconnect_or_exit_handling
					printf("Server on %s:%d denied the connection request.\n", gp_server_address, g_server_port);
					finish_recv_thread = reconnect_or_exit_handling();
				}
				free_all_memory_recv(&accepted_str, &par1, &par2, &par3); // free all allocated memory
				continue; // finish the loop
			}
		}

		if (!write_to_client_log_file(accepted_str, FALSE, TRUE)) // write the received message to the log file
		{ // write to log file failed
			printf("ERROR: recv_data_thread, write_to_client_log_file\n");
			break;
		}
		if (FALSE == parse_message_client(accepted_str, &par1, &par2, &par3)) break; // parse the message from server
		if(!execute_operation_client(&timeout, par1, par2, par3)) break; // execute the operation according to the message received from server
		free_all_memory_recv(&accepted_str, &par1, &par2, &par3); // free all allocated memory
	} // finish while loop
	if (g_no_errors == FALSE)
	{ // if the recv thread was finished because of an error
		free_all_memory_recv(&accepted_str, &par1, &par2, &par3); // free all allocated memory
		return 1; // return 1
	}
	return 0; // recv thread finished his progress properly - return 0
}

/* This function constructs the message that will be sent to server according to the client state
* Parameters: message buffer
*/
void construct_message_client(char* msg)
{
	switch (g_client_state)
	{
	case CLIENT_REQUEST: // CLIENT_REQUEST - send CLIENT_REQUEST:<username>
		strcpy_s(msg, sizeof("CLIENT_REQUEST:"), "CLIENT_REQUEST:");
		strcat(msg, gp_username);
		break;

	case CLIENT_VERSUS: // CLIENT_VERSUS - send CLIENT_VERSUS
		strcpy_s(msg, sizeof("CLIENT_VERSUS"), "CLIENT_VERSUS");
		break;

	case CLIENT_PLAYER_MOVE: // CLIENT_PLAYER_MOVE - send CLIENT_PLAYER_MOVE:<user input>
		strcpy_s(msg, sizeof("CLIENT_PLAYER_MOVE:"), "CLIENT_PLAYER_MOVE:");
		strcat(msg, g_user_input);
		memset(g_user_input, 0, sizeof(g_user_input));
		break;

	case CLIENT_DISCONNECT: // CLIENT_DISCONNECT - send CLIENT_DISCONNECT
		strcpy_s(msg, sizeof("CLIENT_DISCONNECT"), "CLIENT_DISCONNECT");
		break;

	}
	char temp_str[2] = { '\n', '\0' };
	strcat(msg, temp_str);
}

/*The function is the send data thread routine
* Returns 0 if the function finished his progress properly without any errors 
*/
static DWORD send_data_thread(void)
{
	char* send_str = NULL; // send_str buffer
	TransferResult_t SendRes; // transfer result

	while (!g_finish_send_thread && g_no_errors)
	{ //while we keep sending data to the server
		if (g_client_state == CLIENT_REQUEST ||
			g_client_state == CLIENT_VERSUS		 ||
			g_client_state == CLIENT_PLAYER_MOVE ||
			g_client_state == CLIENT_DISCONNECT)
		{ // if the client state is one of the states that will be sent to server - make the send routine, else wait for the client state to be changed.
			send_str = make_char_alloc(MAX_MSG_TYPE_LENGTH + strlen(gp_username) + strlen(g_user_input), &g_no_errors); // make memory dynamic allocation for the send string buffer
			if (send_str == NULL)
			{ // memory dynamic allocation failed
				printf("ERROR: make_char_alloc\n");
				write_to_client_log_file("ERROR: memory allocation failed\n", FALSE, FALSE);
				break;
			}
			construct_message_client(send_str); // construct the message that will be sent to the server
			SendRes = SendString(send_str, m_socket); // send the string

			if (SendRes == TRNS_FAILED)
			{ // send routine has failed
				printf("send_data_thread - Socket error while trying to write data to socket\n");
				write_to_client_log_file("ERROR: failed to transfer data to server\n", FALSE, FALSE);
				break;
			}
			if (!write_to_client_log_file(send_str, TRUE, TRUE)) // write the sent string to the log file
			{ // write_to_client_log_file failed
				printf("ERROR: send_data_thread, write_to_client_log_file\n");
				break;
			}
			if (g_client_state == CLIENT_DISCONNECT)
			{ // if CLIENT_DISCONNECT state has just been sent - start the disconnection routine
				if (!shutdown_client()) break; // shut down client
				g_finish_send_thread = TRUE; // stop the send thread
			}
			free_str(&send_str);
			g_client_state = CLIENT_HOLD; // change the client state to CLIENT_HOLD -> waiting for a new message to send to the server
		}
	}
	if (g_no_errors == FALSE)
	{ // if the thread finished his progress because of an error
		free_str(&send_str);
		return 1; // return 1
	}
	return 0; // send thread finished his progress properly - return 0
}

/*This function closes thread handles
Parameters: thread handles array*/
void close_handles(HANDLE* thread_handles)
{
	// close send thread handle
	close_one_handle(&thread_handles[SEND_THREAD], &g_no_errors);
	// close send thread handle
	close_one_handle(&thread_handles[RECV_THREAD], &g_no_errors);
	// close client log file handle
	close_one_handle(&g_client_log, &g_no_errors);
}


/* This function handling the finish program routine - closing handles, close socket and WSACleanup
* Parameter: thread handles array
*/
void finish_program_routine(HANDLE* thread_handles)
{
	close_handles(thread_handles); // close all open handles
	close_socket(); // close socket
	wsa_cleanup(&g_no_errors); // wsa cleanup
	if (g_no_errors == FALSE)
	{ // if an error occurred during the program - exit with 1
		exit(1);
	}
}

/* client program main function
*/
void main_client(char* argv[])
{
	// get command line arguments
	gp_server_address = argv[1];
	g_server_port = atoi(argv[2]);
	memcpy(gp_username, argv[3], strlen(argv[3]));

	// init thread handles and ids
	HANDLE thread_handles[NUM_THREADS];
	memset(thread_handles, 0, sizeof(HANDLE) * 2);
	DWORD thread_ids[NUM_THREADS];

	/* Initialize Winsock. Create a WSADATA object called wsaData. The WSADATA structure contains information
	about the Windows Sockets implementation.*/
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //Call WSAStartup and check for errors.
	if (iResult != NO_ERROR)
	{ // error occurred
		printf("Error at WSAStartup()\n");
		g_no_errors = FALSE;
		exit(1);
	}
	do
	{
		g_connection_status = connect_to_server(); // connect to server
		if (g_connection_status == EXIT)
		{ // connection has failed and wants to finish the program.
			g_no_errors = FALSE;
			finish_program_routine(thread_handles); // call finish program routine
		}
	} while (g_connection_status == RECONNECT); // try to reconnect to server while connection status is reconnect
	
	// if reached here - g_connection_status = CONNECTION_SUCCESS
	// open log file
	if (!file_opening_client(gp_username))
	{ // open log file has failed
		printf("ERROR: file_opening_client\n");
		finish_program_routine(thread_handles); // call finish program routine
	}
	// Send and receive data.
	/*
		In this code, two integers are used to keep track of the number of bytes that are sent and received.
		The send and recv functions both return an integer value of the number of bytes sent or received,
		respectively, or an error. Each function also takes the same parameters:
		the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.
	*/
	thread_handles[SEND_THREAD] = create_thread_simple((LPTHREAD_START_ROUTINE)send_data_thread, &thread_ids[SEND_THREAD]);
	if (thread_handles[SEND_THREAD] == NULL)
	{ // send thread creation failed
		printf("ERROR: create_thread_simple - SEND_THREAD\n");
		g_no_errors = FALSE;
		finish_program_routine(thread_handles);// call finish program routine
	}
	thread_handles[RECV_THREAD] = create_thread_simple((LPTHREAD_START_ROUTINE)recv_data_thread, &thread_ids[RECV_THREAD]);
	if (thread_handles[RECV_THREAD] == NULL)
	{ // recv thread creation failed
		printf("ERROR: create_thread_simple - RECV_THREAD\n");
		g_no_errors = FALSE;
		get_exit_code_per_thread(thread_handles, NUM_THREADS, &g_no_errors);
		finish_program_routine(thread_handles); // call finish program routine
	}

	//Wait for all active threads to finish their progress
	if (!wait_for_multiple_objects(thread_handles, NUM_THREADS, &g_no_errors, INFINITE))
	{ //if ERROR detected - need to terminate all threads, close handles and free memory
		printf("ERROR: wait_for_multiple_objects\n");
		get_exit_code_per_thread(thread_handles, NUM_THREADS, &g_no_errors);
		finish_program_routine(thread_handles);// call finish program routine
	}

	if (!get_exit_code_per_thread(thread_handles, NUM_THREADS, &g_no_errors))
	{ // get_exit_code_per_thread failed
		printf("ERROR: get_exit_code_per_thread\n");
	}
	finish_program_routine(thread_handles); // call finish program routine
	return;
}

/* main function
* Parameters: command line arguments
* Returns 0 if program finished properly
*/
int main(int argc, char* argv[])
{
	main_client(argv);
	return 0;
}