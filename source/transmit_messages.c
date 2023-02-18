/* transmit_messages.h
 *
 * ITESO - Especialidad en Sistemas Embebidos
 * Sistemas de Comunicación para Sistemas Embebidos
 * Práctica 1 
 * Cesar Tomás Gómez Cruz
 * Febrero de 2023
 *
 */

#include <transmit_messages.h>

/*!
 * @brief Initializes lwIP stack.
 *
 * @param arg unused
 */
void init_client(struct client_setup *client_)
{
    static struct netif netif;
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyHandle  = &phyHandle,
        .macAddress = configMAC_ADDR,
    };

    mdioHandle.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;
	
	/* Configuring the client parameters with the arguments received */
    IP4_ADDR(&netif_ipaddr, client_->client_ip_address[0], client_->client_ip_address[1], client_->client_ip_address[2], client_->client_ip_address[3]);
    IP4_ADDR(&netif_netmask, client_->mask[0], client_->mask[1], client_->mask[2], client_->mask[3]);
    IP4_ADDR(&netif_gw, client_->gateway[0], client_->gateway[1], client_->gateway[2], client_->gateway[3]);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" TCP Echo example\r\n");
    PRINTF("************************************************\r\n");
    PRINTF(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
           ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]);
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
           ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]);
    PRINTF(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
           ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]);
    PRINTF("************************************************\r\n");
}

/* The function received the struct with the encrypted data, the server IP address and other struct to store the
 * echoed messages from server */
void send_messages_by_tcp(struct encrypted_data *encryp_data, struct server_setup *server_, struct encrypted_data *encryp_echoed_data)
{
  /* Creating the struct that going to hold the client IP Address */
  struct client_setup client_;

  /* Defining client IP Address, socket number, mask and gateway IP Address */
  int client_ip[] = {192,168,0,102};
  int client_socket = 10000;
  int mask[] = {255,255,255,0};
  int gateway[] = {192,168,0,100};

  /* Assigning the parameters to client and server */
  client_.client_ip_address = client_ip;
  client_.socket = client_socket;
  client_.mask = mask;
  client_.gateway = gateway;

  struct netconn *conn;
  ip_addr_t server_addr;
  err_t err;

  uint8_t **encryp_echoed_messages;
  encryp_echoed_messages = (uint8_t **)malloc(encryp_data->how_many * sizeof(uint8_t *));

  for(int i = 0; i < encryp_data->how_many; i++) {
	encryp_echoed_messages[i] = (uint8_t *)malloc((encryp_data->lengths_encrypted_messages[i]) * sizeof(uint8_t));
  }

  /* Initializing the TCP stack with the client parameters */
  init_client(&client_);

  /* Configuring the server IP Address */
  server_addr.addr = IP4_ADDR(&server_addr, server_->server_ip_address[0], server_->server_ip_address[1], server_->server_ip_address[2], server_->server_ip_address[3]);
  conn = netconn_new(NETCONN_TCP);
  LWIP_ERROR("\n\rTCP echo: invalid conn", (conn != NULL), return;);
  
  /* Establishing the connection with the server */
  err = netconn_connect(conn, &server_addr, server_->socket);
  if (err == ERR_OK) {
	  PRINTF("\n\rConnected to server!\n");
  }

  /* Getting the number of messages that going to be send */
  int iterations = encryp_data->how_many;
  encryp_echoed_data->how_many = encryp_data->how_many;

  /* Sending every single encrypted message */
  while(iterations--)
  {
	  struct netbuf *buffer;
	  void *data;
	  u16_t len;
	  /* Extracting every message from the struct */
	  const char* message = encryp_data->encrypted_messages[iterations];
	  data = (void*) message;
	  len = encryp_data->lengths_encrypted_messages[iterations];
	  /* Sending the encrypted message to the server */
	  write_messages_to_tcp(conn,data,len);

	  /* Receiving the echo from the server */
	  echo_received_from_tcp(conn, &buffer);

		/* Printing the echo received with the decrypted message */
//		PRINTF("\n\rEcho: ");
		do {
		  netbuf_data(buffer, &data, &len);
		  uint8_t *cdata = (uint8_t *)data;
		  for(int i = 0; i < len; i++) {
			  encryp_echoed_data->encrypted_messages[iterations][i] = cdata[i];
			  encryp_echoed_data->lengths_encrypted_messages[iterations] = len;
//			  PRINTF("0x%02x,", encryp_echoed_data->encrypted_messages[iterations][i]);
		  }
	    } while (netbuf_next(buffer) >= 0);
		  netbuf_delete(buffer);
		//PRINTF("\n\rEchoed Message: %s", encryp_echoed_messages[iterations]);

	  vTaskDelay(500/portTICK_PERIOD_MS);
  }

  /* Close connection and discard connection identifier. */
  netconn_close(conn);
  netconn_delete(conn);
  return;
}

/* Function to send messages to the server*/
void write_messages_to_tcp(struct netconn *conn, void *data, u16_t len) {
	err_t err;
	err = netconn_write(conn, data, len, NETCONN_COPY);
	if (err != ERR_OK) {
	  PRINTF("\n\rTCP Client: netconn_write: error \"%s\"\n", lwip_strerr(err));
	  return;
	}
	PRINTF("\n\rMessage was send it successfully!");
	return;
}

/* Function to received the echoes from the server */
void echo_received_from_tcp(struct netconn *conn, struct netbuf **buffer) {
	err_t err;
	err = netconn_recv(conn, buffer);
	if (err != ERR_OK) {
	  PRINTF("\n\rTCP Client: netconn_recv: error \"%s\"\n", lwip_strerr(err));
	  return;
	}
	PRINTF("\n\rEcho received!");
	return;
}



 
 
 
