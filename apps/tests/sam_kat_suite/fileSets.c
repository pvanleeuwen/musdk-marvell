/******************************************************************************
 *      Copyright (C) 2016 Marvell International Ltd.
 *
 *  If you received this File from Marvell, you may opt to use, redistribute
 *  and/or modify this File under the following licensing terms.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *      * Neither the name of Marvell nor the names of its contributors may be
 *        used to endorse or promote products derived from this software
 *        without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "fileSets.h"
#include "encryptedBlock.h"
#include "common.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
/**
 * fileSetsGetArgs: gets a new type and its relevant data from the file.
 *
 * @param fstr - The pointer to the file to take the arguments from.
 * @param line - The line to get the arguments from.
 * @param type - The output element type.
 * @param outputDataArray - The element array.
 * @param outputDataNumber - The element array length or the counter value.
 * 	 @return
 * 	FILE_NULL_ARGS - If at least one of the parameters is NULL
 * 	FILE_TOO_BIG_STRING - If the phrase to get is bigger than
 * 		CHUNK_SIZE (who's defined)
 * 	FILE_NOT_VALID - If other error occurred
 * 	FILE_SUCCESS - If the function succeed to get the arguments
 */
static FileMessage fileSetsGetArgs(FILE *fstr, char *line,
		EncryptedBlockType *type, unsigned char *outputDataArray,
		int *outputDataNumber);
/**
 * fileSetsGetCounter: gets a new counter from the given line.
 *
 * @param line - The line to get the counter from.
 * @param outputCounter - The counter value to assign to.
 * 	 @return
 * 	FILE_NOT_VALID - If an error occurred while trying to get the counter
 * 			from the line
 * 	FILE_SUCCESS - If the function succeed to get the counter
 */
static FileMessage fileSetsGetCounter(char *line, int *outputCounter);
/**
 * fileSetsGetString: gets a string from the given line.
 *
 * @param line - The line to get the string from.
 * @param outputDataArray - The array to assign the string to.
 * @param outputDataLen - The length of the string.
 * 	 @return
 * 	FILE_NOT_VALID - If an error occurred while trying to get the string
 * 			from the line
 * 	FILE_SUCCESS - If the function succeed to gets the counter
 */
static FileMessage fileSetsGetString(char *line, unsigned char *outputDataArray,
		int *outputDataLen);
/**
 * fileSetsGetData: gets a new data from the file.
 *
 * @param fstr - The pointer to the file to take the data from
 * @param line - The first line of the data
 * @param dataOut - The element array
 * @param dataOutLen - The element array length
 * 	 @return
 * 	FILE_NULL_ARGS - If at least one of the parameters is NULL
 * 	FILE_NOT_VALID - If the data is not a valid hex phrase
 * 	FILE_TOO_BIG_STRING - If the data to get is bigger than
 * 		CHUNK_SIZE (who's defined)
 * 	FILE_SUCCESS - If the function succeed to get the data
 */
static FileMessage fileSetsGetData(FILE *fstr, char *line,
		unsigned char *outputDataArray, int *outputDataLen);
/**
 * fileSetsGetTypeFromString: Gets the encrypted block type.
 *
 * @param str - The string to get the type from.
 * @return
 *  NAME - If the string is "Name"
 *  KEY - If the string is "Key"
 *  AUTH_KEY - If the string is "Authkey"
 *  IV - If the string is "IV"
 *  ICB - If the string is "ICB"
 *  PLAINTEXT - If the string is "PLAINTEXT" or "PT"
 *  CIPHERTEXT - If the string is "Ciphertext" or "CT"
 *  TESTCOUNTER - If the string is "Testcounter"
 *  MODE - If the string is "Mode"
 *  ALGORITHM - If the string is "Algorithm"
 *  AUTH_ALGORITHM - If the string is "Authalgorithm"
 *  DIRECTION - If the string is "Direction"
 *  CRYPTO_OFFSET - If the string is "Cryptoffset"
 *  INVALID - If other string was sent
 */
static EncryptedBlockType fileSetsGetTypeFromString(char *str);

/**
 * fileSetsCnvertMessage: Converts encrypted block message to file sets message.
 *
 * @param message - The encrypted block message to be converted
 * 	 @return
 * 		the relevant file sets message
 */
static FileMessage fileSetsConvertMessage(EncryptedBlockMessage message);
///////////////////////////////////////////////////////////////////////////////

