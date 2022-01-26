
/*
* This module contains the finish server program handling functions
*/

#include "ServerFinishHandling.h"


/* This is the main function of the "wait for exit" thread, it lets the user to write "exit" and finish the server program
* Returns 0 when "exit" is written and no errors occurred
*/
int wait_for_exit()
{
	char input[MAX_INPUT_LENGTH] = { 0 };
	do
	{ // get input from user
		gets_s(input, MAX_INPUT_LENGTH);
	} while (!STRINGS_ARE_EQUAL(input, "exit") && g_no_errors); // stop when the input is exit or when error occurred

	g_server_active_flag = FALSE; // deactivating server
	return 0;
}

/* This function closes the server handles - mutex and threads
*/
void close_server_handles()
{
	close_one_handle(&g_client_input_mutex, &g_no_errors); // close mutex
	for (int i = 0; i < THREAD_MAX; i++)
	{ // close each thread handle
		close_one_handle(&g_thread_handles[i], &g_no_errors);
	}
}


/* This function is the finish server program routine
* closes the server handles and closes the main socket
*/
void finish_server_program_routine()
{
	free_str(&gp_client_input); // free client input
	close_one_handle(&g_thread_logs[0], &g_no_errors); // close thread log handle
	close_one_handle(&g_thread_logs[1], &g_no_errors); // close thread log handle
	close_one_handle(&g_thread_logs[2], &g_no_errors); // close thread log handle
	close_socket_server(&g_accept_socket[0]); // close thread socket
	close_socket_server(&g_accept_socket[1]); // close thread socket
	close_socket_server(&g_accept_socket[2]); // close thread socket
	close_server_handles(); // close the server handles
	close_socket_server(&g_main_socket); // close main socket
	wsa_cleanup(&g_no_errors);
	if (g_no_errors == FALSE)
	{ // if an error occurred during the server program - finish the server program with 1
		exit(1);
	}
}

/* The function handles the service thread finish routine.
* Parametrs: pointers to send_str, par, accepted_str, thread input
*/
void finish_program_service_thread(char** send_str, thread_input_t* thread_input, char** par, char** accepted_str)
{
	free_str(send_str); // free send_str
	free_str(par); // free par
	free_str(accepted_str); // free accepted_str
	lock_mutex(&g_client_input_mutex); // lock mutex
	free_str(&gp_client_input); // free client input
	release_mutex(&g_client_input_mutex); // release mutex
	close_one_handle(&g_thread_logs[thread_input->num_player], &g_no_errors); // close thread log handle
	close_socket_server(&g_accept_socket[thread_input->num_player]); // close socket
	if (g_no_errors == FALSE) // if the service thread has finished because of an error - exit code is 1
	{
		printf("ERROR\n");
		ExitThread(1);
	}
}

/* The function closes a socket
* Parameter: pointer to a socket
* Return TRUE if closing procedure succeed, else FALSE
*/
BOOL close_socket_server(SOCKET* sock)
{
	// if socket i valid
	if (*sock != INVALID_SOCKET)
	{
		if (0 != closesocket(*sock)) // try to close socket
		{ // if close socket routine failed
			printf("ERROR: closesocket\n");
			g_no_errors = FALSE;
			return FALSE;
		}
		// change the socket value to invalid
		*sock = INVALID_SOCKET;
	}
	return TRUE;
}

/* This function is used for grace disconnection from client
* Parameters: pointer to the socket
* Returns TRUE if the function succeed, else FALSE
*/
BOOL grace_disconnect_from_client(SOCKET* socket_input)
{
	const char ft_buffer[] = FINAL_TRANSMISSION;
	SendString(ft_buffer, *socket_input); // sending "final transmission" to the client
	if (shutdown(*socket_input, SD_SEND) == SOCKET_ERROR) { // shut down the sending option
		// shutdown failed
		printf("grace_disconnect_from_client() shutdown failed with error %d\n", WSAGetLastError());
		g_no_errors = FALSE;
		return FALSE;
	}
	if (!close_socket_server(socket_input)) return FALSE; // close the socket
	return TRUE;
}