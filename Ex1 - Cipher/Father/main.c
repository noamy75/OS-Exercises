#include <HardCodedData.h>

/* Authors:
Ron Lewinsohn: 313291759
Noam Yakar: 318418027

Project: Father

Description: This module gets the command line arguments, creates a new son process
while the plaintext EOF haven't been reached.
*/


//variables
static int g_process_counter = 0; // counter of the finished son processes
static TCHAR exe[9] = { 0 }; // declation of Son program as an array of TCHAR
static BOOL havent_reached_eof_flag = TRUE; //should start a new process signal

//global variables

extern HANDLE g_h_plaintext_file; //plaintext file handle
extern TCHAR* pg_key; //key file name
extern TCHAR* pg_command; //command line
extern TCHAR* pg_plaintext; //plaintext file name
extern int g_allocated_tchar_bitmask; //bitmask of allocated memory variables
extern int g_active_handle_bitmask; //bitmask of active handles

//functions
BOOL create_process_simple(LPTSTR CommandLine, PROCESS_INFORMATION* ProcessInfoPtr);
TCHAR* make_alloc(DWORD len, allocated_memory_e allocated_mem);
void close_process_handles(PROCESS_INFORMATION procinfo);
void create_process_main(char* argv[]);
BOOL havent_reached_eof(char* plaintext_file);
void get_son_process_arguments(char* argv[]);
void father_process_main(char* argv[]);


/*main function - father process start
Parameters: command line arguments
Returns 0 if process is finished
*/
int main(int argc, char* argv[])
{
	printf("Started father process\n");
	father_process_main(argv);
	printf("finished father process\n");
	return 0;
}


/*This function creates the Son process
Parameters: son process' command line, process information pointer
Returns TRUE if process creation succeeds, else returns FALSE
*/

BOOL create_process_simple(LPTSTR CommandLine, PROCESS_INFORMATION* ProcessInfoPtr)
{
	STARTUPINFO	startinfo = { sizeof(STARTUPINFO), NULL, 0 }; 
	return CreateProcess(NULL, /*  No module name (use command line). */
		CommandLine,			/*  Command line. */
		NULL,					/*  Process handle not inheritable. */
		NULL,					/*  Thread handle not inheritable. */
		FALSE,					/*  Set handle inheritance to FALSE. */
		NORMAL_PRIORITY_CLASS,	/*  creation/priority flags. */
		NULL,					/*  Use parent's environment block. */
		NULL,					/*  Use parent's starting directory. */
		&startinfo,				/*  Pointer to STARTUPINFO structure. */
		ProcessInfoPtr			/*  Pointer to PROCESS_INFORMATION structure. */
	);
}


/*This function makes the memory dynamic allocation
Parameters: length of the required array of TCHAR; allocated memory identifier
Returns the array of TCHAR
*/
TCHAR* make_alloc(DWORD len, allocated_memory_e allocated_mem)
{
	TCHAR* str_t = NULL; // array of tchar declaration
	str_t = (TCHAR*)malloc((len) * sizeof(TCHAR)); //generates memory allocation
	if (str_t == NULL)
	{ // memory allocation failed
		printf("malloc failed, exiting...\n");
		error_handling();
	}
	activate_allocated_memory(allocated_mem); //call activate allocated memory for turning on the correct bit in bitmask
	return str_t; //return the array of tchar
}


/*This function converts an array of CHAR to array of TCHAR
Parameters: length of the required array of TCHAR; the converted string; allocated memory identifier
Returns the array of TCHAR
*/
TCHAR* convert_char_to_tchar(DWORD len, char* str, allocated_memory_e allocated_mem)
{
	TCHAR* str_t = make_alloc(len, allocated_mem); //makes the memory allocation for str_t
	swprintf(str_t, len, L"%hs", str); //convert array of char to array of tchar
	return(str_t); // return the array of tchar
}

/*This function changes the running process identifier to father process and closes the process and thread handles
Parameters: process information structure
*/
void close_process_handles(PROCESS_INFORMATION procinfo)
{
	CloseHandle(procinfo.hProcess); /* Closing the handle to the process */
	CloseHandle(procinfo.hThread); /* Closing the handle to the main thread of the process */
}

/* This function gets the exit code of the Son process
Parameters: process information structure and the address of exitcode
*/
void get_exit_code(PROCESS_INFORMATION procinfo, LPDWORD *exitcode)
{
	printf("Getting process exit code\n");
	if (!GetExitCodeProcess(procinfo.hProcess, exitcode))
	{//GetExitCodeProcess function failed
		printf("GetExitCodeProcess failed, exitting program...\n");
		close_process_handles(procinfo);
		error_handling();
	}
}