FileMessage fileSetsReadBlocksFromFile(char *fileName, generic_list encryptedBlocks)
{
	EncryptedBlockMessage encryptedBlockMessage;
	EncryptedBlockPtr currentBlock;
	FILE *fstr;
	char line[CHUNK_SIZE] = { 0 };

	if (!fileName || !encryptedBlocks) {
		return FILE_NULL_ARGS;
	}

	fstr = fopen(fileName, "r");
	if (fstr == NULL) {
		return FILE_OPEN_PROBLEM;
	}

	while (fgets(line, CHUNK_SIZE, fstr) != NULL) {
		EncryptedBlockType type = INVALID;
		unsigned char dataArray[CHUNK_SIZE] = { 0 };
		int dataNumber = 0;
		FileMessage message;
		int current_index = 0, next_index = 0;

		if (isCommentOrEmptyLine(line)) {
			cleanStr(line);
			continue;
		}

		message = fileSetsGetArgs(fstr, line, &type, dataArray, &dataNumber);
		if (message != FILE_SUCCESS) {
			printf("fileSetsGetArgs FAILED - rc = %d, type = %d\n", message, type);
			fclose(fstr);
			return message;
		}

		if (type == NEW_BLOCK_TYPE) {
			encryptedBlockMessage = encryptedBlockCreate(&currentBlock);
			if (encryptedBlockMessage == ENCRYPTEDBLOCK_OUT_OF_MEMORY) {
				fclose(fstr);
				return FILE_OUT_OF_MEMORY;
			}
			current_index = 0;
			next_index = 0;
		} else {
			currentBlock = generic_list_get_last(encryptedBlocks);
		}

		if (isEncryptedBlockTypeSession(type)) {
			message = fileSetsConvertMessage(
				encryptedBlockSessionAddElement(currentBlock, type, dataArray,
						dataNumber));
		} else if (isEncryptedBlockTypeOperation(type)) {
			if (type == NEW_OPERATION_TYPE) {
				current_index = next_index;
				next_index++;
			}
			if (!encryptedBlockOperationExist(currentBlock, current_index)) {
				encryptedBlockMessage = encryptedBlockOperationCreate(currentBlock, current_index);
				if (encryptedBlockMessage != ENCRYPTEDBLOCK_SUCCESS) {
					fclose(fstr);
					return FILE_OUT_OF_MEMORY;
				}
			}
			message = fileSetsConvertMessage(
				encryptedBlockOperationAddElement(currentBlock, type, current_index,
								dataArray, dataNumber));
		} else {
			printf("Unexpected type = %d\n", type);
			message = FILE_NULL_ARGS;
		}
		if (message != FILE_SUCCESS) {
			if (type == NEW_BLOCK_TYPE) {
				encryptedBlockDestroy(currentBlock);
			}
			fclose(fstr);
			return message;
		}
		if (type == NEW_BLOCK_TYPE) {
			if (generic_list_insert_last(encryptedBlocks, currentBlock) != LIST_SUCCESS) {
				encryptedBlockDestroy(currentBlock);
				fclose(fstr);
				return FILE_OUT_OF_MEMORY;
			}
			encryptedBlockDestroy(currentBlock);
		}
		cleanStr(line);
	}
	fclose(fstr);
	return FILE_SUCCESS;
}
static FileMessage fileSetsGetArgs(FILE *fstr, char *line,
		EncryptedBlockType *type, unsigned char *outputDataArray,
		int *outputDataNumber)
{
	if (!line || !type || !outputDataArray) {
		return FILE_NULL_ARGS;
	}
	char firstArg[CHUNK_SIZE] = { 0 }, secondArg;
	int scanned = sscanf(line, " %s %c", firstArg, &secondArg);
	if (scanned != 2) {
		return FILE_NOT_VALID;
	}
	*type = fileSetsGetTypeFromString(firstArg);
	if (*type == INVALID || secondArg != ':') {
		printf("Unknown parameter: %s\n", firstArg);
		return FILE_NOT_VALID;
	}
	removeBeforeColon(line);
	switch (*type) {
	case TESTCOUNTER:
	case CRYPTO_OFFSET:
	case AUTH_OFFSET:
		return fileSetsGetCounter(line, outputDataNumber);
		break;
	case NAME:
	case ALGORITHM:
	case AUTH_ALGORITHM:
	case MODE:
	case DIRECTION:
		return fileSetsGetString(line, outputDataArray, outputDataNumber);
		break;
	default:
		return fileSetsGetData(fstr, line, outputDataArray, outputDataNumber);
		break;
	}
}
static FileMessage fileSetsGetCounter(char *line, int *outputCounter)
{
	if (sscanf(line, " %d", outputCounter) != 1) {
		return FILE_NOT_VALID;
	}
	return FILE_SUCCESS;
}
static FileMessage fileSetsGetString(char *line, unsigned char *outputDataArray,
					int *outputDataLen) {
	if (sscanf(line, " %s", outputDataArray) != 1) {
		return FILE_NOT_VALID;
	}
	if (*outputDataArray == '"') {
		if (!getPhraseInQuotationMarks((char*) outputDataArray, line)) {
			return FILE_NOT_VALID;
		}
	}
	*outputDataLen = strlen((char*) outputDataArray) + 1;
	return FILE_SUCCESS;
}

