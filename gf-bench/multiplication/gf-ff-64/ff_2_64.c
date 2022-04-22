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
#include "ff_2_64.h"

#define BLOCKS2 ( (FFBITS*2 + sizeof(ff_block)*8-1) / (sizeof(ff_block)*8) )
#define CHARS ( (FFBITS + sizeof(char)*8-1) / (sizeof(char)*8) )

unsigned char squaring_table [ 512 ] = {
0,0,1,0,4,0,5,0,16,0,17,0,20,0,21,0,64,0,65,0,68,0,69,0,80,0,81,0,84,0,85,0,0,1,1,1,4,1,5,1,16,1,17,1,20,1,21,1,64,1,65,1,68,1,69,1,80,1,81,1,84,1,85,1,0,4,1,4,4,4,5,4,16,4,17,4,20,4,21,4,64,4,65,4,68,4,69,4,80,4,81,4,84,4,85,4,0,5,1,5,4,5,5,5,16,5,17,5,20,5,21,5,64,5,65,5,68,5,69,5,80,5,81,5,84,5,85,5,0,16,1,16,4,16,5,16,16,16,17,16,20,16,21,16,64,16,65,16,68,16,69,16,80,16,81,16,84,16,85,16,0,17,1,17,4,17,5,17,16,17,17,17,20,17,21,17,64,17,65,17,68,17,69,17,80,17,81,17,84,17,85,17,0,20,1,20,4,20,5,20,16,20,17,20,20,20,21,20,64,20,65,20,68,20,69,20,80,20,81,20,84,20,85,20,0,21,1,21,4,21,5,21,16,21,17,21,20,21,21,21,64,21,65,21,68,21,69,21,80,21,81,21,84,21,85,21,0,64,1,64,4,64,5,64,16,64,17,64,20,64,21,64,64,64,65,64,68,64,69,64,80,64,81,64,84,64,85,64,0,65,1,65,4,65,5,65,16,65,17,65,20,65,21,65,64,65,65,65,68,65,69,65,80,65,81,65,84,65,85,65,0,68,1,68,4,68,5,68,16,68,17,68,20,68,21,68,64,68,65,68,68,68,69,68,80,68,81,68,84,68,85,68,0,69,1,69,4,69,5,69,16,69,17,69,20,69,21,69,64,69,65,69,68,69,69,69,80,69,81,69,84,69,85,69,0,80,1,80,4,80,5,80,16,80,17,80,20,80,21,80,64,80,65,80,68,80,69,80,80,80,81,80,84,80,85,80,0,81,1,81,4,81,5,81,16,81,17,81,20,81,21,81,64,81,65,81,68,81,69,81,80,81,81,81,84,81,85,81,0,84,1,84,4,84,5,84,16,84,17,84,20,84,21,84,64,84,65,84,68,84,69,84,80,84,81,84,84,84,85,84,0,85,1,85,4,85,5,85,16,85,17,85,20,85,21,85,64,85,65,85,68,85,69,85,80,85,81,85,84,85,85,85
};


ff_element ff_zero = { 0,0 };
ff_element ff_one = { 1,0 };



void reduce ( ff_block* a ) {
  int i;
  ff_block t;
  t = a[BLOCKS2-1];
  a[BLOCKS2-1-1] ^= t>>29 ^ t>>28;
  a[BLOCKS2-1-2] ^= t ^ t<<1 ^ t<<3 ^ t<<4;
  for ( i = 2; i>=2; i-- ) {
    t = a[i];
  a[i-1] ^= t>>31 ^ t>>29 ^ t>>28;
  a[i-2] ^= t ^ t<<1 ^ t<<3 ^ t<<4;
  }
  t = a[BLOCKS-1] & (0xFFFFFFFF-4294967295);
  a[BLOCKS-1-1] ^= t>>31 ^ t>>29 ^ t>>28;
  a[BLOCKS-1-2] ^= t ^ t<<1 ^ t<<3 ^ t<<4;
  a[1] &= 4294967295;
}


void ff_add ( ff_element a, ff_element b, ff_element c ) {
  int i;
  for (i=0; i<BLOCKS; i++)
    c->data[i] = a->data[i] ^ b->data[i];
}

