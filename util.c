#include "bvi.h"
#include "set.h"

#ifdef HAVE_OPENSSL

#include <openssl/sha.h>

void sha1_hash_string(unsigned char *string, char outputBuffer[65])
{
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA_CTX sha1;
	SHA1_Init(&sha1);
	SHA1_Update(&sha1, string, strlen(string));
	SHA1_Final(hash, &sha1);
	int i = 0;
	for(i = 0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[64] = 0;
}

int sha1_file(char *path, char outputBuffer[65])
{
	FILE *file = fopen(path, "rb");
	if(!file) return -534;
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA_CTX sha1;
	SHA1_Init(&sha1);
	const int bufSize = 32768;
	char *buffer = malloc(bufSize);
	int bytesRead = 0;
	if(!buffer) return ENOMEM;

	while((bytesRead = fread(buffer, 1, bufSize, file))) {
		SHA1_Update(&sha1, buffer, bytesRead);
	}

	SHA1_Final(hash, &sha1);
	sha1_hash_string(hash, outputBuffer);
	fclose(file);
	free(buffer);
	return 0;
}

void sha256_hash_string(unsigned char *string, char outputBuffer[65])
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, string, strlen(string));
	SHA256_Final(hash, &sha256);
	int i = 0;
	for(i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
	}
	outputBuffer[64] = 0;
}

int sha256_file(char *path, char outputBuffer[65])
{
	FILE *file = fopen(path, "rb");
	if(!file) return -534;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	const int bufSize = 32768;
	char *buffer = malloc(bufSize);
	int bytesRead = 0;
	if(!buffer) return ENOMEM;

	while((bytesRead = fread(buffer, 1, bufSize, file))) {
		SHA256_Update(&sha256, buffer, bytesRead);
	}

	SHA256_Final(hash, &sha256);
	sha256_hash_string(hash, outputBuffer);
	fclose(file);
	free(buffer);
	return 0;
}
#endif