static FileMessage fileSetsGetData(FILE *fstr, char *line,
		unsigned char *outputDataArray, int *outputDataLen)
{
	char strOut[CHUNK_SIZE] = { 0 };
	bool continuesData;
	int scanned, strLen = 0;
	do {
		continuesData = isContinuesData(line);
		char str[CHUNK_SIZE] = { 0 };
		scanned = sscanf(line, "%s", str);
		if (scanned != 1) {
			printf("%s: sscanf Failed - scanned = %d\n", __func__, scanned);
			return FILE_NOT_VALID;
		}
		if (str[0] != '"') {
			removeHexHead(str);
			if (!isValidHexPhrase(str)) {
				return FILE_NOT_VALID;
			}
			if ((strLen + strlen(str)) >= CHUNK_SIZE) {
				return FILE_TOO_BIG_PHRASE;
			}
			strcpy(&strOut[strLen], str);
		} else {
			if (!getPhraseInQuotationMarks(str, line)) {
				return FILE_NOT_VALID;
			}
			if ((strLen + strlen(str)) >= CHUNK_SIZE) {
				return FILE_TOO_BIG_PHRASE;
			}
			strAsci2HexAsci(str, &strOut[strLen]);
			strLen += strlen(str);
		}
		strLen += strlen(str);
		if (continuesData) {
			cleanStr(line);
			if (fgets(line, CHUNK_SIZE, fstr) == NULL) {
				printf("%s: Can't read continues Data\n", __func__);
				return FILE_NOT_VALID;
			}
		}
	} while (continuesData);
	*outputDataLen = strLen / 2;
	hexString2Int(strOut, outputDataArray);
	return FILE_SUCCESS;
}
static EncryptedBlockType fileSetsGetTypeFromString(char* str) {
	if (!str) {
		return INVALID;
	}
	if (strcmp(str, "Name") == 0)
		return NAME;

	if (strcmp(str, "Key") == 0)
		return KEY;

	if (strcmp(str, "Authkey") == 0)
		return AUTH_KEY;

	if (strcmp(str, "IV") == 0)
		return IV;

	if (strcmp(str, "ICB") == 0)
		return ICB;

	if (strcmp(str, "AAD") == 0)
		return AAD;

	if (strcmp(str, "Plaintext") == 0 || strcmp(str, "PT") == 0)
		return PLAINTEXT;

	if (strcmp(str, "Ciphertext") == 0 || strcmp(str, "CT") == 0)
		return CIPHERTEXT;

	if (strcmp(str, "Testcounter") == 0)
		return TESTCOUNTER;

	if (strcmp(str, "Mode") == 0)
		return MODE;

	if (strcmp(str, "Algorithm") == 0)
		return ALGORITHM;

	if (strcmp(str, "Authalgorithm") == 0)
		return AUTH_ALGORITHM;

	if (strcmp(str, "Cryptoffset") == 0)
		return CRYPTO_OFFSET;

	if (strcmp(str, "Authoffset") == 0)
		return AUTH_OFFSET;

	if (strcmp(str, "Direction") == 0)
		return DIRECTION;

	return INVALID;
}

static FileMessage fileSetsConvertMessage(EncryptedBlockMessage message)
{
	switch (message) {
	case ENCRYPTEDBLOCK_SUCCESS:
		return FILE_SUCCESS;
		break;
	case ENCRYPTEDBLOCK_ELEMENT_ALREADY_EXIST:
		return FILE_ELEMENT_ALREADY_EXIST;
		break;
	case ENCRYPTEDBLOCK_OUT_OF_MEMORY:
		return FILE_OUT_OF_MEMORY;
		break;
	case ENCRYPTEDBLOCK_NOT_VALID_ARGS:
	case ENCRYPTEDBLOCK_NOT_VALID_TYPE:
	case ENCRYPTEDBLOCK_NOT_VALID:
		return FILE_NOT_VALID;
		break;
	case ENCRYPTEDBLOCK_NULL_ARGS:
		return FILE_NULL_ARGS;
		break;
	default:
		return FILE_NOT_VALID;
		break;
	}
	return FILE_NOT_VALID;
}

generic_list_element fileSetsEncryptedBlockCopyForList(generic_list_element encryptedBlock)
{
	if (!encryptedBlock) {
		return NULL;
	}
	EncryptedBlockPtr copyBlock = NULL;
	EncryptedBlockMessage message = encryptedBlockCopy(encryptedBlock,
			&copyBlock);
	if (message != ENCRYPTEDBLOCK_SUCCESS) {
		return NULL;
	}
	return copyBlock;
}

void fileSetsEncryptedBlockDestroyForList(generic_list_element encryptedBlock)
{
	encryptedBlockDestroy(encryptedBlock);
}
