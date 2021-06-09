/*
 * Utility functions to support the record_sort example
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  
 * If not, see <https://www.gnu.org/licenses/> 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record_sort.h"

int read_file (char *filename, int *size, record_t *records[])
/*
 * Reads a file consisting of records where each record is a
 * text name and an ID number.  To keep it simple, the spaces
 * in the name field are replaced by '_'.
 * 
 * Returns the number of records in the file and an array of
 * records.
 */
{
	int i, n, nrecs = 0;
	FILE *file;
	record_t *temp = NULL;
	
	if ((file = fopen (filename, "r")) == 0)
		return -1;
	
	do
	{
		temp = realloc (temp, (nrecs + 10)*sizeof (record_t));
		
		for (i = nrecs; i < nrecs + 10; i++)
		{
			temp[i].name = malloc (40);
			if ((n = fscanf (file, "%s %d\n", temp[i].name, &temp[i].ID)) != 2)
			{
				free (temp[i].name);
				*size = i - 1;
				*records = temp;
				break;
			}
		}
		nrecs += 10;
	}
	while (n == 2);
	return 0;
}

int write_sorted (int size, record_t records[])
/*
 * Write the sorted file to stdout
 */
{
	int i;
	
	for (i = 0; i < size; i++) // pyadav - correction
		printf ("%-40s %d\n", records[i].name, records[i].ID);
	return 0;
}

void return_records (int size, record_t records[])
/*
 * Free the memory that was allocated for the name fields and the records array
 */
{
	int i;
	
	for (i = 0; i < size; i++)
		free (records[i].name);
	free (records);
}

void sort_name (int size, record_t records[])
/*
 * Sort records in ascending order by name using the shell sort algorithm
 */
{
	int i, j;
	int h = 1;

	do
		h = h*3 + 1;
	while (h <= size);
	
	do
	{
		h /= 3;
		for (i = h; i < size; i++)
		{
			record_t temp = records[i];
			for (j = i;
				j >= h && strcmp (records[j - h].name, temp.name) > 0; j -= h)
				records[j] = records[j - h];
			if (i != j)
				records[j] = temp;
		}
	}
	while (h != 1);	
}

void sort_ID (int size, record_t records[])
/*
 * Sort records in ascending order by ID using the shell sort algorithm
 */
{
	int i, j;
	int h = 1;

	do
		h = h*3 + 1;
	while (h <= size);
	
	do
	{
		h /= 3;
		for (i = h; i < size; i++)
		{
			record_t temp = records[i];
			for (j = i; j >= h && records[j - h].ID > temp.ID; j -= h)
				records[j] = records[j - h];
			if (i != j)
				records[j] = temp;
		}
	}
	while (h != 1);
}