void ff_square ( ff_element a, ff_element b ) {
  int i, j, k;
  unsigned short int to [ sizeof(ff_block)*BLOCKS ];
  ff_block tb;
  for (i=0; i<BLOCKS; i++) {
    tb = a->data[i];
    for (j=0; j<sizeof(ff_block); j++) {
      k = ( tb>>(8*j) ) & 255;
      to[ sizeof(ff_block)*i+j ] = ((unsigned short int*)squaring_table)[ k ];
    }
  }
  reduce((ff_block*)to);
  for (i=0; i<BLOCKS; i++)
    b->data[i] = ((ff_block*)to)[i];
/*
  int i;
  unsigned char to [ sizeof(ff_block)*BLOCKS2 ];
  ff_block* fto = to;
  unsigned char* cptr = a->data;
  for (i=0; i<CHARS; i++) {
    to[ 2*i ] = squaring_table[ 2*cptr[i] ];
    to[ 2*i+1] = squaring_table[ 2*cptr[i] + 1 ];
  }
  for (i=2*CHARS; i<sizeof(ff_block)*BLOCKS2; i++)
    to[i] = 0;
  reduce(to);
  for (i=0; i<BLOCKS; i++)
    b->data[i] = fto[i];
*/
}

void ff_mul_rlcomb ( ff_element a, ff_element b, ff_element c ) {
  int i, k;
  ff_block j;
  
  ff_block tb [ BLOCKS+1 ]; // Will contain b and its shifts
  ff_block t [ BLOCKS2 ]; // Will contain the product polynomial
  ff_block* ap = a->data; // Pointer to beginning of a

  /* Set tb = b */
  for (i=0; i<BLOCKS; i++)
    tb[i] = b->data[i];
  tb[BLOCKS] = 0;

  /* Set t = 0 */
  for (i=0; i<BLOCKS2; i++)
    t[i] = 0;

  /* Right to left comb method */
  for (j=1; j; j<<=1) {
    for (i=0; i<BLOCKS; i++) {
      if (ap[i] & j)
	for (k=0; k<BLOCKS+1; k++)
	  t[ i+k ] ^= tb[k];
    }
    for (i=BLOCKS; i>0; i--)
      tb[i] = tb[i]<<1 ^ tb[i-1]>>31;
    tb[0] = tb[0]<<1;
  }

  /* Reduce */
  reduce (t);

  /* Set c */
  for (i=0; i<BLOCKS; i++)
    c->data[i] = t[i];
}

void ff_mul_lrcomb ( ff_element a, ff_element b, ff_element c ) {
  int i, k;
  ff_block j;
  
  ff_block t [ BLOCKS2 ]; // Will contain the product polynomial
  ff_block* ap = a->data; // Pointer to beginning of a
  ff_block* bp = b->data; // Pointer to beginning of a

  /* Set t = 0 */
  for (i=0; i<BLOCKS2; i++)
    t[i] = 0;

  /* Left to right comb method */
  for (j=(ff_block)1<<31; j; j>>=1) {
    for (i=0; i<BLOCKS; i++) {
      if (ap[i] & j)
	for (k=0; k<BLOCKS; k++)
	  t[ i+k ] ^= bp[k];
    }
    if (j!=1) {
      for (i=BLOCKS2-1; i>0; i--)
	t[i] = t[i]<<1 ^ t[i-1]>>31;
      t[0] = t[0]<<1;
    }
  }

  /* Reduce */
  reduce (t);

  /* Set c */
  for (i=0; i<BLOCKS; i++)
    c->data[i] = t[i];
}

