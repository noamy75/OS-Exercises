/*
Description: This module includes the thread's main function and other functions relevant to its operation such as
file handling, handles opening, mutex locking/releasing and grades calculations. 
*/

/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>

/*Functions */


/* Thread main function
* Parameters: array of handles_t structs
* Returns 0 if Thread finished his process
*/
int thread_main(LPVOID lpParam)
{
	printf("Thread %d is starting now!!\n", g_thread_idx);

	//lock mutex
	lock_mutex();
	printf("Mutex is locked for num_school_idx = %d\n", g_thread_idx);

	//Enter critical area - each thread receives it's school index
	int num_school_idx = g_thread_idx++;
	printf("Releasing Mutex\n");

	// Release mutex
	release_mutex(num_school_idx);

	// Prepare the array of handles_t structs
	handles_t* p_struct_arr = NULL;
	if (NULL == lpParam)
	{ // If the thread parameter is NULL
		printf("ERROR: Thread parameters are NULL\n");
		printf("THREAD %d CRASHED\n", num_school_idx);
		ExitThread(1);
	}
	p_struct_arr = (handles_t*)lpParam;

	// Open all file handles
	open_handles(num_school_idx, p_struct_arr);

	// Average calculation
	avarage_calculation(num_school_idx, p_struct_arr);

	// Thread finished
	printf("Thread %d finished\n", num_school_idx);
	return 0;
}

/* The function locks the mutex handle
*/
void lock_mutex()
{
	int wait_code = WaitForSingleObject(g_school_idx_mutex_handle, TIMEOUT_IN_MILLISECONDS);
	if (WAIT_OBJECT_0 != wait_code)
	{ //If locking procedure failed
		printf("ERROR: Error when waiting for mutex\n");
		printf("THREAD %d CRASHED\n", g_thread_idx);
		ExitThread(1);
	}
}

/* The function releases the mutex handle
*/
void release_mutex(int num_school_idx)
{
	BOOL ret_val = ReleaseMutex(g_school_idx_mutex_handle);
	if (FALSE == ret_val)
	{ //if the mutex releasing procedure failed
		printf("ERROR: Error when releasing mutex\n");
		printf("THREAD %d CRASHED\n", num_school_idx);
		ExitThread(1);
	}
}

/* This function turns on the required bit in the active handle bitmask
Parameters: active handle identifier
*/
void activate_handles(handles_t* p_struct_arr, int num_school_idx, handles_enum active_handle)
{
	p_struct_arr[num_school_idx].active_handles_bitmask |= BIT(active_handle);
	switch (active_handle)
	{ //prints
	case REAL:
		printf("real handle activated\n"); break;
	case HUMAN:
		printf("human handle activated\n"); break;
	case ENG:
		printf("eng handle activated\n"); break;
	case EVAL:
		printf("eval handle activated\n"); break;
	case RESULTS:
		printf("results handle activated\n"); break;
	}
	printf("active handles bitmask = 0x%x\n", p_struct_arr[num_school_idx].active_handles_bitmask);
}

/* The function opens the file with the desired access
Parameters: file name string, the desired access, creation disposition; active handle identifier
Returns the file handle.
*/
HANDLE file_opening(handles_t* p_struct_arr, int num_school_idx,
	char* filename, DWORD desired_access, DWORD creation_disposition, handles_enum active_handle)
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
		printf("ERROR: CreateFile failed\n");
		printf("THREAD %d CRASHED\n", num_school_idx);
		ExitThread(1);
	}
	printf("File %s created\n", filename);
	activate_handles(p_struct_arr, num_school_idx, active_handle);
	return hFile;
}

/* The function creates the file path string
* Parameters: school_idx string, directory name string and the designated file path string
*/
void create_file_name(char* num_school_idx_char, char* directory, char* filename)
{
	strcpy(filename, directory); //copy the directory name to the designated file path string
	strcat(filename, num_school_idx_char); //copy the school index to the designated file path string
	strcat(filename, (char*)".txt"); // copy ".txt" to the designated file path string
}

