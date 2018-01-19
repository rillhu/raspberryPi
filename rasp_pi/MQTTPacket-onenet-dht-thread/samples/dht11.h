#ifndef _DHT11_H_
#define _DHT11_H_

extern int dht_humi;
extern int dht_temp;
int get_dht11(void);
void* dht11_read_thread(void* arg);


#endif

