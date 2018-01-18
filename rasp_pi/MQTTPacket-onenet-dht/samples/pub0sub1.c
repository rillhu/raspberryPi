/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - clarifications and/or documentation extension
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>  
  
#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <sys/socket.h> 

#include <time.h>

#include "ssd1306.h"
#include "fonts.h"


#include "dht11.h"


#include "MQTTPacket.h"
#include "transport.h"

/* This is in order to get an asynchronous signal to stop the sample,
as the code loops waiting for msgs on the subscribed topic.
Your actual code will depend on your hw and approach*/
#include <signal.h>

int toStop = 0;

void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}

void stop_init(void)
{
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
}



int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd) {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, 16, "%s", inet_ntoa(sin.sin_addr));

    close(sd);

    return 0;
}

/* */

int main(int argc, char *argv[])
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	int mysock = 0;
	unsigned char buf[200];
	int buflen = sizeof(buf);
	int msgid = 1;
	MQTTString topicString = MQTTString_initializer;
	int req_qos = 0;
	char payload[128];// = "mypayload";
	int payloadlen;// = strlen(payload);
	int len = 0;
	//char *host = "open.iot.10086.cn";
	
	char *host = "183.230.40.39";
	int port = 6002;

	stop_init();
	if (argc > 1)
		host = argv[1];

	if (argc > 2)
		port = atoi(argv[2]);

	mysock = transport_open(host, port);
	if(mysock < 0)
		return mysock;

	printf("Sending to hostname %s port %d\n", host, port);

    data.struct_id[3] = 'T';
	data.clientID.cstring = "24813778";
	data.keepAliveInterval = 120;
	data.cleansession = 0;
	data.username.cstring = "118217";
	data.password.cstring = "kwKorRXQBlWebhHuLsD4E48Y7WU=";
	data.MQTTVersion = 4;
	
	len = MQTTSerialize_connect(buf, buflen, &data);
	rc = transport_sendPacketBuffer(mysock, buf, len);

	/* wait for connack */
	if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
	{
		unsigned char sessionPresent, connack_rc;

		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
		{
			printf("Unable to connect, return code %d\n", connack_rc);
			goto exit;
		}
	}
	else
		goto exit;
#if 0
	/* subscribe */
	topicString.cstring = "mytopic";
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);

	rc = transport_sendPacketBuffer(mysock, buf, len);
	if (MQTTPacket_read(buf, buflen, transport_getdata) == SUBACK) 	/* wait for suback */
	{
		unsigned short submsgid;
		int subcount;
		int granted_qos;

		rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);
		if (granted_qos != 0)
		{
			printf("granted qos != 0, %d\n", granted_qos);
			goto exit;
		}
	}
	else
		goto exit;
#endif

    time_t time_now;
    struct tm *time_local; 
    char time_string[32];
    char clock_string[16];
    char ip_string[16];  
    
    //const char *eth = "eth0";  
    const char *eth = "wlan0";  
    unsigned int i = 0;

    ssd1306_init();


	/* loop getting msgs on subscribed topic */
    unsigned int cnt_dp = 0;
	while (!toStop)
	{
        get_dht11();
        char str[64] = {0};
        memset(str, '\0',64);
        sprintf(str, "humi: %d%% temp: %d*C",dht_humi, dht_temp);
        get_local_ip(eth,ip_string);  
        time(&time_now);
        time_local = localtime(&time_now);
        sprintf(time_string, "%s", asctime(time_local));
        strncpy(clock_string, time_string + 11, 8);
        clock_string[8] = '\0';
        ssd1306_clear_screen(0x00);
        ssd1306_display_string(32-strlen(ip_string), 0, ip_string, FONT_1608, NORMAL);
        ssd1306_display_string(0, 16, clock_string, FONT_3216, NORMAL);            
        ssd1306_display_string(22-strlen(str), 48, str, FONT_1206, NORMAL);
        ssd1306_refresh_gram();
        
		/* transport_getdata() has a built-in 1 second timeout,
		your mileage will vary */
		if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBLISH)
		{
			unsigned char dup;
			int qos;
			unsigned char retained;
			unsigned short msgid;
			int payloadlen_in;
			unsigned char* payload_in;
			int rc;
			MQTTString receivedTopic;

			rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
					&payload_in, &payloadlen_in, buf, buflen);
			printf("message arrived %.*s\n", payloadlen_in, payload_in);
		}

		printf("publishing sending dp\n");
        topicString.cstring = "$dp";

        
        memset(payload,0,128);
		sprintf(payload,"{\"Temperature\":\"%d\", \"Humidity\":\"%d\"}",dht_temp,dht_humi);
        printf("payload: %s\n",payload);

        unsigned char *data = (char *)malloc(strlen(payload)+3);
        data[0] = 3; //data type
        data[1] = 0x00;
        data[2] = strlen(payload);
        memcpy(&data[3],payload,strlen(payload));
        //mqtt_publish(client, "$dp", data, strlen(out)+3, 0, 0);
		len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)data, strlen(payload)+3);
		rc = transport_sendPacketBuffer(mysock, buf, len);
        if(rc>0) cnt_dp++;
        
        free(data);
        data = NULL;
        usleep(100000);
        
	}

	printf("disconnecting\n");
	len = MQTTSerialize_disconnect(buf, buflen);
	rc = transport_sendPacketBuffer(mysock, buf, len);

exit:
	transport_close(mysock);

	return 0;
}
