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

#include "lwip/apps/httpd.h"

extern "C" {
#include "rmii_ethernet/netif.h"
}

//#include "rpPIO.h"
#include "rpNeoPixel.h"

//extern void netif_rmii_ethernet_loop();
rpNeoPixel obLEDs;

void netif_link_callback(struct netif *netif)
{
    printf("netif link status changed %s\n", netif_is_link_up(netif) ? "up" : "down");
}

void netif_status_callback(struct netif *netif)
{
    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

extern "C" {
    void LEDMessageReceived(unsigned char * pData, int iLength)
    {
        printf("LED Data : ");
        for (int i = 0; i < iLength; i++)
        {

            printf("%02X", pData[i]);
        }
        printf("\n");

        for (int i = 0; i < 12; i+=3)
        {

            obLEDs.setColor(i/3,pData[i], pData[i+1], pData[i+2]);
        }
        obLEDs.process();
    }
}

int main() {
    // LWIP network interface
    struct netif netif;

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

    printf("pico rmii ethernet - httpd\n");

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
    printf("pico rmii ethernet - start dhcp\n");
    // Start DHCP client and httpd
    dhcp_start(&netif);

#else
	ip4_addr_t ip_addr, net_submask, default_gateway;

	IP4_ADDR(&ip_addr, 192, 168, 111, 100);
	IP4_ADDR(&net_submask, 255, 255, 255, 0);
	IP4_ADDR(&default_gateway, 192, 168, 111, 1);

	netif_set_addr(&netif, &ip_addr, &net_submask, &default_gateway);

#endif	//(_USE_DHCP)

    httpd_init();

    // setup core 1 to monitor the RMII ethernet interface
    // this let's core 0 do other things :)
    //multicore_launch_core1(netif_rmii_ethernet_loop);

    printf("pico rmii ethernet - running\n");

#if !defined(_LED_ONLY_TEST)
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
