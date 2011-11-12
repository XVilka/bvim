#include "bvi.h"
#include "blocks.h"
#include "set.h"
#include "ui.h"
#include "math.h"

extern struct MARKERS_ markers[MARK_COUNT];

/* Math expressions */

char* bvi_substr(const char* str, size_t begin, size_t len) 
{
	if (str == 0 || strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+len))
		return 0;
	return strndup(str + begin, len);
}

// TODO: Implement recognition of hexadecimal, binary, octary, decimal arithmetics.
// TODO: Implement simple operations: +,-,*,/,mod,>>,<<,^,|,&.
long math__eval(int mode, char* expression) {
	char multiplicator;
	long i = 1;

	if (mode == MATH_ARITH) {
		if (!strcmp(bvi_substr(expression, 0, 1), "+")) {
			multiplicator = expression[strlen(expression) - 1];
			switch (multiplicator) {
				case 'K':
					i = 1000;
					break;
				case 'M':
					i = 1000000;
					break;
				default:
					i = 1;
					break;
			}
			return atoi(substr(expression, 1, strlen(expression) -1 )) * i;
		}
		else if (!strcmp(bvi_substr(expression, 0, 2), "0x")) {
			
		}
		else {
			return atoi(expression);
		}
	}
	return 0;
}

/* Logic functions */

int math__logic(int mode, char* str)
{
	int a, b;
	int value;
	size_t n;
	char *err_str = "Invalid value@for bit manipulation";

	if (mode == LSHIFT || mode == RSHIFT || mode == LROTATE
	    || mode == RROTATE) {
		value = atoi(str);
		if (value < 1 || value > 8) {
			ui__ErrorMsg(err_str);
			return 1;
		}
	} else {
		if (strlen(str) == 8) {
			value = strtol(str, NULL, 2);
			for (n = 0; n < 8; n++) {
				if (str[n] != '0' && str[n] != '1') {
					value = -1;
					break;
				}
			}
		} else if (str[0] == 'b' || str[0] == 'B') {
			value = strtol(str + 1, NULL, 2);
		} else if (str[0] == '0') {
			value = strtol(str, NULL, 16);
			for (n = 0; n < strlen(str); n++) {
				if (!isxdigit(str[n])) {
					value = -1;
					break;
				}
			}
		} else {
			value = atoi(str);
		}
		if (value < 0 || value > 255) {
			ui__ErrorMsg(err_str);
			return 1;
		}
	}
	if ((undo_count =
	     alloc_buf((off_t) (end_addr - start_addr + 1), &undo_buf))) {
		memcpy(undo_buf, start_addr, undo_count);
	}
	undo_start = start_addr;
	edits = U_EDIT;
	while (start_addr <= end_addr) {
		a = *start_addr;
		a &= 0xff;
		switch (mode) {
		case LSHIFT:
			a <<= value;
			break;
		case RSHIFT:
			a >>= value;
			break;
		case LROTATE:
			a <<= value;
			b = a >> 8;
			a |= b;
			break;
		case RROTATE:
			b = a << 8;
			a |= b;
			a >>= value;
			/*
			   b = a << (8 - value);
			   a >>= value; 
			   a |= b;
			 */
			break;
		case AND:
			a &= value;
			break;
		case OR:
			a |= value;
			break;
		case XOR:
		case NOT:
			a ^= value;
			break;
		case NEG:
			a ^= value;
			a++;	/* Is this true */
			break;
		}
		*start_addr++ = (char)(a & 0xff);
	}
	ui__Screen_Repaint();
	return (0);
}

