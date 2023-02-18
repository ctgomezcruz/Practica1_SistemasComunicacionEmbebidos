/* send_secure_messages.c
 *
 * ITESO - Especialidad en Sistemas Embebidos
 * Sistemas de Comunicación para Sistemas Embebidos
 * Práctica 1 
 * Cesar Tomás Gómez Cruz
 * Febrero de 2023
 *
 */

#include "lwip/opt.h"
#include <send_secure_messages.h>
#include <aes_and_crc.h>

/* Top function of the messaging service */
void send_secure_messages(void *arg)
{
	/* Creating the struct that going to hold all the messages and the number of them */
	struct data summary;
	/* Array of char arrays with the messages and integer with the number of messages */
    char *messages[] = {"                This is the first message of many that I am going to send!", "                This is the second message!", "                This is the third message!", "                This is the fourth message!", "                This is the fifth message!", "                Dabale arroz a la zorra el Abad.", "                Lucy in the sky with diamonds.", "                In the town where I was born live a man who sails the sea..."};
    int how_many = 8;
    /* Configuring the server IP address */
    int server_ip[] = {192,168,0,100};
    int server_socket = 10000;

	/* Putting the messages and the number of them in the structure */
    summary.how_many = how_many;
    summary.messages = messages;
	/* Calling the next layer of the service where the messages are encrypted and the CRC are calculated.
	 * Also it is send to the next layer the server IP address */
    encrypt_protect_and_send_messages(&summary, server_ip, server_socket);
    vTaskDelete(NULL);
}
