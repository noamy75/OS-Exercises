#include <HardCodedData.h>

/* Authors:
Ron Lewinsohn: 313291759
Noam Yakar: 318418027

Project: Son

Description: This module gets the command line arguments, encrypts 16 bytes of the plaintext file with the 16 bytes key
and writes the encrypted data to the encrypted file. In each call, the plaintext file is read from an advanced offset
and the encrypted file is written in an advanced offset.
*/

// global variables
extern HANDLE g_h_plaintext_file;
extern HANDLE g_h_key_file;
extern HANDLE g_h_encrypted_file;
extern int g_active_handle_bitmask;


// functions
void son_process(char* argv[]);
void open_all_handles(char* argv[]);
void write_to_file(HANDLE file, char buffer[]);
void handle_xor(char plaintext[], char key[], char encrypted[]);
void encrypt_data(char plaintext[], char key[], char encrypted[]);


/* main function - son process
Parameters: command line arguments
Returns 0 if process succeeds
*/
int main(int argc, char* argv[])
{
    printf("\n\n\n\n\nEntered Son Process\n");
    son_process(argv);
    printf("Finished son process\n\n\n\n\n\n");
    return 0;
}

/*this function opens all handles
Parameters: command line arguments
*/
void open_all_handles(char* argv[])
{
    g_h_plaintext_file = file_opening(argv[1], GENERIC_READ, OPEN_EXISTING, HANDLE_PLAINTEXT_ACTIVE);
    g_h_key_file = file_opening(argv[3], GENERIC_READ, OPEN_EXISTING, HANDLE_KEY_ACTIVE);
    g_h_encrypted_file = file_opening("Encrypted_message.txt", GENERIC_WRITE, OPEN_ALWAYS, HANDLE_ENCRYPTED_ACTIVE);
}

/*son process - encrypt data from plaintext file to encrypted file
Parameters: command line arguments
*/
void son_process(char* argv[])
{
    int offset = atoi(argv[2]); //get offset
    /* declaration of buffers*/
    char plaintext_buffer[KEY_SIZE_BYTES + 1] = { 0 }; 
    char key_buffer[KEY_SIZE_BYTES + 1] = { 0 };
    char encrypted_buffer[KEY_SIZE_BYTES + 1] = { 0 };

    /*open handles*/
    open_all_handles(argv);

    /* read the key from key file*/
    read_from_file(g_h_key_file, key_buffer);
    printf("read from key file %s\n", key_buffer);

    /*sets the offset in the plaintext file and encrypted file*/
    set_file_offset(g_h_plaintext_file, offset);
    set_file_offset(g_h_encrypted_file, offset);

    /* encrypt the data */
    encrypt_data(plaintext_buffer, key_buffer, encrypted_buffer);

    /* close active handles */
    close_active_handles();
}

/* This function writes the buffer to the file
Parameters: file handle, the buffer written to file
*/
void write_to_file(HANDLE file, char buffer[])
{
    if (!WriteFile(file, buffer, KEY_SIZE_BYTES, NULL, NULL))
    {
        printf("Unable to write to file\n");
        error_handling();
    }
}

/* This function does the xor operation and assigns the encrypted data to the encrypted buffer
Parameters: plaintext buffer, key buffer, encrypted buffer
*/
void handle_xor(char plaintext[], char key[], char encrypted[])
{
    for (int idx = 0; idx < KEY_SIZE_BYTES; idx++)
    {
        encrypted[idx] = plaintext[idx] ^ key[idx];
    }
}

/* This function handles the encrypt data routine
Parameters: plaintext buffer, key buffer, encrypted buffer
*/
void encrypt_data(char plaintext[], char key[], char encrypted[])
{
    read_from_file(g_h_plaintext_file, plaintext);
    printf("read from plaintext file %s\n", plaintext);
    handle_xor(plaintext, key, encrypted);
    write_to_file(g_h_encrypted_file, encrypted);
    printf("wrote to encrypted file %s\n", encrypted);
}