int math__logic_block(int mode, char* str, int block_id)
{
	int a, b;
	int value;
	size_t n;
	char *err_str = "Invalid value@for bit manipulation";
	struct block_item *tmp_blk;
	
	tmp_blk = blocks__GetByID(block_id);
	if ((tmp_blk == NULL) | ((tmp_blk != NULL) & (tmp_blk->pos_start < tmp_blk->pos_end))) {
		ui__ErrorMsg("Invalid block for bit manupulation!");
		return 1;
	}

	if (mode == LSHIFT || mode == RSHIFT || mode == LROTATE
	    || mode == RROTATE) {
		value = atoi(str);
		if (value < 1 || value > 8) {
			ui__ErrorMsg(err_str);
			return 1;
		}
	} else {
		if (strlen(str) == 8) {
			value = strtol(str, NULL, 2);
			for (n = 0; n < 8; n++) {
				if (str[n] != '0' && str[n] != '1') {
					value = -1;
					break;
				}
			}
		} else if (str[0] == 'b' || str[0] == 'B') {
			value = strtol(str + 1, NULL, 2);
		} else if (str[0] == '0') {
			value = strtol(str, NULL, 16);
			for (n = 0; n < strlen(str); n++) {
				if (!isxdigit(str[n])) {
					value = -1;
					break;
				}
			}
		} else {
			value = atoi(str);
		}
		if (value < 0 || value > 255) {
			ui__ErrorMsg(err_str);
			return 1;
		}
	}
	if ((undo_count =
	     alloc_buf((off_t) (tmp_blk->pos_end -
				tmp_blk->pos_start + 1),
		       &undo_buf))) {
		memcpy(undo_buf,
		       start_addr + tmp_blk->pos_start,
		       undo_count);
	}
	undo_start = start_addr + tmp_blk->pos_start;
	edits = U_EDIT;
	start_addr = start_addr + tmp_blk->pos_start;
	end_addr =
	    start_addr + tmp_blk->pos_end - tmp_blk->pos_start;
	while (start_addr <= end_addr) {
		a = *start_addr;
		a &= 0xff;
		switch (mode) {
		case LSHIFT:
			a <<= value;
			break;
		case RSHIFT:
			a >>= value;
			break;
		case LROTATE:
			a <<= value;
			b = a >> 8;
			a |= b;
			break;
		case RROTATE:
			b = a << 8;
			a |= b;
			a >>= value;
			/*
			   b = a << (8 - value);
			   a >>= value; 
			   a |= b;
			 */
			break;
		case AND:
			a &= value;
			break;
		case OR:
			a |= value;
			break;
		case XOR:
		case NOT:
			a ^= value;
			break;
		case NEG:
			a ^= value;
			a++;	/* Is this true */
			break;
		}
		*start_addr++ = (char)(a & 0xff);
	}
	ui__Screen_Repaint();
	return (0);
}

/* ------------------------------------------------------------------------------------
 *                           Cyclic redundancy check
 * ------------------------------------------------------------------------------------
 */
static crc_reg_t strtoreg(const char *str)
{
    crc_reg_t v;
    if (str[0] == '0' && str[1] == 'x') {
        sscanf(&str[2], "%" REGFMT, &v);
    } else {
        sscanf(str, "%" REGFMT, &v);
    }
    return v;
}

/*
 * Implementation of a shift register with parameterized length
 */

struct bit_reg {
    unsigned int bits;
    crc_reg_t top_bit;
    crc_reg_t mask;
    crc_reg_t reg;
};

static void bit_reg_init(struct bit_reg *reg, unsigned int bits)
{
    reg->bits = bits;
    reg->top_bit = (crc_reg_t)1 << (bits - 1);
    reg->mask = reg->top_bit | (reg->top_bit - 1);
    reg->reg = 0;
}

static bit_t bit_reg_shift_left(struct bit_reg *reg, bit_t in)
{
    bit_t out = !!(reg->reg & reg->top_bit);
    reg->reg <<= 1;
    reg->reg &= reg->mask;
    reg->reg |= !!in;
    return out;
}

static bit_t bit_reg_shift_right(struct bit_reg *reg, bit_t in)
{
    bit_t out = (bit_t)(reg->reg & 1);
    reg->reg >>= 1;
    if (in) {
        reg->reg |= reg->top_bit;
    }
    return out;
}

static void bit_reg_reverse(struct bit_reg *reg)
{
    struct bit_reg tmp_reg = *reg;
    unsigned int i;
    for (i = 0; i < reg->bits; i++) {
        bit_reg_shift_right(reg, bit_reg_shift_left(&tmp_reg, 0));
    }
}

static void bit_reg_set(struct bit_reg *reg, crc_reg_t value)
{
    reg->reg = value & reg->mask;
}

static crc_reg_t bit_reg_get(struct bit_reg *reg)
{
    return reg->reg;
}

static void bit_reg_xor(struct bit_reg *reg, crc_reg_t value)
{
    reg->reg = (reg->reg ^ value) & reg->mask;
}

struct crc_param {
    unsigned int bits;
    crc_reg_t poly;
    crc_reg_t init;
    crc_reg_t eor;
    bool_t reverse;
    bool_t non_direct;
};

//static crc_reg_t crc_universal(data)

/*
 * General CRC calculation, really slow but handles all variants
 * Note: len is given in bits
 */
