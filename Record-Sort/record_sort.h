/*
 * record_sort.h
 *
 *  Created on: May 2, 2020
 *      Author: Pratik Yadav
 *
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
 
#ifndef RECORD_SORT_H_
#define RECORD_SORT_H_

/*
 * record type to sort on
 */
typedef struct {
	char *name;
	unsigned int ID;
} record_t;

/*
 * Function prototypes
 */
int read_file (char *filename, int *size, record_t *records[]);
int write_sorted (int size, record_t records[]);
void return_records (int size, record_t records[]);
void sort_name (int size, record_t records[]);
void sort_ID (int size, record_t records[]);

#endif /*RECORD_SORT_H_*/
