/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   Network connection configuration
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dht11.h"

/* Private typedef -----------------------------------------------------------*/
#define MAX_DHCP_TRIES        4

/* Private define ------------------------------------------------------------*/
typedef enum 
{ 
  DHCP_START=0,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT
} 
DHCP_State_TypeDef;
struct http_state
{
	int state; //0:OK 1:Others
  char *body;
};
struct data
{
	char ip[20];
	int temp;
	int humid;
};
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif netif;
uint32_t TCPTimer = 0;
uint32_t ARPTimer = 0;
uint32_t IPaddress = 0;
struct tcp_pcb *testpcb;
err_t error;
struct data data;
uint32_t UpdateTimer = 0;
uint16_t sent_count = 0;
struct ip_addr server_ip;

#ifdef USE_DHCP
uint32_t DHCPfineTimer = 0;
uint32_t DHCPcoarseTimer = 0;
DHCP_State_TypeDef DHCP_state = DHCP_START;
#endif

/* Private functions ---------------------------------------------------------*/
void LwIP_DHCP_Process_Handle(void);
err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void domainFound(const char *, struct ip_addr *, void *);
void update_sensors(void);
void connect_close(struct tcp_pcb *);

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
#ifndef USE_DHCP
  uint8_t iptab[4];
  uint8_t iptxt[20];
#endif

  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();

#ifdef USE_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else
  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

#ifdef USE_LCD  
   iptab[0] = IP_ADDR3;
   iptab[1] = IP_ADDR2;
   iptab[2] = IP_ADDR1;
   iptab[3] = IP_ADDR0;

   sprintf((char*)iptxt, "  %d.%d.%d.%d", iptab[3], iptab[2], iptab[1], iptab[0]); 
      
   LCD_DisplayStringLine(Line8, (uint8_t*)"  Static IP address   ");
   LCD_DisplayStringLine(Line9, iptxt);
#endif
#endif

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))
    
   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif);

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif);
}

/**
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
  ethernetif_input(&netif);
}

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{
#if LWIP_TCP
  /* TCP periodic process every 250 ms */
  if (localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  localtime;
    tcp_tmr();
  }
#endif
  
  /* ARP periodic process every 5s */
  if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  localtime;
    etharp_tmr();
  }

#ifdef USE_DHCP
  /* Fine DHCP periodic process every 500ms */
  if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  localtime;
    dhcp_fine_tmr();
    if ((DHCP_state != DHCP_ADDRESS_ASSIGNED)&&(DHCP_state != DHCP_TIMEOUT))
    { 
      /* toggle LED1 to indicate DHCP on-going process */
      STM_EVAL_LEDToggle(LED1);
      
      /* process DHCP state machine */
      LwIP_DHCP_Process_Handle();    
    }
  }

  /* DHCP Coarse periodic process every 60s */
  if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  localtime;
    dhcp_coarse_tmr();
  }
  
#endif
	
	if (localtime - UpdateTimer >= 60 * 1000UL)
  {
    UpdateTimer = localtime;
		update_sensors();
  }
}

void update_sensors(void)
{	
	uint8_t dht_data[5]; /* dht11 data container */
	uint8_t humidInt;
	uint8_t humidDec;
	uint8_t tempInt;
	uint8_t tempDec;
	uint8_t  aTextBuffer[50];
	do
	{
		if (dht11_read(dht_data)==DHT11_OK)
		{
			humidInt = dht_data[0];
			humidDec = dht_data[1];
			tempInt = dht_data[2];
			tempDec = dht_data[3];
			
			sprintf((char*)aTextBuffer,"   Tempture = %d.%d C" , tempInt, tempDec);
			LCD_DisplayStringLine(Line3, aTextBuffer);
			sprintf((char*)aTextBuffer,"   Humid = %d.%d %%" , humidInt, humidDec);
			LCD_DisplayStringLine(Line4, aTextBuffer);			
		}
	} while (tempInt == 0 || humidInt == 0 || tempInt > 50 || humidInt > 100);
	send_sensors(tempInt, humidInt);
}

#ifdef USE_DHCP
/**
  * @brief  LwIP_DHCP_Process_Handle
  * @param  None
  * @retval None
  */
void LwIP_DHCP_Process_Handle()
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  uint8_t iptab[4];
  uint8_t iptxt[20];

  switch (DHCP_state)
  {
    case DHCP_START:
    {
      dhcp_start(&netif);
      IPaddress = 0;
      DHCP_state = DHCP_WAIT_ADDRESS;
#ifdef USE_LCD
      LCD_DisplayStringLine(Line5, (uint8_t*)"     Looking for    ");
      LCD_DisplayStringLine(Line6, (uint8_t*)"     DHCP server    ");
      LCD_DisplayStringLine(Line7, (uint8_t*)"     please wait... ");
#endif
    }
    break;

    case DHCP_WAIT_ADDRESS:
    {
      /* Read the new IP address */
      IPaddress = netif.ip_addr.addr;

      if (IPaddress!=0) 
      {
        DHCP_state = DHCP_ADDRESS_ASSIGNED;	

        /* Stop DHCP */
        dhcp_stop(&netif);

#ifdef USE_LCD      
        iptab[0] = (uint8_t)(IPaddress >> 24);
        iptab[1] = (uint8_t)(IPaddress >> 16);
        iptab[2] = (uint8_t)(IPaddress >> 8);
        iptab[3] = (uint8_t)(IPaddress);

        sprintf((char*)iptxt, " %d.%d.%d.%d", iptab[3], iptab[2], iptab[1], iptab[0]);       

        LCD_ClearLine(Line4);
        LCD_ClearLine(Line5);
        LCD_ClearLine(Line6);

        /* Display the IP address */
        LCD_DisplayStringLine(Line7, (uint8_t*)"IP address assigned ");
        LCD_DisplayStringLine(Line8, (uint8_t*)"  by a DHCP server  ");
        LCD_DisplayStringLine(Line9, iptxt);
#endif
        STM_EVAL_LEDOn(LED1);
      }
      else
      {
        /* DHCP timeout */
        if (netif.dhcp->tries > MAX_DHCP_TRIES)
        {
          DHCP_state = DHCP_TIMEOUT;

          /* Stop DHCP */
          dhcp_stop(&netif);

          /* Static address used */
          IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
          IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
          IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
          netif_set_addr(&netif, &ipaddr , &netmask, &gw);

#ifdef USE_LCD   
          LCD_DisplayStringLine(Line7, (uint8_t*)"    DHCP timeout    ");

          iptab[0] = IP_ADDR3;
          iptab[1] = IP_ADDR2;
          iptab[2] = IP_ADDR1;
          iptab[3] = IP_ADDR0;

          sprintf((char*)iptxt, "  %d.%d.%d.%d", iptab[3], iptab[2], iptab[1], iptab[0]); 

          LCD_ClearLine(Line4);
          LCD_ClearLine(Line5);
          LCD_ClearLine(Line6);

          LCD_DisplayStringLine(Line8, (uint8_t*)"  Static IP address   ");
          LCD_DisplayStringLine(Line9, iptxt);         
#endif    
          STM_EVAL_LEDOn(LED1);
        }
      }
    }
    break;
    default: break;
  }
}
#endif

