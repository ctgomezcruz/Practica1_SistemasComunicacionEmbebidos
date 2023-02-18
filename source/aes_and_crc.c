/* aes_and_crc.c
 *
 * ITESO - Especialidad en Sistemas Embebidos
 * Sistemas de Comunicación para Sistemas Embebidos
 * Práctica 1 
 * Cesar Tomás Gómez Cruz
 * Febrero de 2023
 *
 */

#include "aes_and_crc.h"
#include "transmit_messages.h"

/*!
 * @brief Init for CRC-32.
 * @details Init CRC peripheral module for CRC-32 protocol.
 *          width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
 *          name="CRC-32"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
void InitCrc32(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;
    config.polynomial         = 0x04C11DB7U;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}

/* The function receives a struct with the messages, the number of them
 * and the server IP address and socket*/
void encrypt_protect_and_send_messages(struct data *summary, int *server_ip, int server_socket)
{
	/* Creating the struct that going to hold all the messages encrypted */
	struct encrypted_data encryp_data;
	/* Creating the struct that going to hold the server IP Address */
	struct server_setup server_;
	/* Creating the struct that going to hold all the echoed messages encrypted */
    struct encrypted_data encryp_echoed_data;

    char *message_;
    uint8_t *CRC_byte;

	/* Defining the server IP Address */
    server_.server_ip_address = server_ip;
    server_.socket = server_socket;

	/* Passing the number of messages from parameters received to new struct with the messages encrypted */
    encryp_data.how_many = summary->how_many;
	/* Allocating memory for the messages encrypted and their lengths */
    encryp_data.encrypted_messages = (uint8_t **)malloc(encryp_data.how_many * sizeof(uint8_t *));
    encryp_data.lengths_encrypted_messages = (int *)malloc(encryp_data.how_many * sizeof(int));

	/* Passing the number of messages from parameters received to new struct with the messages encrypted */
    encryp_echoed_data.how_many = summary->how_many;
	/* Allocating memory for the echoed messages encrypted and their lengths */
    encryp_echoed_data.encrypted_messages = (uint8_t **)malloc(encryp_echoed_data.how_many * sizeof(uint8_t *));
    encryp_echoed_data.lengths_encrypted_messages = (int *)malloc(encryp_echoed_data.how_many * sizeof(int));

    /* AES data */
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	struct AES_ctx ctx;
	size_t test_string_len, padded_len;
	uint8_t padded_msg[256] = {0};

	/* CRC data */
	CRC_Type *base = CRC0;
	uint32_t checksum32;

	PRINTF("AES and CRC test task\r\n");

	PRINTF("\n\rTesting AES128\n");
	/* Init the AES context structure */
	AES_init_ctx_iv(&ctx, key, iv);

	/*
 	   Taking every single message stored in the struct to encrypted it, put its lenght in 1 byte and calculate its CRC.
	   Every message is padded until complete a multiple of 16.
	   The message lenght is added at the end of the datagram with the full encrypted message.
	   The last 4 bytes of the datagram are the CRC calculated.
	   ______________________________        ______________________________________________________________________________________
	   |     |     |     |    |     |  ...   |     |     | Msg lenght | CRC 1st byte | CRC 2nd byte | CRC 3rd byte | CRC 4th byte |
	   ------------------------------        --------------------------------------------------------------------------------------
	   |<-    Encrypted message in multiples of 16     ->|
	*/
    for(int i = 0; i < summary->how_many; i++){
    	/* To encrypt an array its lenght must be a multiple of 16 so we add zeros */
    	test_string_len = strlen(summary->messages[i]);
		padded_len = test_string_len + (16 - (test_string_len%16));
		memcpy(padded_msg, summary->messages[i], test_string_len);

		/* Encrypting one single message */
		AES_CBC_encrypt_buffer(&ctx, padded_msg, padded_len);

		/* Allocating memory for encrypted message plus 5 bytes: 
		   1 byte for the message length and 4 for the CRC */
		encryp_data.encrypted_messages[i] = (uint8_t *)malloc((padded_len + 5) * sizeof(uint8_t));
		encryp_data.lengths_encrypted_messages[i] = padded_len + 5;

		/* Allocating memory for encrypted echoed message plus 5 bytes:
		   1 byte for the message length and 4 for the CRC */
		encryp_echoed_data.encrypted_messages[i] = (uint8_t *)malloc((padded_len + 5) * sizeof(uint8_t));
		encryp_echoed_data.lengths_encrypted_messages[i] = padded_len + 5;

		/* Assembling the first part of the datagram: the encrypted message.
		   And printing the encrypted message in the console */
		PRINTF("\n\rEncrypted Message - CRC32 (%d): ", encryp_data.lengths_encrypted_messages[i]);
		for(int j = 0; j < padded_len; j++) {
			encryp_data.encrypted_messages[i][j] = padded_msg[j];
			PRINTF("0x%02x,", encryp_data.encrypted_messages[i][j]);
		}
		
		/* Calculating the CRC checksum */
    	InitCrc32(base, 0xFFFFFFFFU);
    	CRC_WriteData(base, (uint8_t *)&padded_msg[0], padded_len);
    	checksum32 = CRC_Get32bitResult(base);
    	PRINTF("\n\rCRC-32: 0x%08x", checksum32);
		
		/* Adding the message's length to the datagram */
    	encryp_data.encrypted_messages[i][padded_len] = (uint8_t) test_string_len;

		/* Adding the four bytes of the CRC to the datagram */
    	CRC_byte = (uint8_t *)&checksum32;
		for(int k = padded_len+1; k < (padded_len + 5); k++) {
			encryp_data.encrypted_messages[i][k] = *CRC_byte;
			CRC_byte++;
		}
    }

	/* Calling the next service layer to transmit the encrypted message and received the encrypted
	 * echoed messages. The server IP address is also passed to next layer.*/
    send_messages_by_tcp(&encryp_data, &server_, &encryp_echoed_data);

    /*
    PRINTF("\n\rEchoed encrypted messages printed from AES and CRC: ");
    for(int i = 0; i < encryp_echoed_data.how_many; i++) {
    	PRINTF("\n\rMessage + CRC32 %d: ", i);
    	for(int j = 0; j < encryp_echoed_data.lengths_encrypted_messages[i]; j++) {
        	PRINTF("0x%02x,", encryp_echoed_data.encrypted_messages[i][j]);
    	}
    }
    */

    /* Once the encrypted echoed messages are stored in its corresponding data structure
     * they are send to check its integrity and be decrypted */
    decrypt_and_check_echoed_messages(&encryp_echoed_data);

	/* Freeing the allocated memory */
    free(encryp_data.lengths_encrypted_messages);
    for(int i = 0; i < encryp_data.how_many; i++) {
    	free(encryp_data.encrypted_messages[i]);
    }
    free(encryp_data.encrypted_messages);

    free(encryp_echoed_data.lengths_encrypted_messages);
    for(int i = 0; i < encryp_echoed_data.how_many; i++) {
    	free(encryp_echoed_data.encrypted_messages[i]);
    }
    free(encryp_echoed_data.encrypted_messages);
    return;
}

