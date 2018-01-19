
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


#include <errno.h>  

#include<pthread.h>

#include <time.h>

#include "ssd1306.h"
#include "fonts.h"


#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <sys/socket.h> 


#include "dht11.h"
#include "pub0sub1.h"



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


/*
 * thread1() will be execute by thread1, after pthread_create()
 * it will set g_Flag = 1;
 */
void* oled_display_thread(void* arg)
{
	printf("this is oled thread, tid is %u\n", (unsigned int)pthread_self());

    time_t time_now;
    struct tm *time_local; 
    char time_string[32];
    char clock_string[16];
    char ip_string[16];  
    
    //const char *eth = "eth0";  
    const char *eth = "wlan0";      
    get_local_ip(eth,ip_string);  
    
    ssd1306_init();

    while(1){                               
        char str[64] = {0};
        memset(str, '\0',64);
        sprintf(str, "humi: %d%% temp: %d*C",dht_humi, dht_temp);

        time(&time_now);
        time_local = localtime(&time_now);
        sprintf(time_string, "%s", asctime(time_local));
        strncpy(clock_string, time_string + 11, 8);
        clock_string[8] = '\0';
        
        ssd1306_clear_screen(0x00);
        //line 1
        ssd1306_display_string(32-strlen(ip_string), 0, ip_string, FONT_1206, NORMAL);
        //line 2
        ssd1306_display_string(0, 12, clock_string, FONT_1616, NORMAL); 
        //line 3
        ssd1306_display_string(0, 28, str, FONT_1206, NORMAL);             
        //line 4
        memset(str, '\0',64);
        sprintf(str, "MQTTonenet pub: %d",cnt_dp);        
        ssd1306_display_string(0, 40, str, FONT_1206, NORMAL);           
        //line 5
        ssd1306_display_string(0, 52, pub_msg_get, FONT_1206, NORMAL); 
        
        ssd1306_refresh_gram();

        usleep(100000);
    }

	printf("leave thread1\n");
	pthread_exit(0);
}