void connect_close(struct tcp_pcb *pcb)
{
   tcp_arg(pcb, NULL);
   tcp_sent(pcb, NULL);
   tcp_recv(pcb, NULL);
   tcp_close(pcb);
}

void DNS_Init()
{
		struct ip_addr ip;
	  dns_init();
	  dns_gethostbyname("dchome.sytes.net", &ip, domainFound, NULL);		
}

void send_sensors(int temp,int humid)
{
    /* create an ip */
    //IP4_ADDR(&ip, 192,168,0,24);    //IP of my PHP server
	
		/* clear screen */
		LCD_ClearLine(Line5);
		LCD_ClearLine(Line6);
		LCD_ClearLine(Line7);
	
		/* dummy data to pass to callbacks*/
		data.temp = temp;
		data.humid = humid;
	
    /* create the control block */
    testpcb = tcp_new();    //testpcb is a global struct tcp_pcb
                            // as defined by lwIP

    /* register callbacks with the pcb */
    tcp_recv(testpcb, tcpRecvCallback);
	
		/* new connect */
		LCD_DisplayStringLine(Line5,(uint8_t*)"  Connecting to server...");
		tcp_connect(testpcb, &server_ip, DEST_PORT, connectCallback);
}

/* connection established callback, err is unused and only return 0 */
err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    char string[100];
    uint32_t len = strlen(string);
		uint8_t text[20];
	
		sprintf(string, "GET /sprout-grow/push_sensor.php?temp=%d&humid=%d HTTP/1.1\r\nHost: %s\r\n\r\n", data.temp, data.humid, data.ip);

    /* push to buffer */
    if (tcp_write(testpcb, string, strlen(string), TCP_WRITE_FLAG_COPY)) {
				sprintf((char*)text, "  WRITE ERROR: Code: %d", error);
				LCD_DisplayStringLine(Line5,(uint8_t*)text);
        return 1;
    }

    /* now send */
    if (tcp_output(testpcb)) {
				sprintf((char*)text, "  SENT ERROR: Code: %d", error);
				LCD_DisplayStringLine(Line5,(uint8_t*)text);
        return 1;
    }
    return ERR_OK;
}

err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
		struct http_state hs;
		char* tdata;
		uint8_t text[20];
	
		/* clear screen */
		LCD_ClearLine(Line5);
		LCD_ClearLine(Line6);
		LCD_ClearLine(Line7);
	
		LCD_DisplayStringLine(Line5,(uint8_t*)"  Data recieved.");
	
    if (p == NULL || err != ERR_OK) {
				LCD_DisplayStringLine(Line6,(uint8_t*)"  Host closed.");
			  pbuf_free(p);
        connect_close(testpcb);
        return ERR_ABRT;
    } else {
			
				tcp_recved(testpcb, p->tot_len);
				tdata = p->payload;
			
				if (strstr(tdata, "200 OK") != NULL)
				{
					hs.state = 0;
					hs.body = strstr(tdata, "\r\n\r\n") + 4*sizeof(char);
					if(hs.body != NULL) {
						LCD_DisplayStringLine(Line6,(uint8_t*)"  HTTP OK");
						if( !strncmp(hs.body,"0",1) ) 
						{
							sent_count++;
						}
						sprintf((char*)text, "  Count:%u", sent_count);
						LCD_DisplayStringLine(Line7,text);
					} else {
						LCD_DisplayStringLine(Line7,(uint8_t*)"  Failed to load content.");
					}
				} else {
					hs.state = 1;
					LCD_DisplayStringLine(Line6,(uint8_t*)"  HTTP ERROR");
				}
				pbuf_free(p);
        connect_close(testpcb);
    }
    return ERR_OK;
}

void domainFound(const char *name, struct ip_addr *ipaddr, void *arg) {
		LCD_DisplayStringLine(Line5,(uint8_t*)"  Success to resolve domain!");
	  server_ip = *ipaddr;
		 sprintf(data.ip, "%u.%u.%u.%u", 
			ip4_addr1(ipaddr), 
			ip4_addr2(ipaddr), 
			ip4_addr3(ipaddr), 
			ip4_addr4(ipaddr));
	  /* now connect */
    //tcp_connect(testpcb, ipaddr, DEST_PORT, connectCallback);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