/*This function is responsible for creating the process, its commandline, and handling process time execution
Parameters: command line arguments
*/
void create_process_main(char* argv[])
{
	PROCESS_INFORMATION procinfo;
	DWORD				waitcode;
	DWORD				exitcode = -1;
	BOOL				creation_success;

	//Offset handling
	char offset[MAX_LEN_INT + REQUIRED_SPACES_IN_COMMAND] = { 0 }; //declaration of offset as an array of char
	snprintf(offset, MAX_LEN_INT + REQUIRED_SPACES_IN_COMMAND, " %d ", g_process_counter * KEY_SIZE_BYTES); //convert int to array of char
	TCHAR offset_t[MAX_LEN_INT + REQUIRED_SPACES_IN_COMMAND] = { 0 }; //declaration of offset as an array of TCHAR
	swprintf(offset_t, (MAX_LEN_INT + REQUIRED_SPACES_IN_COMMAND), L"%hs", offset); // convert char array to TCHAR array
	_tprintf(_T("offset_t = %s\n"), offset_t);

	//command handling
	pg_command = make_alloc((_tcslen(pg_key) + MAX_LEN_INT + REQUIRED_SPACES_IN_COMMAND + _tcslen(exe) + _tcslen(pg_plaintext)), COMMAND_ALLOCATED); //makes the memory allocation for command
	_tcscpy(pg_command, exe); //copy exe to command
	_tcscat(pg_command, pg_plaintext); //concatenate plaintext to command
	_tcscat(pg_command, offset_t); //concatenate offset to command
	_tcscat(pg_command, pg_key); //concatenate key to command
	_tprintf(_T("command is %s\n"), pg_command);
	creation_success = create_process_simple(pg_command, &procinfo); //create process with the command
	if (!creation_success)
	{ //process creation failed
		printf("Process Creation Failed!\n");
		error_handling();
	}
	waitcode = WaitForSingleObject( //wait for return value from the son process
		procinfo.hProcess,
		TIMEOUT_IN_MILLISECONDS); /* Waiting 5 secs for the process to end */
	printf("WaitForSingleObject output: ");
	switch (waitcode)
	{
	case WAIT_TIMEOUT: //the process takes too long
		printf("WAIT_TIMEOUT\n"); break;
	case WAIT_OBJECT_0: //the process succeed
		printf("WAIT_OBJECT_0\n"); break;
	case WAIT_FAILED: //WaitForSingleObject failed
		printf("WAIT_FAILED, exitting program...\n");
		close_process_handles(procinfo);
		error_handling();
		break;
	default:
		printf("0x%x\n", waitcode);
	}
	if (waitcode == WAIT_TIMEOUT) /* Process is still alive */
	{//terminate process
		printf("Process was not terminated before timeout!\n"
			"Terminating brutally!\n");
		if (TerminateProcess(procinfo.hProcess,BRUTAL_TERMINATION_CODE))/* Terminating process with an exit code of 0x55 */
		{
			waitcode = WaitForSingleObject( //wait for process to be terminated
			procinfo.hProcess,	TIMEOUT_IN_MILLISECONDS); /* Waiting 5 secs for the process to end */
		}
		printf("process is terminated\n");
		error_handling();
	}
	get_exit_code(procinfo, &exitcode);
	printf("The exit code for the process is 0x%x and it %s\n", exitcode, !exitcode ? "succeed":"failed");
	close_process_handles(procinfo);
	if (exitcode)
	{
		error_handling();
	}
	free(pg_command); //free command memory allocation
	g_allocated_tchar_bitmask &= ~(BIT(COMMAND_ALLOCATED)); //turn off command memory allocated bit
	printf("command memory allocation was free\n");
	g_process_counter++; //increment the number of finished son processes
}

/*This function checks if the program haven't reached to the plaintext EOF
Parameters: plaintext file's name
Returns FALSE if reached plaintext EOF, else TRUE
*/
BOOL havent_reached_eof(char* plaintext_file)
{
	char plaintext_buffer[KEY_SIZE_BYTES + 1] = { 0 }; //plaintext buffer allocation, not in use in this function
	g_h_plaintext_file = file_opening(plaintext_file, GENERIC_READ, OPEN_EXISTING, HANDLE_PLAINTEXT_ACTIVE); // open plaintext file handle
	set_file_offset(g_h_plaintext_file, g_process_counter * KEY_SIZE_BYTES); //set the offset in the plaintext file

	/* get the number of bytes read from file, FALSE means reached EOF*/
	havent_reached_eof_flag = read_from_file(g_h_plaintext_file, plaintext_buffer); 
	printf("reached the end of plaintext file? %s\n", (havent_reached_eof_flag) ? "False" : "True");
	close_active_handles(); //close handles
	return havent_reached_eof_flag; //return flag
}

/* This function gets the arguments needed for creating the son process
Parameters: command line arguments
*/
void get_son_process_arguments(char *argv[])
{
	//exe handling
	_tcscpy(exe, TEXT("Son.exe ")); //exe = "Son.exe "
	_tprintf(_T("Son program name is = %s\n"), exe);

	//plaintext handling - converts the file name to TCHAR array type and makes memory allocation
	pg_plaintext = convert_char_to_tchar((1 + strlen(argv[1])), argv[1], PLAINTEXT_ALLOCATED);
	_tprintf(_T("plaintext = %s\n"), pg_plaintext);

	//key handling - converts the file name to TCHAR array type and makes memory allocation
	pg_key = convert_char_to_tchar((1 + strlen(argv[2])), argv[2], KEY_ALLOCATED);
	_tprintf(_T("key = %s\n"), pg_key);
}

/* father process - get files names from command line and starts new son process when program haven't reached the plaintext EOF
Parameters: command line arguments
*/
void father_process_main(char* argv[])
{
	get_son_process_arguments(argv); //get files names
	while (havent_reached_eof(argv[1])) { //while the program haven't reached the plaintext's eof start a new process
		create_process_main(argv); //call process function
	}
	//reached the plaintext's EOF - finished the program
	free_allocated_bitmask(); /*free allocated memory*/
}

