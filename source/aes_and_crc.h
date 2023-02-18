/* aes_and_crc.h
 *
 * ITESO - Especialidad en Sistemas Embebidos
 * Sistemas de Comunicación para Sistemas Embebidos
 * Práctica 1 
 * Cesar Tomás Gómez Cruz
 * Febrero de 2023
 *
 */

#ifndef AES_AND_CRC_H__
#define AES_AND_CRC_H__

//#include "send_secure_messages.h"
#include "aes.h"
#include "fsl_crc.h"
#include "transmit_messages.h"

/* Struct that holds the messages that going to be send and the number of them */
struct data {
	int how_many;
	char **messages;
};

/* Function prototypes */
void InitCrc32(CRC_Type *, uint32_t);
void encrypt_protect_and_send_messages(struct data *, int *, int);
void decrypt_and_check_echoed_messages(struct encrypted_data *);
int check_CRC(uint8_t *, uint8_t, uint8_t *);

#endif /* AES_AND_CRC_H__ */