/* The function opens all file handles.
* Parameters: num_school_idx and the array of handles_t structs
*/
void open_handles(int num_school_idx, handles_t* p_struct_arr)
{
	char filename[MAX_FILE_NAME_LEN] = { 0 };
	//convert the num_school_idx from type int to array of char
	char num_school_idx_char[MAX_LEN_INT] = { 0 };
	snprintf(num_school_idx_char, MAX_LEN_INT, "%d", num_school_idx);

	/* open real handle */
	create_file_name(num_school_idx_char, (char*)"Real/Real", filename); //creates the file full path
	printf("the file name is %s\n", filename);
	//Opens the real handle for read
	p_struct_arr[num_school_idx].h_real = file_opening(p_struct_arr, num_school_idx, filename, GENERIC_READ, OPEN_EXISTING, (handles_enum)REAL);

	/* open human handle */
	create_file_name(num_school_idx_char, (char*)"Human/Human", filename);//creates the file full path
	printf("the file name is %s\n", filename);
	//Opens the human handle for read
	p_struct_arr[num_school_idx].h_human = file_opening(p_struct_arr, num_school_idx, filename, GENERIC_READ, OPEN_EXISTING, (handles_enum)HUMAN);

	/* open eng handle */
	create_file_name(num_school_idx_char, (char*)"Eng/Eng", filename);//creates the file full path
	printf("the file name is %s\n", filename);
	//Opens the eng handle for read
	p_struct_arr[num_school_idx].h_eng = file_opening(p_struct_arr, num_school_idx, filename, GENERIC_READ, OPEN_EXISTING, (handles_enum)ENG);

	/* open eval handle */
	create_file_name(num_school_idx_char, (char*)"Eval/Eval", filename);//creates the file full path
	printf("the file name is %s\n", filename);
	//Opens the eval handle for read
	p_struct_arr[num_school_idx].h_eval = file_opening(p_struct_arr, num_school_idx, filename, GENERIC_READ, OPEN_EXISTING, (handles_enum)EVAL);

	/* open results handle */
	create_file_name(num_school_idx_char, (char*)"Results/Results", filename);//creates the file full path
	printf("the file name is %s\n", filename);
	//Opens the results handle for write
	p_struct_arr[num_school_idx].h_results = file_opening(p_struct_arr, num_school_idx, filename, GENERIC_WRITE, OPEN_ALWAYS, (handles_enum)RESULTS);
}


/* The function uses for reading the grade from a file and converting it to int
Parameters: file handle, point to the read grade, num school index and a handle identifier number
Returns True if haven't reached the end of file.
*/
BOOL read_from_file(HANDLE h_file, int* p_read_grade, int num_school_idx, handles_enum handle)
{
	char grade_char[2] = { 0 }; int nByteRead;
	char grade_str[MAX_GRADE_LEN] = { 0 };
	do
	{// Read grade from the file byte by byte
		if (!ReadFile(h_file, (void*)grade_char, ONE_BYTE, &nByteRead, NULL))
		{ //If file reading failed.
			printf("ERROR: Cannot read file on Thread %d, handle is %d\n", num_school_idx, handle);
			printf("THREAD %d CRASHED\n", num_school_idx);
			ExitThread(1);
		}
		if (nByteRead == 0) return FALSE; //Read nothing - reached the end of file
		strcat(grade_str, grade_char); // concatenate the read byte to the temporary grade string
	} while (grade_char[0] != '\n'); // do this loop until reached '\n' -> read grade finished
	*p_read_grade = atoi(grade_str); //convert the grade string to an integer
	return TRUE;
}

/* The function writes the avarage grade into the results file
* Parameters: results file handle, average grade
*/
void write_to_file(HANDLE h_results, int average_grade, int num_school_idx)
{
	char average_grade_char[MAX_GRADE_LEN] = { 0 };
	snprintf(average_grade_char, MAX_GRADE_LEN, "%d", average_grade); //convert int to array of char
	strcat(average_grade_char, "\r\n"); // concatenate to avarage grade char '\r\n'
	printf("average grade in char is %s\n", average_grade_char);
	if (!WriteFile(h_results, average_grade_char, strlen(average_grade_char), NULL, NULL))
	{// if writting failed
		printf("ERROR: Unable to write to file\n");
		printf("THREAD %d CRASHED\n", num_school_idx);
		ExitThread(1);
	}
}


/* The function reads grades, calculates the average grade of every student and write it to the results file
* Parameters: num school idx, array of handles_t structs
*/
void avarage_calculation(int num_school_idx, handles_t* p_struct_arr)
{
	int real_grade, human_grade, eng_grade, eval_grade, average_grade;
	//While we haven't finished to calculate the average grade of all student in num_school_idx 
	while (read_from_file(p_struct_arr[num_school_idx].h_real, &real_grade, num_school_idx, (handles_enum)REAL))
	{
		// Read the student's real grade
		printf("thread: %d, real grade is %d\n", num_school_idx, real_grade);
		read_from_file(p_struct_arr[num_school_idx].h_human, &human_grade, num_school_idx, (handles_enum)HUMAN);
		// Read the student's human grade
		printf("thread: %d, human grade is %d\n", num_school_idx, human_grade);
		read_from_file(p_struct_arr[num_school_idx].h_eng, &eng_grade, num_school_idx, (handles_enum)ENG);
		// Read the student's eng grade
		printf("thread: %d, eng grade is %d\n", num_school_idx, eng_grade);
		read_from_file(p_struct_arr[num_school_idx].h_eval, &eval_grade, num_school_idx, (handles_enum)EVAL);
		printf("thread: %d, eval grade is %d\n", num_school_idx, eval_grade);
		// Read the student's eval grade
		average_grade = (real_grade * g_weight_real + human_grade * g_weight_human + eng_grade * g_weight_eng + eval_grade * g_weight_eval) / 100;
		// Calculate the student's average grade
		printf("thread: %d, avarage grade is %d\n", num_school_idx, average_grade);
		write_to_file(p_struct_arr[num_school_idx].h_results, average_grade, num_school_idx);
		// Write the student's average grade to the results file
	}
}
