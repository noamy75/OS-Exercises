/*
Description: This module includes functions and data necessary for both server and client for communication.
*/

#include <SharedFunctions.h>
#include "HardCodedData.h"

TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd)
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;
	while (RemainingBytesToSend > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send(sd, CurPlacePtr, RemainingBytesToSend, 0);
		if (BytesTransferred == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAECONNRESET)
			{
				return TRNS_UNEXPECTED_DISCONNECT;
			}
			else
			{
				printf("send() failed, error %d\n", WSAGetLastError());
				return TRNS_FAILED;
			}
		}

		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

TransferResult_t SendString(const char* Str, SOCKET sd)
{
	/* Send the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	TotalStringSizeInBytes = (int)(strlen(Str) + 1); // terminating zero also sent	

	SendRes = SendBuffer(
		(const char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // sizeof(int) 
		sd);

	if (SendRes != TRNS_SUCCEEDED) return SendRes;

	SendRes = SendBuffer(
		(const char*)(Str),
		(int)(TotalStringSizeInBytes),
		sd);

	return SendRes;
}

TransferResult_t ReceiveBuffer(char* OutputBuffer, int BytesToReceive, SOCKET sd, int timeout)
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;

	while (RemainingBytesToReceive > 0)
	{
		/* send does not guarantee that the entire message is sent */
		int to = timeout;
		setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&to, sizeof(int));
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if (BytesJustTransferred == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT) 
			{
				return TRNS_TIMEOUT;
			}
			else if (WSAGetLastError() == WSAECONNRESET)
			{
				return TRNS_UNEXPECTED_DISCONNECT;
			}
			else 
			{
				printf("recv() failed, error %d\n", WSAGetLastError());
			}
			return TRNS_FAILED;
		}
		else if (BytesJustTransferred == 0) {
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.
		}

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}
	return TRNS_SUCCEEDED;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd, int timeout)
{
	/* Recv the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;

	if ((OutputStrPtr == NULL) || (*OutputStrPtr != NULL))
	{
		printf("The first input to ReceiveString() must be "
			"a pointer to a char pointer that is initialized to NULL. For example:\n"
			"\tchar* Buffer = NULL;\n"
			"\tReceiveString( &Buffer, ___ )\n");
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	RecvRes = ReceiveBuffer(
		(char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // 4 bytes
		sd, timeout);

	if (RecvRes != TRNS_SUCCEEDED) return RecvRes;

	StrBuffer = (char*)malloc(TotalStringSizeInBytes * sizeof(char));

	if (StrBuffer == NULL)
		return TRNS_FAILED;
	memset(StrBuffer, 0, TotalStringSizeInBytes);
	RecvRes = ReceiveBuffer(
		(char*)(StrBuffer),
		(int)(TotalStringSizeInBytes),
		sd, timeout);

	if (RecvRes == TRNS_SUCCEEDED)
	{
		*OutputStrPtr = StrBuffer;
	}
	else
	{
		free_str(&StrBuffer);
	}
	return RecvRes;
}



/* This function opens the file with the desired access
Parameters: file name string, the desired access, creation disposition; active handle identifier
Returns the file handle.
*/
HANDLE file_opening(char* filename, DWORD desired_access, DWORD creation_disposition)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFileA(
		filename,              // file name 
		desired_access,        // reading/writting 
		0,                     // do not share 
		NULL,                  // default security 
		creation_disposition,  // reading - existing file only, writting - open always 
		FILE_ATTRIBUTE_NORMAL, // normal file 
		NULL);                 // no template 
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed\n");
		return NULL;
	}
	return hFile;
}

/* This function makes dynamic allocation of array of char
* Parameter: dynamic allocated memory size buffer, and a pointer to no_errors identifier
* Return NULL if malloc failed, else returns the address of the allocated memory
*/
char* make_char_alloc(int size, BOOL* no_errors)
{
	char* return_address = NULL;
	return_address = (char*)malloc(size * sizeof(char)); // Memory allocation
	if (return_address == NULL) // allocation failed
	{
		printf("ERROR: make_char_alloc\n");
		*no_errors = FALSE;
		return NULL;
	}
	// allocation succeed - return the address of the allocated memory
	return return_address;
}