static crc_reg_t crc_calc(const unsigned char *data,
                          size_t len,
                          const struct crc_param *param)
{
    struct bit_reg reg;
    struct bit_reg poly;
    struct bit_reg msg;
    unsigned int i;

    bit_reg_init(&reg, param->bits);
    bit_reg_init(&poly, param->bits);
    bit_reg_init(&msg, 8);

    bit_reg_set(&reg, param->init);
    bit_reg_set(&poly, param->poly);
    if (param->reverse) {
        bit_reg_reverse(&poly);
    }

    if (len > 0) {
        bit_reg_set(&msg, *data++);
    }

    while (len > 0) {
        if (param->non_direct) {
            if (param->reverse) {
                if (bit_reg_shift_right(&reg, bit_reg_shift_right(&msg, 0))) {
                    bit_reg_xor(&reg, bit_reg_get(&poly));
                }
            } else {
                if (bit_reg_shift_left(&reg, bit_reg_shift_left(&msg, 0))) {
                    bit_reg_xor(&reg, bit_reg_get(&poly));
                }
            }
        } else {
            if (param->reverse) {
                if (bit_reg_shift_right(&reg, 0) ^ bit_reg_shift_right(&msg, 0)) {
                    bit_reg_xor(&reg, bit_reg_get(&poly));
                }
            } else {
                if (bit_reg_shift_left(&reg, 0) ^ bit_reg_shift_left(&msg, 0)) {
                    bit_reg_xor(&reg, bit_reg_get(&poly));
                }
            }
        }

        len--;

        if (len % 8 == 0 && len > 0) {
            bit_reg_set(&msg, *data++);
        }
    }

    if (param->non_direct) {
        for (i = 0; i < param->bits; i++) {
            if (param->reverse) {
                if (bit_reg_shift_right(&reg, 0)) {
                    bit_reg_xor(&reg, bit_reg_get(&poly));
                }
            } else {
                if (bit_reg_shift_left(&reg, 0)) {
                    bit_reg_xor(&reg, bit_reg_get(&poly));
                }
            }
        }
    }

    bit_reg_xor(&reg, param->eor);

    return bit_reg_get(&reg);
}

/*
 * 9 bits (CRC-8)
 * 17 bits (CRC-16)
 * 33 bits (CRC-32)
 * 65 bits (CRC-64)
 */

#define crc16_poly 0x1021

/* On entry, addr=>start of data
 *              num = length of data
 *                           crc = incoming CRC     */
unsigned int crc16(char *addr, int num, unsigned int crc)
{
	int i;
	for (; num > 0; num--) {	/* Step through bytes in memory */
		crc = crc ^ (*addr++ << 8);	/* Fetch byte from memory, XOR into CRC top byte */
		for (i = 0; i < 8; i++) {	/* Prepare to rotate 8 bits */
			if (crc & 0x10000)	/* b15 is set... */
				crc = (crc << 1) ^ crc16_poly;	/* rotate and XOR with XMODEM polynomic */
			else	/* b15 is clear... */
				crc <<= 1;	/* just rotate */
		}		/* Loop for 8 bits */
		crc &= 0xFFFF;	/* Ensure CRC remains 16-bit value */
	}			/* Loop until num=0 */
	return (crc);		/* Return updated CRC */
}

#define crc32_poly 0xEDB88320

/* On entry, addr=>start of data
 *              num = length of data
 *                           crc = incoming CRC     */
unsigned int crc32(char *addr, int num, unsigned int crc)
{
	int i;
	for (; num > 0; num--) {	/* Step through bytes in memory */
		crc = crc ^ *addr++;	/* Fetch byte from memory, XOR into CRC */
		for (i = 0; i < 8; i++) {	/* Prepare to rotate 8 bits */
			if (crc & 1)	/* b0 is set... */
				crc = (crc >> 1) ^ crc32_poly;	/* rotate and XOR with ZIP polynomic */
			else	/* b0 is clear... */
				crc >>= 1;	/* just rotate */
			/* Some compilers need:
			 * crc &= 0xFFFFFFFF;
			 */
		}
		/* Loop for 8 bits */
	}
	/* Loop until num=0 */
	return (crc);		/* Return updated CRC */
}

/* ---------------------------------------------------------------------------------
 *                            Crypto Hashes
 * ---------------------------------------------------------------------------------
 */

#ifdef HAVE_OPENSSL

#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

/* MD4
 * MD5
 */

