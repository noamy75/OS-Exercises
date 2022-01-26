#pragma once

#define _CRT_SECURE_NO_WARNINGS

/*
Description: This module contains hardcoded data, included libraries and definitions 
*/

// included libraries
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

//definitions


#define KEY_SIZE_BYTES 16
#define MAX_LEN_INT 11 // Number of digits in 2^32 + 1
#define TIMEOUT_IN_MILLISECONDS 5000
#define BRUTAL_TERMINATION_CODE 0x55
#define REQUIRED_SPACES_IN_COMMAND 2
#define BIT(n) 1 << n

//enums

// memory allocation enum
typedef enum {
	PLAINTEXT_ALLOCATED = 0,
	KEY_ALLOCATED,
	COMMAND_ALLOCATED
}allocated_memory_e;

// active handles enum
typedef enum {
	HANDLE_PLAINTEXT_ACTIVE = 0,
	HANDLE_KEY_ACTIVE,
	HANDLE_ENCRYPTED_ACTIVE
}handle_active_e;

