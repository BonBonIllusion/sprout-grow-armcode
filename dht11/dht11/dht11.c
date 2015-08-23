/* simple delay based dht11 library */
/* author: Adam Orcholski, www.tath.eu, tath@o2.pl */

#include "dht11.h"

/* dht11_read */
int dht11_read(uint8_t* p)
{
  int i, j, exitCode = DHT11_OK;
	
	#ifdef ENABLE_TIMEOUTS
	int timeout=TIMEOUT_VALUE;
	#endif
	
	CRITICAL_SECTION_DECL;
	
	ENABLE_TIMER;

	SET_GPIO_AS_OUTPUT;
	
	CRITICAL_SECTION_PROTECT;
	
	/* start sequence */
	GPIO_CLEAR;	
	DELAY_US(18000);

	GPIO_SET;
	DELAY_US(40);

	SET_GPIO_AS_INPUT;

	//timeout handler
	while(GPIO_GET_INPUT==0) /* 80us on '0' */
	{
		#ifdef ENABLE_TIMEOUTS
		if (--(timeout) <= 0)
		{
			exitCode = DHT11_TIMEOUT;
			break;
		}
		#endif
	};
	
	#ifdef ENABLE_TIMEOUTS
	timeout = TIMEOUT_VALUE;
	#endif
	if (exitCode == DHT11_OK)
		while(GPIO_GET_INPUT==1) /* 80us on '1' */
		{
			#ifdef ENABLE_TIMEOUTS
			if (--(timeout) <= 0)
			{
				exitCode = DHT11_TIMEOUT;
				break;
			}
			#endif
		};		
	/* start sequence - end */

	/* read sequence */
	if (exitCode == DHT11_OK)
		for(j=0;j<5;j++)
		{
			for(i=0;i<8;i++)
			{
				#ifdef ENABLE_TIMEOUTS
				timeout = TIMEOUT_VALUE;
				#endif
				while(GPIO_GET_INPUT==0)
				{
					#ifdef ENABLE_TIMEOUTS
					if (--(timeout) <= 0)
					{
						exitCode = DHT11_TIMEOUT;
						break;
					}
					#endif
				}; /* 50 us on 0 */

				if (GPIO_GET_INPUT==1)
					DELAY_US(30);

				p[j] <<= 1;
				
				if(GPIO_GET_INPUT==1)
				{
					DELAY_US(40); /* wait 'till 70us */
					p[j] |= 1;
				}
				else
				{
					p[j] &= 0xfe;
				}
			}
		}
	/* read sequence - end */
		
	CRITICAL_SECTION_UNPROTECT;
	DISABLE_TIMER;

	/* checksum check */
	if (exitCode == DHT11_OK)
	{
		if ((p[0] + p[2]) != p[4])
			exitCode = DHT11_WRONG_CHCKSUM;
		else
			exitCode = DHT11_OK;
	}

	return exitCode;
}