void ff_mul_lrcomb_w4 ( ff_element a, ff_element b, ff_element c ) {
  int i, j, k;

  ff_block base [ BLOCKS*16 ];
  
  ff_block t [ BLOCKS2 ]; // Will contain the product polynomial
  ff_block* ap = a->data; // Pointer to beginning of a
  ff_block* bp = b->data; // Pointer to beginning of b

  /* Set base */

  /* 0 b */
  for (i=0; i<BLOCKS; i++)
    base[ i ] = 0;

  /* 1 b */
  for (i=0; i<BLOCKS; i++)
    base[ BLOCKS+i ] = bp[i];

  /* 2 b */
  base[ 2*BLOCKS ] = base[1*BLOCKS]<<1;
  for (i=1; i<BLOCKS; i++)
    base[ 2*BLOCKS+i ] = base[1*BLOCKS+i]<<1 ^ base[1*BLOCKS+i-1]>>31;
  if ( base[ 1*BLOCKS+BLOCKS-1 ] & 2147483648 ) {
    base[2*BLOCKS+BLOCKS-1] &= 4294967295;
    base[2*BLOCKS+0] ^= 27;
  }

  /* 4 b */
  base[ 4*BLOCKS ] = base[2*BLOCKS]<<1;
  for (i=1; i<BLOCKS; i++)
    base[ 4*BLOCKS+i ] = base[2*BLOCKS+i]<<1 ^ base[2*BLOCKS+i-1]>>31;
  if ( base[ 2*BLOCKS+BLOCKS-1 ] & 2147483648 ) {
    base[4*BLOCKS+BLOCKS-1] &= 4294967295;
    base[4*BLOCKS+0] ^= 27;
  }

  /* 8 b */
  base[ 8*BLOCKS ] = base[4*BLOCKS]<<1;
  for (i=1; i<BLOCKS; i++)
    base[ 8*BLOCKS+i ] = base[4*BLOCKS+i]<<1 ^ base[4*BLOCKS+i-1]>>31;
  if ( base[ 4*BLOCKS+BLOCKS-1 ] & 2147483648 ) {
    base[8*BLOCKS+BLOCKS-1] &= 4294967295;
    base[8*BLOCKS+0] ^= 27;
  }

  /* 3 b */
  for (i=0; i<BLOCKS; i++)
    base[ 3*BLOCKS+i ] = base[ 2*BLOCKS+i ] ^ base[ 1*BLOCKS+i ];

  /* 5 b */
  for (i=0; i<BLOCKS; i++)
    base[ 5*BLOCKS+i ] = base[ 4*BLOCKS+i ] ^ base[ 1*BLOCKS+i ];

  /* 6 b */
  for (i=0; i<BLOCKS; i++)
    base[ 6*BLOCKS+i ] = base[ 4*BLOCKS+i ] ^ base[ 2*BLOCKS+i ];

  /* 7 b */
  for (i=0; i<BLOCKS; i++)
    base[ 7*BLOCKS+i ] = base[ 4*BLOCKS+i ] ^ base[ 3*BLOCKS+i ];

  /* 9 b */
  for (i=0; i<BLOCKS; i++)
    base[ 9*BLOCKS+i ] = base[ 8*BLOCKS+i ] ^ base[ 1*BLOCKS+i ];

  /* 10 b */
  for (i=0; i<BLOCKS; i++)
    base[ 10*BLOCKS+i ] = base[ 8*BLOCKS+i ] ^ base[ 2*BLOCKS+i ];

  /* 11 b */
  for (i=0; i<BLOCKS; i++)
    base[ 11*BLOCKS+i ] = base[ 8*BLOCKS+i ] ^ base[ 3*BLOCKS+i ];

  /* 12 b */
  for (i=0; i<BLOCKS; i++)
    base[ 12*BLOCKS+i ] = base[ 8*BLOCKS+i ] ^ base[ 4*BLOCKS+i ];

  /* 13 b */
  for (i=0; i<BLOCKS; i++)
    base[ 13*BLOCKS+i ] = base[ 8*BLOCKS+i ] ^ base[ 5*BLOCKS+i ];

  /* 14 b */
  for (i=0; i<BLOCKS; i++)
    base[ 14*BLOCKS+i ] = base[ 8*BLOCKS+i ] ^ base[ 6*BLOCKS+i ];

  /* 15 b */
  for (i=0; i<BLOCKS; i++)
    base[ 15*BLOCKS+i ] = base[ 8*BLOCKS+i ] ^ base[ 7*BLOCKS+i ];


  /* Set t = 0 */
  for (i=0; i<BLOCKS2; i++)
    t[i] = 0;

  /* Left to right comb method */
  for (j=32-4; j>=0; j-=4) {
    for (i=0; i<BLOCKS; i++) {
      bp = &base[((ap[i]>>j)&(16-1))*BLOCKS];
      for (k=0; k<BLOCKS; k++)
	t[ i+k ] ^= bp[k];
    }
    if (j!=0) {
      for (i=BLOCKS2-1; i>0; i--)
	t[i] = t[i]<<4 ^ t[i-1]>>(32-4);
      t[0] = t[0]<<4;
    }
  }

  /* Reduce */
  reduce (t);

  /* Set c */
  for (i=0; i<BLOCKS; i++)
    c->data[i] = t[i];
}


