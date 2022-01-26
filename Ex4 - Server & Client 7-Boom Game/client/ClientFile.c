
/*
* Description : this module contains the file handling functions from client side.
*/

#include "ClientFile.h"
#include "main.h"

/*This function writes to the output file
Paramters: output message, is sent from client or received from server.
Returns TRUE if the write_to_client_log_file routine succeed, else FALSE
*/
BOOL write_to_client_log_file(char* message, BOOL is_sent_to_server, BOOL network_message)
{
	char* line_buffer = NULL;
	line_buffer = make_char_alloc(strlen(message) + strlen("received from server-") + 1, &g_no_errors); // make memory allocation for line_buffer  according to the received/sent message
	if (NULL == line_buffer)
	{ //memory dynamic allocation failed
		printf("write_to_client_log_file - make_char_alloc() returned NULL, exitting thread");
		return FALSE;
	}
	if (network_message)
	{
		if (is_sent_to_server)
		{ //if this function called from the send data thread
			strcpy(line_buffer, "sent to server-");
		}
		else
		{ // this function called from the recv data thread
			strcpy(line_buffer, "received from server-");
		}
		strcat(line_buffer, message); // concatenate the message content
	}
	else
	{
		strcpy(line_buffer, message);
	}

	if (!WriteFile(g_client_log, line_buffer, strlen(line_buffer), NULL, NULL)) // write to client log file
	{// if writting failed
		printf("ERROR: Unable to write to file\n");
		free_str(&line_buffer);
		g_no_errors = FALSE;
		return FALSE;
	}
	free_str(&line_buffer);
	return TRUE;
}

/* Consturct <IP:port> string
* Parameter array of char that this string will be written to
*/
void construct_ip_port_str(char* par)
{
	strcpy(par, gp_server_address); // copy the IP
	strcat(par, ":"); // concatenate ':'
	char port_str[PORT_LEN] = { 0 }; // init port string
	snprintf(port_str, PORT_LEN, "%d", g_server_port); // make port string
	strcat(par, port_str); // concatenate par to port string
}

/* Construct connection message to log file and write it to log file
* Parameter: connection message string
* Returns TRUE if succeed, else FALSE
*/
BOOL construct_message_to_log_file(char* par)
{
	char constructed_message[FAILED_CONNECTING_TO_SERVER_LEN + IP_LEN + PORT_LEN] = { 0 };
	char ip_port_str[IP_LEN + PORT_LEN + 1] = { 0 };
	strcpy(constructed_message, par);
	construct_ip_port_str(ip_port_str);
	strcat(constructed_message, ip_port_str);
	strcat(constructed_message, "\n");
	if (FALSE == write_to_client_log_file(constructed_message, FALSE, FALSE)) return FALSE;
	return TRUE;
}

/* This function constructs the file name and handles the log file opening routine for the client
Parameter: username
Returns TRUE if the log file opening routine finished properly, else FALSE
*/
BOOL file_opening_client(char* username)
{
	// The file name is "Client_log_<username>.txt
	char file_name[MAX_FILE_NAME_LENGTH] = "Client_log_";
	strcat(file_name, username);
	strcat(file_name, ".txt");

	g_client_log = file_opening(file_name, GENERIC_WRITE, OPEN_ALWAYS); // open the log faile for writting
	if (g_client_log == NULL)
	{ // file opening has failed
		printf("ERROR: file_opening_client");
		g_no_errors = FALSE;
		return FALSE;
	}
	return TRUE;
}