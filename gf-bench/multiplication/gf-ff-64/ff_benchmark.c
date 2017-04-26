/*
    This file is part of the Binary Finite Field Library Version 0.02
    Copyright (C) 2001 Antonio Bellezza

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "ff_2_163.h"
#include <sys/time.h>
#include <stdlib.h>

#define CELLS 1000
#define ITERATIONS 2500000

int main () {
  int i;
  int op;
  //struct timeb tm;
	struct timeval	start, end;
  //long int seconds;
  //long int milliseconds;

  ff_element cell[CELLS];
  ff_element res;
  
  int coeff [ 7 ] = { 10, 5, 3, 3, 3, 1, 1 };


  /* Randomize a[0],...,a[CELLS-1] */
  for (i=0; i<CELLS; i++)
    ff_rand(cell[i]);

  for (op=0; op<2; op++) {
    /* Set timer */
    //ftime (&tm);
    //seconds = tm.time;
    //milliseconds = tm.millitm;
	gettimeofday(&start, NULL); // Get start time

    /* Operation */
    switch(op) {
    case 0:
      /* Product rlcomb */
      for (i=0; i<ITERATIONS*coeff[op]; i++)
	ff_mul_rlcomb( cell[i%CELLS], cell[(i+CELLS/2)%CELLS], res );
      break;
    case 1:
      /* Division */
      for (i=0; i<ITERATIONS*coeff[op]; i++)
	ff_div( cell[i%CELLS], cell[(i+CELLS/2)%CELLS], res );
      break;
    }

    /* Print elapsed time */
    //ftime(&tm);
    //seconds = tm.time - seconds;
    //milliseconds = seconds*1000+(tm.millitm -milliseconds);
	gettimeofday(&end, NULL);
    switch(op) {
    case 0:
      printf("Product:   ");
      break;
    case 1:
      printf("Division:     ");
      break;
    }
    //printf("%.3f seconds elapsed -\t %2.3f microseconds average\n",((double)milliseconds/1000), ((double)milliseconds*1000)/(ITERATIONS*coeff[op]));
printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
  }
  return 0;
}