void ff_inv ( ff_element a, ff_element r ) {
  int i,j;
  int du, dv, dtmp; // Degrees of temporary polynomials
  int jblocks, jbits, jbitscomp; // indices of shift
  /*
      ba = u  (mod f)
      ca = v  (mod f)
      Starting from: b = 1
                     u = a
		     c = 0
		     v = f
  */
  ff_block b1[BLOCKS];
  ff_block b2[BLOCKS];
  ff_block b3[BLOCKS];
  ff_block b4[BLOCKS];
  ff_block* b = b1;
  ff_block* c = b2;
  ff_block* u = b3;
  ff_block* v = b4; // It may need 1 more block for the case 2^BITS > maximum
  ff_block* btmp;

  //  ff_block ptr;

  for (i=0; i<BLOCKS; i++) {
    u[i] = a->data[i];
  //    v[i] = 0;
    b[i] = 0;
    c[i] = 0;
  }
  b[0] = 1;
  v[0] = 27;
  v[1] = 0;


  for (i=BLOCKS-1; u[i]==0; i--);

  /* binary search for highest non-zero bit
  ptr = (1<<16);
  for (j=8; j!=0; j>>=1) {
    if (u[i]<ptr)
      ptr >>= j;
    else
      ptr <<=j;
  };
  */

  for (j=31; u[i]<((ff_block)1<<j); j--);
  du = 32*i + j;
  dv = FFBITS;

  while (du>0) { // Better: while (du)
    jbits = du - dv;
    if (jbits<0) {
      // Swap u and v
      btmp = u;
      u = v;
      v = btmp;
      // Swap b and c
      btmp = b;
      b = c;
      c = btmp;
      // Swap du and dv
      dtmp = du;
      du = dv;
      dv = dtmp;
      // j = -j
      jbits = -jbits;
    }
    jblocks = jbits>>5; // jbits/5
    jbits = jbits & (32-1); // jbits % 5
    if (jbits) {
      jbitscomp = 32-jbits;
      // for (i = du/32; i>jblocks; i--) { // is valid for u
      for (i = BLOCKS-1; i>jblocks; i--) { // To optimize
	u[i] ^= (v[i-jblocks]<<jbits) ^ (v[i-jblocks-1]>>jbitscomp);
	b[i] ^= (c[i-jblocks]<<jbits) ^ (c[i-jblocks-1]>>jbitscomp);
      }
      u[jblocks] ^= (v[0]<<jbits);
      b[jblocks] ^= (c[0]<<jbits);
    } else {
      for (i = BLOCKS-1; i>=jblocks; i--) {
	u[i] ^= v[i-jblocks];
	b[i] ^= c[i-jblocks];
      }
    }
    du--;
    while ( (u[du>>5] & ((ff_block)1<<(du & 31))) == 0 )
      du--;
  }
  for (i=0; i<BLOCKS; i++)
    r->data[i] = b[i];
}

void ff_div ( ff_element a, ff_element b, ff_element c ) {
  ff_inv(b,c);
  ff_mul(a,c,c);
}

void ff_copy ( ff_element a, ff_element b ) {
  int i;
  for (i=0; i<BLOCKS; i++) {
    b->data[i] = a->data[i];
  }
}

void ff_rand ( ff_element a ) {
  int i;
  for (i=0; i<BLOCKS; i++) {
    a->data[i] = (ff_block) rand();
  }
  a->data[BLOCKS-1] &= 4294967295;
}

void ff_set ( ff_element a, ff_block b [BLOCKS] ) {
  int i;
  for (i=0; i<BLOCKS; i++) {
    a->data[i] = b[i];
  }
}


void ff_read ( ff_element a ) {
  scanf("%x %x", a->data+1, a->data+0);

  assert( !(a->data[BLOCKS-1] & !4294967295) );
  a->data[BLOCKS-1] &= 4294967295;
}

void ff_sread ( const char * s, ff_element a ) {
  sscanf(s, "%x %x", a->data+1, a->data+0);

  assert( !(a->data[BLOCKS-1] & !4294967295) );
  a->data[BLOCKS-1] &= 4294967295;
}

void ff_print ( ff_element a ) {
  printf("[%x %x]", a->data[1], a->data[0]);

  fflush(0);
}


int ff_eq ( ff_element a, ff_element b ) {
  int res = 1;
  int i;
  for (i=0; i<BLOCKS && res; i++)
    res = res && ( a->data[i] == b->data[i] );
  return res;
}

