/* Checksums */
unsigned int crc16(char *addr, int num, unsigned int crc);
unsigned int crc32(char *addr, int num, unsigned int crc);

/* MD hashes */
void md4_hash_string(unsigned char *string, long len, char *outputBuffer);
void md5_hash_string(unsigned char *string, long len, char *outputBuffer);

/* SHA hashes */
void sha1_hash_string(unsigned char *string, long len, char *outputBuffer);
void sha256_hash_string(unsigned char *string, long len, char *outputBuffer);
void sha512_hash_string(unsigned char *string, long len, char *outputBuffer);

/* RIPEMD hash */
void ripemd160_hash_string(unsigned char *string, long len, char *outputBuffer);
