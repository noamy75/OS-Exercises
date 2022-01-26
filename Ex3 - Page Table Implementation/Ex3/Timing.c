/*
Description: This module includes the relevant functions for the times linked list, such as sort of the list, add new object and free object
*/

/*Includes*/

#include <HardCodedData.h>
#include <ThreadMain.h>
#include <main.h>
#include <FileHandling.h>
#include <MemAllocation.h>
#include <Timing.h>

/*Functions*/

/* The function frees the remain time objects in the linked list
*/
void free_times_list()
{
	time_object_t* temp;
	while (pg_time_list != NULL)
	{
		temp = pg_time_list;
		pg_time_list = temp->next;
		free(temp);
	}
}

/*This function insert a new time node into a sorted linked list
Parameter: time_object - the time object to be inserted
*/
void sort_time_list(time_object_t* time_object)
{
	if (g_num_elements_in_list == 0) // empty list
	{
		pg_time_list = time_object; // The new object is the head and the only node of the list
		return;
	}
	if (pg_time_list->time > time_object->time) // insert in beggining
	{
		time_object->next = pg_time_list;
		pg_time_list = time_object;
		return;
	}
	time_object_t* prev = pg_time_list, * curr;
	curr = pg_time_list->next;
	while (curr != NULL) // find and insert in the correct place
	{
		if (curr->time < time_object->time)
		{
			prev = curr;
			curr = curr->next;
		}
		else
		{
			break;
		}
	}

	time_object->next = curr;
	prev->next = time_object;
}

/*This function creates a start time object
Parameter: time
Returns TRUE if creation succeed
*/
BOOL create_start_time_object(int time)
{
	time_object_t* time_object = (time_object_t*)malloc(sizeof(time_object_t)); // Memory allocation
	if (time_object == NULL) // allocation failed
	{
		printf("ERROR: create_start_time_object\n");
		return FALSE;
	}
	// Initialize fields
	time_object->time = time;
	time_object->next = NULL;
	sort_time_list(time_object); // Insert into the list
	g_num_elements_in_list++; // counter for the list size 
	return TRUE;
}

/*This function creates a time object
Parameter: time
*/
void create_time_object(int time)
{
	time_object_t* time_object = (time_object_t*)malloc(sizeof(time_object_t)); // Memory allocation
	if (time_object == NULL) // allocation failed
	{
		printf("ERROR: create_time_object\n");
		ExitThread(1);
	}
	// Initialize fields
	time_object->time = time;
	time_object->next = NULL;
	sort_time_list(time_object); // Insert into the list
	g_num_elements_in_list++; // counter for the list size 
}

/*This function updates the finish time of a time object in the linked list and then places it in the correct place
Parameters: old_finish_time - the time node to update, new_finish_time - the new time value
*/
void update_object(int old_finish_time, int new_finish_time)
{
	time_object_t* temp = pg_time_list;
	while (temp->time != old_finish_time) // find the node that needs update
	{
		temp = temp->next;
	}
	temp->time = new_finish_time; // update time value
	int temp_time;
	while (temp->next != NULL)
	{// find correct place to insert
		if (temp->time > temp->next->time)
		{
			temp_time = temp->time;
			temp->time = temp->next->time;
			temp->next->time = temp_time;
			temp = temp->next;
		}
		else
		{
			break;
		}
	}
}

/*This function removes a node from the linked list
Parameter: time - The time node to remove
*/
void free_one_object(int time)
{
	g_num_elements_in_list--; // update number of elemnts in the list
	if (g_num_elements_in_list == 0)
	{
		return;
	}
	time_object_t* curr = pg_time_list;
	while (curr->time != time) // Find the wanted node
	{
		curr = curr->next;
	}
	time_object_t* prev = NULL;
	while (curr->next != NULL) // correct the nodes' values from this point till the end of the list
	{
		prev = curr;
		curr = curr->next;
		prev->time = curr->time;
	}
	if (prev != NULL)
	{
		prev->next = NULL; // set the last node to point to NULL
	}
	free(curr); // free node
}
