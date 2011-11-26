/* Bvim - BVi IMproved, binary analysis framework
 *
 * Copyright 1996-2004 by Gerhard Buergmann <gerhard@puon.at>
 * Copyright 2011 by Anton Kochkov <anton.kochkov@gmail.com>
 *
 * This file is part of Bvim.
 *
 * Bvim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bvim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bvim.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include <inttypes.h>
#define MATH_ARITH 1

typedef unsigned char bit_t;
typedef unsigned char bool_t;

/*
 * If your env does not support inttypes.h replace with this:
 * typedef unsigned long long crc_reg_t;
 * #define REGFMT "llx"
 */
typedef uint64_t crc_reg_t;
#define REGFMT PRIx64

int math__logic(int mode, char* str);
int math__logic_block(int mode, char* str, int block_id);
double math__entropy(int block_id);

/* Arithmetics */
long math__eval(int mode, char *expression);

/* Checksums */
unsigned int crc16(char *addr, int num, unsigned int crc);
unsigned int crc32(char *addr, int num, unsigned int crc);

/* MD hashes */
void math__md4_hash_string(unsigned char *string, long len, char *outputBuffer);
void math__md5_hash_string(unsigned char *string, long len, char *outputBuffer);

/* SHA hashes */
void math__sha1_hash_string(unsigned char *string, long len, char *outputBuffer);
void math__sha256_hash_string(unsigned char *string, long len, char *outputBuffer);
void math__sha512_hash_string(unsigned char *string, long len, char *outputBuffer);

/* RIPEMD hash */
void math__ripemd160_hash_string(unsigned char *string, long len, char *outputBuffer);
