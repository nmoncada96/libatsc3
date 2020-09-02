/*
 * atsc3_utils_sha256.h
 *
 *  Created on: Jul 27, 2020
 *      Author: jjustman
 *
 *      borrowed from: https://raw.githubusercontent.com/B-Con/crypto-algorithms/master/sha256.h
 */


#ifndef ATSC3_UTILS_SHA256_H_
#define ATSC3_UTILS_SHA256_H_

/*********************************************************************
* Filename:   sha256.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding SHA1 implementation.
*********************************************************************/

#ifndef SHA256_H
#define SHA256_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include <stdint.h>

/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTE;             // 8-bit byte
#ifdef _WIN32
typedef uint16_t WORD;
#else
typedef unsigned int  WORD;             // 32-bit word, change to "long" for 16-bit machines
#endif

typedef struct {
	BYTE data[64];
	WORD datalen;
	unsigned long long bitlen;
	WORD state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, BYTE hash[]);

#endif   // SHA256_H

#endif /* ATSC3_UTILS_SHA256_H_ */
