#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

/*
Description: This module contains hardcoded data, included libraries and definitions
*/

// a define that must be before including <windows.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Includes

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <SharedFunctions.h>
#pragma comment(lib, "ws2_32.lib")

// Defines

#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define FINAL_TRANSMISSION "final transmission"
#define MAX_USERNAME_LENGTH 21
#define MAX_MSG_TYPE_LENGTH 21
#define MAX_INPUT_LENGTH 256
#define MAX_FILE_NAME_LENGTH 36
#define FIFTEEN_SECONDS 15000
#define TEN_MINUTES 600000
#define THIRTY_SECONDS 30000