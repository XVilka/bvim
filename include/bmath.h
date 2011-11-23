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