void math__md4_hash_string(unsigned char *string, long len, char outputBuffer[65])
{
	int i = 0;
	unsigned char hash[MD4_DIGEST_LENGTH];
	MD4_CTX md4;
	MD4_Init(&md4);
	MD4_Update(&md4, string, len);
	MD4_Final(hash, &md4);
	for (i = 0; i < MD4_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[64] = 0;
}

void math__md5_hash_string(unsigned char *string, long len, char outputBuffer[65])
{
	int i = 0;
	unsigned char hash[MD5_DIGEST_LENGTH];
	MD5_CTX md5;
	MD5_Init(&md5);
	MD5_Update(&md5, string, len);
	MD5_Final(hash, &md5);
	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[64] = 0;
}

/* SHA1
 * SHA-224
 * SHA-256
 * SHA-384
 * SHA-512
 */

void math__sha1_hash_string(unsigned char *string, long len, char outputBuffer[65])
{
	int i = 0;
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA_CTX sha1;
	SHA1_Init(&sha1);
	SHA1_Update(&sha1, string, len);
	SHA1_Final(hash, &sha1);
	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[64] = 0;
}

int math__sha1_file(char *path, char outputBuffer[65])
{
	unsigned char hash[SHA_DIGEST_LENGTH];
	const int bufSize = 32768;
	int bytesRead = 0;
	char *buffer = NULL;
	SHA_CTX sha1;

	FILE *file = fopen(path, "rb");
	if (!file)
		return -534;;

	SHA1_Init(&sha1);
	buffer = (char *)malloc(bufSize);
	if (!buffer)
		return ENOMEM;

	while ((bytesRead = fread(buffer, 1, bufSize, file))) {
		SHA1_Update(&sha1, buffer, bytesRead);
	}

	SHA1_Final(hash, &sha1);
	math__sha1_hash_string(hash, strlen((char*)hash), outputBuffer);
	fclose(file);
	free(buffer);
	return 0;
}

void math__sha256_hash_string(unsigned char *string, long len, char outputBuffer[65])
{
	int i = 0;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;

	SHA256_Init(&sha256);
	SHA256_Update(&sha256, string, len);
	SHA256_Final(hash, &sha256);
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[64] = 0;
}

int math__sha256_file(char *path, char outputBuffer[65])
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	const int bufSize = 32768;
	int bytesRead = 0;
	char *buffer = NULL;
	SHA256_CTX sha256;

	FILE *file = fopen(path, "rb");
	if (!file)
		return -534;

	SHA256_Init(&sha256);
	buffer = (char *)malloc(bufSize);
	if (!buffer)
		return ENOMEM;

	while ((bytesRead = fread(buffer, 1, bufSize, file))) {
		SHA256_Update(&sha256, buffer, bytesRead);
	}

	SHA256_Final(hash, &sha256);
	math__sha256_hash_string(hash, strlen((char*)hash), outputBuffer);
	fclose(file);
	free(buffer);
	return 0;
}

void math__sha512_hash_string(unsigned char *string, long len, char outputBuffer[129])
{
	int i = 0;
	unsigned char hash[SHA512_DIGEST_LENGTH];
	SHA512_CTX sha512;

	SHA512_Init(&sha512);
	SHA512_Update(&sha512, string, len);
	SHA512_Final(hash, &sha512);
	for (i = 0; i < SHA512_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[128] = 0;
}

int math__sha512_file(char *path, char outputBuffer[129])
{
	unsigned char hash[SHA512_DIGEST_LENGTH];
	const int bufSize = 32768;
	int bytesRead = 0;
	char *buffer = NULL;
	SHA512_CTX sha512;

	FILE *file = fopen(path, "rb");
	if (!file)
		return -534;

	SHA512_Init(&sha512);
	buffer = (char *)malloc(bufSize);
	if (!buffer)
		return ENOMEM;

	while ((bytesRead = fread(buffer, 1, bufSize, file))) {
		SHA512_Update(&sha512, buffer, bytesRead);
	}

	SHA512_Final(hash, &sha512);
	math__sha512_hash_string(hash, strlen((char*)hash), outputBuffer);
	fclose(file);
	free(buffer);
	return 0;
}

/* RIPEMD-160 */

void math__ripemd160_hash_string(unsigned char *string, long len,
			   char outputBuffer[65])
{
	int i = 0;
	unsigned char hash[RIPEMD160_DIGEST_LENGTH];
	RIPEMD160_CTX ripemd160;

	RIPEMD160_Init(&ripemd160);
	RIPEMD160_Update(&ripemd160, string, len);
	RIPEMD160_Final(hash, &ripemd160);
	for (i = 0; i < RIPEMD160_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[64] = 0;
}

int math__ripemd160_file(char *path, char outputBuffer[65])
{
	unsigned char hash[RIPEMD160_DIGEST_LENGTH];
	const int bufSize = 32768;
	int bytesRead = 0;
	char *buffer = NULL;
	RIPEMD160_CTX ripemd160;

	FILE *file = fopen(path, "rb");
	if (!file)
		return -534;

	RIPEMD160_Init(&ripemd160);
	buffer = (char *)malloc(bufSize);
	if (!buffer)
		return ENOMEM;

	while ((bytesRead = fread(buffer, 1, bufSize, file))) {
		RIPEMD160_Update(&ripemd160, buffer, bytesRead);
	}

	RIPEMD160_Final(hash, &ripemd160);
	math__ripemd160_hash_string(hash, strlen((char*)hash), outputBuffer);
	fclose(file);
	free(buffer);
	return 0;
}

#endif
