/*
 * @brief LWIP FreeRTOS HTTP Webserver example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "netif/etharp.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#include <string.h>
#include "board.h"
#include "lpc_phy.h"
#include "arch\lpc17xx_40xx_emac.h"
#include "arch\lpc_arch.h"
#include "arch\sys_arch.h"
#include "lpc_phy.h"/* For the PHY monitor support */


#include "ring_buffer.h"

#include "motorCtrl.h"
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* NETIF data */
static struct netif lpc_netif;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

extern void http_server_netconn_init(void);

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* LED0 is used for the link status, on = PHY cable detected */
	/* Initial LED state is off to show an unconnected cable state */
	Board_LED_Set(0, false);
}

/* Callback for TCPIP thread to indicate TCPIP init is done */
static void tcpip_init_done_signal(void *arg)
{
	/* Tell main thread TCP/IP init is done */
	*(s32_t *) arg = 1;
}

/* LWIP kickoff and PHY link monitor thread */
static void vSetupIFTask(void *pvParameters) {
	ip_addr_t ipaddr, netmask, gw;
	volatile s32_t tcpipdone = 0;
	uint32_t physts;
	static int prt_ip = 0;
	
	DEBUGSTR("LWIP HTTP Web Server FreeRTOS Demo...\r\n");

	/* Wait until the TCP/IP thread is finished before
	   continuing or wierd things may happen */
	DEBUGSTR("Waiting for TCPIP thread to initialize...\r\n");
	tcpip_init(tcpip_init_done_signal, (void *) &tcpipdone);
	while (!tcpipdone) {
		msDelay(1);
	}

	DEBUGSTR("Starting LWIP HTTP server...\r\n");

	/* Static IP assignment */
#if LWIP_DHCP
	IP4_ADDR(&gw, 0, 0, 0, 0);
	IP4_ADDR(&ipaddr, 0, 0, 0, 0);
	IP4_ADDR(&netmask, 0, 0, 0, 0);
#else
	IP4_ADDR(&gw, 192, 168, 1, 1);
	IP4_ADDR(&ipaddr, 192, 168, 1, 10);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
#endif

	/* Add netif interface for lpc17xx_8x */
	memset(&lpc_netif, 0, sizeof(lpc_netif));
	if (!netif_add(&lpc_netif, &ipaddr, &netmask, &gw, NULL, lpc_enetif_init,
				   tcpip_input)) {
		DEBUGSTR("Net interface failed to initialize\r\n");
		while(1);			   
	}
	netif_set_default(&lpc_netif);
	netif_set_up(&lpc_netif);

	/* Enable MAC interrupts only after LWIP is ready */
	NVIC_SetPriority(ETHERNET_IRQn, config_ETHERNET_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(ETHERNET_IRQn);

#if LWIP_DHCP
	dhcp_start(&lpc_netif);
#endif
	
	/* Initialize and start application */
	http_server_netconn_init();
	
	/* This loop monitors the PHY link and will handle cable events
	   via the PHY driver. */
	while (1) {
		/* Call the PHY status update state machine once in a while
		   to keep the link status up-to-date */
		physts = lpcPHYStsPoll();

		/* Only check for connection state when the PHY status has changed */
		if (physts & PHY_LINK_CHANGED) {
			if (physts & PHY_LINK_CONNECTED) {
				Board_LED_Set(0, true);
				prt_ip = 0;

				/* Set interface speed and duplex */
				if (physts & PHY_LINK_SPEED100) {
					Chip_ENET_Set100Mbps(LPC_ETHERNET);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 100000000);
				}
				else {
					Chip_ENET_Set10Mbps(LPC_ETHERNET);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 10000000);
				}
				if (physts & PHY_LINK_FULLDUPLX) {
					Chip_ENET_SetFullDuplex(LPC_ETHERNET);
				}
				else {
					Chip_ENET_SetHalfDuplex(LPC_ETHERNET);
				}

				tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_up,
										  (void *) &lpc_netif, 1);
			}
			else {
				Board_LED_Set(0, false);
				tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_down,
										  (void *) &lpc_netif, 1);
			}

			/* Delay for link detection (250mS) */
			vTaskDelay(configTICK_RATE_HZ / 4);
		}

		/* Print IP address info */
		if (!prt_ip) {
			if (lpc_netif.ip_addr.addr) {
				static char tmp_buff[16];
				DEBUGOUT("IP_ADDR    : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.ip_addr, tmp_buff, 16));
				DEBUGOUT("NET_MASK   : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.netmask, tmp_buff, 16));
				DEBUGOUT("GATEWAY_IP : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.gw, tmp_buff, 16));
				prt_ip = 1;
			}
		}
	}
}

void initSSP();

static SSP_ConfigFormat ssp_format;

//typedef struct {
//	unsigned motNum : 4;
//	unsigned div : 10;
//	unsigned dir : 1;
//	unsigned ena : 1;
//} TMotCtrl;





/**
 * @brief	MilliSecond delay function based on FreeRTOS
 * @param	ms	: Number of milliSeconds to delay
 * @return	Nothing
 * Needed for some functions, do not use prior to FreeRTOS running
 */
void msDelay(uint32_t ms)
{
	vTaskDelay((configTICK_RATE_HZ * ms) / 1000);
}

void uartInit();
//void SystemReInit (void);
void enetPinsInit();

int main(void)
{

	//SystemReInit();
	prvSetupHardware();
	uartInit();
	enetPinsInit();
	Board_SSP_Init(LPC_SSP0);
	Chip_SSP_Init(LPC_SSP0);
	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_BITS_16;
	ssp_format.clockMode = SSP_CLOCK_MODE0;
	Chip_SSP_SetFormat(LPC_SSP0, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_Enable(LPC_SSP0);
	Chip_SSP_SetMaster(LPC_SSP0, 1);

	printf("sysclk %d\r\n", Chip_Clock_GetSystemClockRate());

//	xTaskCreate(vSetupIFTask, (signed char *) "SetupIFx",
//				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
//				(xTaskHandle *) NULL);
//	xTaskCreate(vCPLDctrl, (signed char *) "vCPLDctrl",
//				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
//				(xTaskHandle *) NULL);

	xTaskCreate(vUartctrl, (signed char *) "vUartctrl",
				configMINIMAL_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}


void enetPinsInit()
{
	//DEBUGSTR("11\r\n");
	//Chip_ENET_Init(LPC_ETHERNET, true);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_ENET);

	Chip_IOCON_PinMux(LPC_IOCON, 1, 16, IOCON_MODE_INACT, IOCON_FUNC1); //ENET_MDC
	Chip_IOCON_PinMux(LPC_IOCON, 1, 17, IOCON_MODE_INACT, IOCON_FUNC1); //ENET_MDIO

	Chip_IOCON_PinMux(LPC_IOCON, 1, 0, IOCON_MODE_INACT, IOCON_FUNC1); //ENET_TXD0
	Chip_IOCON_PinMux(LPC_IOCON, 1, 1, IOCON_MODE_INACT, IOCON_FUNC1); //ENET_TXD1
	Chip_IOCON_PinMux(LPC_IOCON, 1, 9, IOCON_MODE_INACT, IOCON_FUNC1); //ENET_RXD0
	Chip_IOCON_PinMux(LPC_IOCON, 1, 10,IOCON_MODE_INACT, IOCON_FUNC1); //ENET_RXD1

	Chip_IOCON_PinMux(LPC_IOCON, 1, 4, IOCON_MODE_INACT, IOCON_FUNC1);  //ENET_TX_EN
	Chip_IOCON_PinMux(LPC_IOCON, 1, 14,IOCON_MODE_INACT, IOCON_FUNC1); //ENET_RX_ER
	Chip_IOCON_PinMux(LPC_IOCON, 1, 8, IOCON_MODE_INACT, IOCON_FUNC1);  //ENET_CRS


	Chip_IOCON_PinMux(LPC_IOCON, 1, 15, IOCON_MODE_INACT, IOCON_FUNC1); //ENET_REF_CLK
	//Chip_IOCON_PinMux(LPC_IOCON, 1, 27, IOCON_MODE_INACT, IOCON_FUNC1); //ENET_REF_CLK
}
/* Transmit and receive ring buffers */
RINGBUFF_T txring, uartRxRb;

/* Transmit and receive ring buffer sizes */
#define UART_RRB_SIZE 2048	/* Receive */
/* Transmit and receive buffers */
static uint8_t uartRxBuff[UART_RRB_SIZE];


void uartInit()
{
	Chip_UART_Init(LPC_UART0);
	Chip_UART_SetBaud(LPC_UART0, 115200);
	Chip_UART_ConfigData(LPC_UART0, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);

	Chip_UART_TXEnable(LPC_UART0);

	Chip_IOCON_PinMux(LPC_IOCON, 0, 2, IOCON_MODE_INACT, IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 3, IOCON_MODE_INACT, IOCON_FUNC1);

	RingBuffer_Init(&uartRxRb, uartRxBuff, 1, UART_RRB_SIZE);

	Chip_UART_SetupFIFOS(LPC_UART0, (UART_FCR_FIFO_EN | UART_FCR_RX_RS |
							UART_FCR_TX_RS | UART_FCR_TRG_LEV3));
	Chip_UART_IntEnable(LPC_UART0, (UART_IER_RBRINT | UART_IER_RLSINT));


	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);
}

void initSSP()
{
	Chip_IOCON_PinMux(LPC_IOCON, 0, 15, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 16, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 17, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 18, IOCON_MODE_INACT, IOCON_FUNC2);
}



void UART0_IRQHandler(void)
{
	Chip_UART_IRQRBHandler(LPC_UART0, &uartRxRb, NULL);
}

//void ETH_IRQHandler(void)
//{
//
//}