/* The function received the struct with the encrypted echoed messages to check its integrity
 * and decrypted */
void decrypt_and_check_echoed_messages(struct encrypted_data *encryp_echoed_data) {
	uint8_t *CRC = (uint8_t *)malloc(4 * sizeof(uint8_t));
	uint8_t original_message_length;
	uint8_t *encrypted_echoed_message = (uint8_t *)malloc(256 * sizeof(uint8_t));
	uint8_t *first_message_byte = encrypted_echoed_message;
	uint8_t *first_CRC_byte = CRC;
	int valid_mssg = 0;

    /* AES data */
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, key, iv);

	/* Iterating through all the encrypted messages */
	PRINTF("\n\rCRC and encrypted message extracted from echoed messages!");
	for (int i = 0; i < encryp_echoed_data->how_many; i++) {
		/* First: extracting the CRC bytes of each encrypted message */
		PRINTF("\n\rCRC %d: ", i);
		for(int j = (encryp_echoed_data->lengths_encrypted_messages[i]-1); j > (encryp_echoed_data->lengths_encrypted_messages[i] - 5); j--){
			*CRC = encryp_echoed_data->encrypted_messages[i][j];
			PRINTF("0x%02x,", *CRC);
			CRC++;
		}
		CRC = first_CRC_byte;

		/* Getting the original length of the message before to be padded to be encrypted */
		original_message_length = encryp_echoed_data->encrypted_messages[i][(encryp_echoed_data->lengths_encrypted_messages[i])-5];
//		PRINTF("\n\rOriginal message length: %d", (int)original_message_length);

		/* Getting only the bytes corresponding to the encrypted message */
//		PRINTF("\n\rMessage %d (%d): ", i, encryp_echoed_data->lengths_encrypted_messages[i]-5);
		for(int k = 0; k < (encryp_echoed_data->lengths_encrypted_messages[i]-5); k++) {
			*encrypted_echoed_message = encryp_echoed_data->encrypted_messages[i][k];
//			PRINTF("0x%02x,", *encrypted_echoed_message);
			encrypted_echoed_message++;
		}
		encrypted_echoed_message = first_message_byte;

		/* Checking the message integrity through the CRC bytes */
		valid_mssg = check_CRC(encrypted_echoed_message, encryp_echoed_data->lengths_encrypted_messages[i]-5, CRC);

		/* If the message's integrity doesn't have issues then the message is decrypted
		 * else a error message is printed in the terminal and next messages is processed */
		if(valid_mssg != 0) {
			PRINTF("\n\rValid message, CRCs match!");
			AES_CBC_decrypt_buffer(&ctx, encrypted_echoed_message, encryp_echoed_data->lengths_encrypted_messages[i]-5);
		} else {
			PRINTF("\n\rInvalid message, CRCs don't match!");
		}

		/* Once the message is decrypted, it is printed (only the significant part) */
		PRINTF("\n\rDecrypted message (only the significant part): ");
		for(int m = 0; m < original_message_length; m++) {
			if(m > 15) {
				PRINTF("%c", *encrypted_echoed_message);
			}
			encrypted_echoed_message++;
		}
		encrypted_echoed_message = first_message_byte;
	}

	/* Freeing the allocated memory */
	free(encrypted_echoed_message);
	return;
}

