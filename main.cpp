/*
 * Copyright (c) 2021 Sandeep Mistry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stdlib.h"
#include "pico/stdio.h"
#include "pico/multicore.h"

#include "hardware/clocks.h"

#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/udp.h"


extern "C" {
#include "rmii_ethernet/netif.h"
}

#include "rpNeoPixel.h"

static const u16_t UDP_LED_payload_length = NUM_PIXELS * COLORS_PER_PIXEL;
static const u16_t UDP_LED_port = 5000;


static rpNeoPixel obLEDs;

static void netif_link_callback(struct netif *netif)
{
    printf("netif link status changed %s\n", netif_is_link_up(netif) ? "up" : "down");
}

static void netif_status_callback(struct netif *netif)
{
    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

static void LEDMessageReceived(const u8_t * const pData, const u16_t Length)
{
    printf("LED Data : ");
    for (int i = 0; i < Length; i++)
    {
		  printf("%02X", pData[i]);
    }
    printf("\n");

    for(int i = 0; i < UDP_LED_payload_length; i += COLORS_PER_PIXEL)
    {
		  obLEDs.setColor((i / COLORS_PER_PIXEL), pData[i], pData[i + 1], pData[i + 2]);
    }
    obLEDs.process();
}


static void udp_raw_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	LWIP_UNUSED_ARG(arg);

	if((NULL != pcb) && (NULL != p))
	{
		u16_t packetLength = p->len;

		if(UDP_LED_payload_length == packetLength)
		{
			LEDMessageReceived((const u8_t *)p->payload, packetLength);
		}

		pbuf_free(p);
	}
}


int main() {
    // LWIP network interface
	struct netif netif;
	struct udp_pcb *pcb = NULL;

 //
 struct netif_rmii_ethernet_config netif_config = {
	  pio0, // PIO:            0
	  0,    // pio SM:         0 and 1
	  6,    // rx pin start:   6, 7, 8    => RX0, RX1, CRS
	  10,   // tx pin start:   10, 11, 12 => TX0, TX1, TX-EN
	  14,   // mdio pin start: 14, 15   => ?MDIO, MDC
	  NULL, // MAC address (optional - NULL generates one based on flash id)
 };

#if !defined(_LED_ONLY_TEST)
    // change the system clock to use the RMII reference clock from pin 20
    clock_configure_gpin(clk_sys, 20, 50 * MHZ, 50 * MHZ);
    sleep_ms(100);
#endif	//_LED_ONLY_TEST

    // initialize stdio after the clock change
    stdio_init_all();


    int iColor=0;


    gpio_set_function(28,GPIO_FUNC_SIO);
    gpio_set_dir(28, GPIO_OUT);
    gpio_put(28,1);

    gpio_set_function(28,GPIO_FUNC_PIO1);
    obLEDs.init(28,1,0,true);

    sleep_ms(5000);

    printf("pico rmii ethernet meetup3 - initialize RMII ethernet interface\n");

    // initilize LWIP in NO SYS mode
    lwip_init();

    // initialize the PIO base RMII Ethernet network interface
    netif_rmii_ethernet_init(&netif, &netif_config);

    // assign callbacks for link and status
    netif_set_link_callback(&netif, netif_link_callback);
    netif_set_status_callback(&netif, netif_status_callback);

    // set the default interface and bring it up
    netif_set_default(&netif);
    netif_set_up(&netif);

#if defined(_USE_DHCP)
    printf("pico rmii ethernet meetup3 - start dhcp\n");
    // Start DHCP client and httpd
    dhcp_start(&netif);

#else
	ip4_addr_t ip_addr, net_submask, default_gateway;
	char ip_addr_string[IPADDR_STRLEN_MAX + 1];

	printf("pico rmii ethernet meetup3 - setup static IP address\n");

	IP4_ADDR(&ip_addr, 192, 168, 111, 100);
	IP4_ADDR(&net_submask, 255, 255, 255, 0);
	IP4_ADDR(&default_gateway, 192, 168, 111, 1);

	ipaddr_ntoa_r(&ip_addr, ip_addr_string, sizeof(ip_addr_string));
	printf("Setting IP address to %s\n", ip_addr_string);
	ipaddr_ntoa_r(&net_submask, ip_addr_string, sizeof(ip_addr_string));
	printf("Setting subnet mask to %s\n", ip_addr_string);
	ipaddr_ntoa_r(&default_gateway, ip_addr_string, sizeof(ip_addr_string));
	printf("Setting default gateway to %s\n", ip_addr_string);

	netif_set_addr(&netif, &ip_addr, &net_submask, &default_gateway);

#endif	//(_USE_DHCP)

#if defined(_LED_ONLY_TEST)
	printf("pico rmii ethernet meetup3 - running in LED only mode\n");
#else
	printf("pico rmii ethernet meetup3 - setup UDP receive\n");

	pcb = udp_new();
	udp_bind(pcb, IP_ADDR_ANY, UDP_LED_port);
	udp_recv(pcb, udp_raw_recv_callback, pcb);

	printf("pico rmii ethernet meetup3 - running\n");

	// Note: this function call never returns - LED updates are handled via udp_raw_receive_callback()
	netif_rmii_ethernet_loop();
#endif	//_LED_ONLY_TEST

    while (1) {
#if defined(_LED_ONLY_TEST)
		sleep_ms(100);
		obLEDs.setColor(0,iColor, iColor, iColor);


		obLEDs.setColor(1,0, iColor, 0);
		obLEDs.setColor(2,0, 0, 0);
		obLEDs.setColor(3,0xaa, 0x55, 0);

		if (iColor==255)
			iColor = 0;
		else
		{
			iColor++;
		}

		obLEDs.process();
#else
		tight_loop_contents();
#endif	//(_LED_ONLY_TEST)

    }

    return 0;
}
