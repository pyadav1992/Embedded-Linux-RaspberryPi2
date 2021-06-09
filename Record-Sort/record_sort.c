/*
 * record_sort.c
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

#include <stdio.h>
#include <stdlib.h>
#include "record_sort.h"

int main (int argc, char **argv)
{
    int size, sort = 1;
    record_t *records;

    if (read_file (argv[1], &size, &records))
    {
        printf ("Couldn't open file %s\n", argv[1]);
        exit (1);
    }
    
    if (argc > 2)
        sort = atoi (argv[2]);
    
    switch (sort)
    {
        case 1: sort_name (size, records);
        break;
        case 2: sort_ID (size, records);
        break;
        default:
        printf ("Invalid sort argument\n");
        return_records (size, records);
        exit (2);
    }
    
    write_sorted (size, records);
    return_records (size, records);
    
    return 0;
}

