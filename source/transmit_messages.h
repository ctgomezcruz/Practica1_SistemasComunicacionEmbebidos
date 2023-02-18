/* transmit_messages.h
 *
 * ITESO - Especialidad en Sistemas Embebidos
 * Sistemas de Comunicación para Sistemas Embebidos
 * Práctica 1 
 * Cesar Tomás Gómez Cruz
 * Febrero de 2023
 *
 */

#ifndef TRANSMIT_MESSAGES_H__
#define TRANSMIT_MESSAGES_H__

#include "lwip/opt.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_phy.h"

#include "fsl_phyksz8081.h"
#include "fsl_enet_mdio.h"
#include "fsl_device_registers.h"

/* MAC address configuration. */
#ifndef configMAC_ADDR
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }
#endif

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* MDIO operations. */
#define EXAMPLE_MDIO_OPS enet_ops

/* PHY operations. */
#define EXAMPLE_PHY_OPS phyksz8081_ops

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)

#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/*******************************************************************************
 * Variables
 ******************************************************************************/
static mdio_handle_t mdioHandle = {.ops = &EXAMPLE_MDIO_OPS};
static phy_handle_t phyHandle   = {.phyAddr = EXAMPLE_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &EXAMPLE_PHY_OPS};

/* Struct that holds the encrypted messages, how many of them are and their lengths */
struct encrypted_data {
	int how_many;
	uint8_t **encrypted_messages;
	int *lengths_encrypted_messages;
};

/* Struct that holds the server IP Address and socket */
struct server_setup {
	int *server_ip_address;
	int socket;
};

/* Struct that holds the client parameters */
struct client_setup {
	int *client_ip_address;
	int socket;
	int *mask;
	int *gateway;
};

/* Function prototypes */
void init_client(struct client_setup *);
void send_messages_by_tcp(struct encrypted_data *encryp_data, struct server_setup *server_, struct encrypted_data *encryp_echoed_data);
void write_messages_to_tcp(struct netconn *, void *, u16_t);
void echo_received_from_tcp(struct netconn *, struct netbuf **);

#endif /* TRANSMIT_MESSAGES_H__ */
