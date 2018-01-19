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

#include<pthread.h>

#include "MQTTPacket.h"
#include "transport.h"

#include "dht11.h"
#include "oled_display.h"

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


unsigned int cnt_dp = 0;



/* */

int main(int argc, char *argv[])
{

    pthread_t dht_ids, oled_ids;
    int rc1 = pthread_create(&dht_ids, NULL, dht11_read_thread, NULL);
    if(rc1 != 0)    printf("%s: %d\n",__func__, (rc1));

    rc1 = pthread_create(&oled_ids, NULL, oled_display_thread, NULL);
    if(rc1 != 0)    printf("%s: %d\n",__func__, (rc1));
    

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





	/* loop getting msgs on subscribed topic */
	while (!toStop)
	{

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
