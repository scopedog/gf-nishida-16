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

#define CHECKN 100000

void ff_set_bit( ff_element a, int b ) {
  int k;
  for (k=0; k<BLOCKS; k++) // Set a = 2^i
    if (b/32==k)
      a->data[k] = (ff_block)1<<(b%32);
    else
      a->data[k] = 0;
}

/* If a!=b print s with args c, d ( or just c if d==NULL ) */
void errcheck( ff_element a, ff_element b, const char* s, ff_element c, ff_element d ) {
  if (!ff_eq(a,b)) {
    printf("Error: %s on ", s);
    ff_print(c);
    if (d) {
      printf(" x ");
      ff_print(d);
    };
    printf("\n");
  }
}

int main () {
  int i, j, k;
  ff_element a, b, c, d, e, f;
  ff_block zero[BLOCKS], one[BLOCKS];

  
  printf("Checking product:\n");
  printf("\tbitwise:\n");
  for (i=0; i<163; i++) {
    ff_set_bit(a,i);    
    for (j=0; j<163; j++) {
      ff_set_bit(b,j);
      ff_mul_rlcomb(a,b,c);
      ff_mul_lrcomb(a,b,d);
      ff_mul_lrcomb_w4(a,b,e);
      errcheck(c,d,"mul_rlcomb and mul_lrcomb differ",a,b);
      errcheck(c,e,"mul_rlcomb and mul_lrcomb_w4 differ",a,b);
      errcheck(d,e,"mul_lrcomb and mul_lrcomb_w4 differ",a,b);
    }
  }
  printf("\trandom:\n");
  for (i=0; i<CHECKN; i++) {
    ff_rand(a);
    ff_rand(b);
    ff_mul_rlcomb(a,b,c);
    ff_mul_lrcomb(a,b,d);
    ff_mul_lrcomb_w4(a,b,e);
    errcheck(c,d,"mul_rlcomb and mul_lrcomb differ",a,b);
    errcheck(c,e,"mul_rlcomb and mul_lrcomb_w4 differ",a,b);
    errcheck(d,e,"mul_lrcomb and mul_lrcomb_w4 differ",a,b);
  }

  
  printf("Checking squaring:\n");
  printf("\tbitwise:\n");
  for (i=0; i<163; i++) {
    ff_set_bit(a,i);
    ff_mul(a,a,b);
    ff_square(a,c);
    errcheck(b,c,"ff_square",a,0);
  }
  printf("\trandom:\n");
  for (i=0; i<CHECKN; i++) {
    ff_rand(a);
    ff_mul(a,a,b);
    ff_square(a,c);
    errcheck(b,c,"ff_square",a,0);
  }


  printf("Checking inversion:\n");
  printf("\tbitwise:\n");
  for (i=0; i<163; i++) {
    ff_set_bit(a,i);
    ff_inv(a,b);
    ff_mul(a,b,c);
    errcheck(c,ff_one,"ff_inv wrong",a,0);
  }
  printf("\trandom:\n");
  for (i=0; i<CHECKN; i++) {
    do {
      ff_rand(a);
    } while (ff_eq(a,ff_zero));
    ff_inv(a,b);
    ff_mul(a,b,c);
    errcheck(c,ff_one,"ff_inv wrong",a,0);
  }


  printf("Checking division:\n");
  printf("\tbitwise:\n");
  for (i=0; i<163; i++) {
    ff_set_bit(a,i);
    for (j=0; j<163; j++) {
      ff_set_bit(b,j);
      ff_div(a,b,c);
      ff_mul(b,c,d);
      errcheck(d,a,"division",a,b);
    }
  }
  printf("\trandom:\n");
  for (i=0; i<CHECKN; i++) {
    ff_rand(a);
    do {
      ff_rand(b);
    } while (ff_eq(b,ff_zero));
    ff_div(a,b,c);
    ff_mul(b,c,d);
    errcheck(d,a,"division",a,b);
  } 
}

