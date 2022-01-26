#include <HardCodedData.h>

/* Description: This module contains global variables, and shared functions for both Son and Father projects
*/

//variables
TCHAR* pg_plaintext = NULL;
TCHAR* pg_key = NULL;
TCHAR* pg_command = NULL;

HANDLE g_h_plaintext_file;
HANDLE g_h_key_file;
HANDLE g_h_encrypted_file;

int g_allocated_tchar_bitmask = 0;
int g_active_handle_bitmask = 0;


/* functions */

/*this function turns on the required bit in the allocated memory bitmask
Parameters: allocated memory identifier
*/
void activate_allocated_memory(allocated_memory_e allocated_mem)
{
    g_allocated_tchar_bitmask |= (BIT(allocated_mem)); //turn on bit
    switch (allocated_mem) //check which variable's memory was allocated
    {
    case PLAINTEXT_ALLOCATED: //plaintext memory was allocated
        printf("plaintext memory allocated\n"); break;
    case KEY_ALLOCATED: //key memory was allocated
        printf("key memory allocated\n"); break;
    case COMMAND_ALLOCATED: //command memory was allocated
        printf("command memory allocated\n"); break;
    }
    printf("memory allocation bitmask = 0x%x\n", g_allocated_tchar_bitmask);
}

/* This function turns on the required bit in the active handle bitmask
Parameters: active handle identifier
*/
void activate_handle(handle_active_e active_handle)
{
    g_active_handle_bitmask |= BIT(active_handle); //turns on the required bit
    switch (active_handle)
    { //prints
    case HANDLE_PLAINTEXT_ACTIVE:
        printf("plaintext handle activated\n"); break;
    case HANDLE_KEY_ACTIVE:
        printf("key handle activated\n"); break;
    case HANDLE_ENCRYPTED_ACTIVE:
        printf("encrypted handle activated\n"); break;
    }
    printf("active_handle_bitmask = 0x%x\n", g_active_handle_bitmask);
}

/* The function turns off all active bits in the active handles bitmaskand closes all active handles
*/
void close_active_handles()
{
    if (g_active_handle_bitmask & BIT(HANDLE_PLAINTEXT_ACTIVE)) {
        CloseHandle(g_h_plaintext_file);
        g_active_handle_bitmask &= ~(BIT(HANDLE_PLAINTEXT_ACTIVE));
        printf("plaintext handle closed\n");
    }
    if (g_active_handle_bitmask & (BIT(HANDLE_KEY_ACTIVE))) {
        CloseHandle(g_h_key_file);
        g_active_handle_bitmask &= ~(BIT(HANDLE_KEY_ACTIVE));
        printf("key handle closed\n");
    }
    if (g_active_handle_bitmask & (BIT(HANDLE_ENCRYPTED_ACTIVE))) {
        CloseHandle(g_h_encrypted_file);
        g_active_handle_bitmask &= ~(BIT(HANDLE_ENCRYPTED_ACTIVE));
        printf("encrypted handle closed\n");
    }
    if (g_active_handle_bitmask == 0) printf("All handles are closed\n");
}

/*This function turns off all active bits in the allocated memory bitmaskand frees the dynamic allocated memory
*/
void free_allocated_bitmask()
{
    if (g_allocated_tchar_bitmask & (BIT(PLAINTEXT_ALLOCATED))) {
        free(pg_plaintext);
        g_allocated_tchar_bitmask &= (~(BIT(PLAINTEXT_ALLOCATED)));
        printf("plaintext memory allocation is free\n");
    }
    if (g_allocated_tchar_bitmask & (BIT(KEY_ALLOCATED))) {
        free(pg_key);
        g_allocated_tchar_bitmask &= (~(BIT(KEY_ALLOCATED)));
        printf("key memory allocation is free\n");
    }
    if (g_allocated_tchar_bitmask & (BIT(COMMAND_ALLOCATED))) {
        free(pg_command);
        g_allocated_tchar_bitmask &= (~(BIT(COMMAND_ALLOCATED)));
        printf("command memory allocation is free\n");
    }
    if (g_allocated_tchar_bitmask == 0) printf("All dynamic memory was free\n");
}

/* this function handles error routine - frees dynamic allocated memory and closes active handles
*/
void error_handling()
{
    printf("Entering error handling\n");
    free_allocated_bitmask();
    close_active_handles();
    printf("exit now\n");
    exit(1);
}

/* The function opens the file with the desired access
Parameters: file name string, the desired access, creation disposition; active handle identifier
Returns the file handle.
*/
HANDLE file_opening(char* filename, DWORD desired_access, DWORD creation_disposition, handle_active_e active_handle)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    hFile = CreateFileA(
        filename,               // file name 
        desired_access,         // reading/writting 
        0,                     // do not share 
        NULL,                  // default security 
        creation_disposition,  // reading - existing file only, writting - open always 
        FILE_ATTRIBUTE_NORMAL, // normal file 
        NULL);                 // no template 
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile failed\n");
        error_handling();
    }
    activate_handle(active_handle);
    return hFile;
}

/* This function sets the file pointer in the required position
Parameters: file handle; the required offset
*/ 
void set_file_offset(HANDLE file, int offset)
{
    DWORD plaintext_ptr = SetFilePointer(file, offset, NULL, FILE_BEGIN);
    if (plaintext_ptr == INVALID_SET_FILE_POINTER)
    {
        DWORD dwError = GetLastError();
        printf("cannot set file pointer, exit...\n");
        error_handling();
    }
}

/*This function reads from file and assigns the data in the buffer
Parameters: file handle, buffer of the read bytes
Returns the number of bytes read from the file
*/
BOOL read_from_file(HANDLE file, char buffer[])
{
    int nByteRead;
    if (!ReadFile(file, (void*)buffer, KEY_SIZE_BYTES, &nByteRead, NULL))
    {
        printf("Cannot read file\n");
        error_handling();
    }
    return nByteRead;
}