// This function closes one handle
// Parameter: pointer to the handle
void close_one_handle(HANDLE* handle, BOOL* no_errors)
{
	if (*handle != NULL)
	{
		if (FALSE == CloseHandle(*handle))
		{ // send thread closing failed
			printf("ERROR: CloseHandle failed!!\n");
			*no_errors = FALSE;
			return;
		}
		*handle = NULL;
	}
}

/* The function creates a thread
* Parameters: thread's start routine, thread id and the thread argument
* Returns a thread handle
*/
HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id)
{
	if (NULL == p_start_routine)
	{ // If thread has no start routine
		printf("ERROR: Error when creating a thread");
		printf("Received null pointer");
		return NULL;
	}

	return CreateThread(
		NULL,            /*  default security attributes */
		0,               /*  use default stack size */
		p_start_routine, /*  thread function */
		NULL,/*  argument to thread function */
		0,               /*  use default creation flags */
		p_thread_id);    /*  returns the thread identifier */
}

/* Print the returned value from WaitForMultiplyObjects
* Parameter: wait code
*/
void print_multiple_object(DWORD wait_code)
{
	printf("WaitForMultipleObject output: \n");
	switch (wait_code)
	{
	case WAIT_TIMEOUT: //the thread takes too long
		printf("WAIT_TIMEOUT\n"); break;
	case WAIT_FAILED: //WaitForMultipleObject failed
		printf("WAIT_FAILED...\n");
		break;
	default:
		printf("0x%x\n", wait_code);
	}
}

void free_str(char** str)
{
	if (*str != NULL)
	{
		free(*str);
		*str = NULL;
	}
}

/* The function handles the WSACleanup operation.
*/
void wsa_cleanup(BOOL* no_errors)
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		*no_errors = FALSE;
	}
}

/* The function does the wait for multiple objects routine
* Parameters: num of active threads, array of thread handles
* Returns TRUE if all threads were signaled properly, else FALSE
*/
BOOL wait_for_multiple_objects(HANDLE* thread_handles, int num_threads, BOOL* no_errors, int wait_time)
{
	DWORD wait_code = WaitForMultipleObjects(num_threads, thread_handles, TRUE, wait_time);
	print_multiple_object(wait_code); // print wait code
	if (wait_code == WAIT_FAILED)
	{ //If not all threads are signaled
		printf("ERROR: WaitForExitCode failed!\n"
			"Terminating brutally!\n");
		*no_errors = FALSE;
		return FALSE;
	}
	return TRUE;
}

/* The function Gets the exit code per active thread and print it to the screen
* Parameters: num of active threads, array of thread handles
* Returns FALSE if an error occured during the function progress, else FALSE
*/
BOOL get_exit_code_per_thread(HANDLE* thread_handles, int num_threads, BOOL* no_errors)
{
	DWORD exitcode, wait_code;

	// Get the exit code per active thread and print it to the screen
	for (int num_thread_idx = 0; num_thread_idx < num_threads; num_thread_idx++)
	{
		if (thread_handles[num_thread_idx] != NULL)
		{
			//Get thread's exit code
			GetExitCodeThread(thread_handles[num_thread_idx], &exitcode);

			if (exitcode == STILL_ACTIVE)
			{ // Terminate thread if it is still active
				printf("Thread %d is still active, terminating thread!\n", num_thread_idx);
				if (TerminateThread(thread_handles[num_thread_idx], BRUTAL_TERMINATION_CODE))
				{ //if termination thread succeed
					wait_code = WaitForSingleObject(thread_handles[num_thread_idx], INFINITE); //wait for thread to be signaled
					print_multiple_object(wait_code); //print wait_code
					if (wait_code != WAIT_OBJECT_0)
					{// if wait_code is not 0
						printf("ERROR: wait for single object failed\n");
						*no_errors = FALSE;
						return FALSE;
					}
				}
				else
				{ //TerminateThread process failed
					printf("ERROR: TerminateThread function failed!!!\n");
					*no_errors = FALSE;
					return FALSE;
				}
			}
			else if (exitcode == 0)
			{
				printf("THREAD %d completed its' progress\n", num_thread_idx);
			}
		}

	}
	return TRUE;
}