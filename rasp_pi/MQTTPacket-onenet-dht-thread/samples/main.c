#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include<pthread.h>

#include "dht11.h"
#include "oled_display.h"
#include "pub0sub1.h"

int main(int argc, char *argv[])
{
    pthread_t dht_ids, oled_ids,pubsub_ids;
    int rc1 = pthread_create(&dht_ids, NULL, dht11_read_thread, NULL);
    if(rc1 != 0)    printf("%s: %d\n",__func__, (rc1));

    rc1 = pthread_create(&oled_ids, NULL, oled_display_thread, NULL);
    if(rc1 != 0)    printf("%s: %d\n",__func__, (rc1));

    rc1 = pthread_create(&pubsub_ids, NULL, pubsub_thread, NULL);
    if(rc1 != 0)    printf("%s: %d\n",__func__, (rc1));


    while(1){
        sleep(10);
    }
}