int check_CRC(uint8_t *encryp_echoed_message, uint8_t length, uint8_t *CRC) {
	/* CRC data */
	CRC_Type *base_ = CRC0;
	uint32_t checksum32;
	uint8_t *first_byte = encryp_echoed_message;
	uint8_t *last_CRC = (uint8_t *)(&checksum32) + 3;
	int valid_mssg = 1;
/*
	PRINTF("\n\rMessage (%d): ", length);
	for(int k = 0; k < length; k++) {
		PRINTF("0x%02x,", *encryp_echoed_message);
		encryp_echoed_message++;
	}
*/
//	encryp_echoed_message = first_byte;

	/* Calculating the checksum of the encrypted echoed message */
	InitCrc32(base_, 0xFFFFFFFFU);
	CRC_WriteData(base_, (uint8_t *)encryp_echoed_message, length);
	checksum32 = CRC_Get32bitResult(base_);
	PRINTF("\n\rCalculated CRC-32 from encrypted echoed message: 0x%08x", checksum32);

	/* Comparing the two checksums.
	 * The one recently calculated and the one attached to the echoed message */
	for(int j = 0; j < 4; j++) {
//		PRINTF("\n\r0x%02x ?= 0x%02x", *CRC, *last_CRC);
		if(*CRC != *last_CRC) {
			PRINTF("\n\rWrong message!");
			valid_mssg = 0;
			return valid_mssg;
		}
		CRC++;
		last_CRC--;
	}
	return valid_mssg;
}
