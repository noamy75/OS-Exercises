/*
Description: This module includes the relevant functions for file handling such as write to file, read from file
*/

/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>
#include <main.h>
#include <FileHandling.h>

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

/* The function reads a number from a file and converts it to int
Parameters: h_file - file handle, p_arg - pointer to a storing location, last_byte - a character that serves as a stopping point
Returns True if read operation succeed.
*/
BOOL read_from_file(HANDLE h_file, int* p_arg, char last_byte)
{
	char buffer_char[2] = { 0 };
	int nByteRead;
	char buffer_str[NUM_BITS_INT] = { 0 };
	do
	{// Read number from the file byte by byte
		if (!ReadFile(h_file, (void*)buffer_char, ONE_BYTE, &nByteRead, NULL))
		{ //If file reading failed.
			printf("ERROR: ReadFile failed\n");
			return FALSE;
		}
		strcat(buffer_str, buffer_char); // concatenate the read byte to the temporary number string
	} while (buffer_char[0] != last_byte); // do this loop until reaching last_byte -> finished reading number
	*p_arg = atoi(buffer_str); // convert the number string to an integer
	return TRUE;
}

/*This function reads all numbers of a line from the input file and places them in a struct containing memory call parameters
Parameters: h_input - input file handle, p_input_args - pointer to a struct containing memory call parameters
Return TRUE if read operation succeed
*/
BOOL read_line(HANDLE h_input, memory_call_t* p_input_args)
{
	if (!read_from_file(h_input, &p_input_args->time, ' '))
	{
		printf("ERROR:read_line: read_from_file 1 failed\n");
		return FALSE;
	}
	if (!read_from_file(h_input, &p_input_args->virtual_address, ' '))
	{
		printf("ERROR:read_line: read_from_file 2 failed\n");
		return FALSE;
	}
	if (!read_from_file(h_input, &p_input_args->time_of_use, '\n'))
	{
		printf("ERROR:read_line: read_from_file 3 failed\n");
		return FALSE;
	}
	p_input_args->page_number = p_input_args->virtual_address >> NUM_BITS_PER_PAGE;
	return TRUE;
}

/* This function sets the file pointer to the beggining of the file
Parameters: h_file - file handle
Returns TRUE if function succeeds, else returns FALSE
*/
BOOL set_file_offset(HANDLE h_file)
{
	DWORD returned_value = SetFilePointer(h_file, 0, NULL, FILE_BEGIN);
	if (returned_value == INVALID_SET_FILE_POINTER)
	{
		DWORD dwError = GetLastError();
		printf("ERROR set_file_offset\n");
		return FALSE;
	}
	return TRUE;
}

/*This function counts the number of lines in the input file
Parameters: h_input - input file handle, p_counter - pointer to a counter
Returns TRUE if function succeeds, else returns FALSE
*/
BOOL get_num_of_lines(HANDLE* h_input, int* p_counter)
{
	char buffer_char[2] = { 0 }; int nByteRead = -1;
	while (nByteRead != 0)
	{
		if (buffer_char[0] == '\n') // reached end of line
		{
			(*p_counter)++;
		}
		if (!ReadFile(*h_input, (void*)buffer_char, ONE_BYTE, &nByteRead, NULL)) // Read one byte
		{ // If file reading failed
			printf("ERROR get_num_of_lines\n");
			return FALSE;
		}
	}
	if (!set_file_offset(*h_input))
	{ // Settings the file pointer failed
		return FALSE;
	}
	return TRUE;
}

/*This function writes to the output file
Paramters: h_output - output file handle, time, page, frame, tav - P/E
*/
void write_to_file(HANDLE h_output, int time, int page, int frame, char* tav)
{
	char line_buffer[MAX_LINE_LENGTH] = { 0 };
	char page_buffer[NUM_BITS_INT] = { 0 };
	char frame_buffer[NUM_BITS_INT] = { 0 };
	snprintf(line_buffer, MAX_LINE_LENGTH, "%d", time); //convert int to array of char
	snprintf(page_buffer, NUM_BITS_INT, "%d", page); //convert int to array of char
	snprintf(frame_buffer, NUM_BITS_INT, "%d", frame); //convert int to array of char
	strcat(line_buffer, " ");
	strcat(line_buffer, page_buffer);
	strcat(line_buffer, " ");
	strcat(line_buffer, frame_buffer);
	strcat(line_buffer, " ");
	strcat(line_buffer, tav);
	strcat(line_buffer, "\r\n");
	if (!WriteFile(h_output, line_buffer, strlen(line_buffer), NULL, NULL))
	{// if writting failed
		printf("ERROR: Unable to write to file\n");
		ExitThread(1);
	}
}