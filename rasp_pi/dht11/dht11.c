#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define MAX_TIME 85
#define DHT11PIN 7
#define ATTEMPTS 500                 //retry 5 times when no response
int dht11_val[5]={0,0,0,0,0};
 
int dht11_read_val(){
    uint8_t lststate=HIGH;         //last state
    uint16_t counter=0;
    uint8_t j=0,i;
    for(i=0;i<5;i++)
        dht11_val[i]=0;
        
    //host send start signal    
    pinMode(DHT11PIN,OUTPUT);      //set pin to output 
    //digitalWrite(DHT11PIN,HIGH);   //set to high 250ms
    //delay(250);
    
    digitalWrite(DHT11PIN,LOW);    //set to low at least 18ms 
    delay(30);
    
    digitalWrite(DHT11PIN,HIGH);   //set to high 20-40us
    delayMicroseconds(40);
    
    //start recieve dht response
    pinMode(DHT11PIN,INPUT);       //set pin to input
    for(i=0;i<MAX_TIME;i++)         
    {
        counter=0;
        while(digitalRead(DHT11PIN)==lststate){     //read pin state to see if dht responsed. if dht always high for 255 + 1 times, break this while circle
            counter++;
            delayMicroseconds(1);
            if(counter==255)
                break;
        }
        
        delayMicroseconds(18); //delay 18~20us, not understand the root cause, guess the timing sequence.
        lststate=digitalRead(DHT11PIN);           //read current state and store as last state. 
        
        if(counter==255){                         //if dht always high for 255 + 1 times, break this for circle
            //printf("Time out wait dht11 response\n");
            break;
        }
        
        // top 3 transistions are ignored, maybe aim to wait for dht finish response signal
        if((i>=4)&&(i%2==0)){
            dht11_val[j/8]<<=1;                     //write 1 bit to 0 by moving left (auto add 0)
            if(counter>30)                          //long mean 1
                dht11_val[j/8]|=1;                  //write 1 bit to 1 
            j++;
        }
    }
    // verify checksum and print the verified data
    if((j>=39)&&(dht11_val[4]==((dht11_val[0]+dht11_val[1]+dht11_val[2]+dht11_val[3])& 0xFF))){
        printf("Humi:%d %%,TEMP:%d *C\n",dht11_val[0],dht11_val[2]);
        return 1;
    }
    else{
        printf("Check sum error\n");
        return 0;
      }
}
 
int main(void){
    int attempts=ATTEMPTS;
    if(wiringPiSetup()==-1)
        exit(1);
    while(attempts){                        //you have 5 times to retry
        printf("Try %d time\n",attempts);
        int success = dht11_read_val();     //get result including printing out
        if (success) {                      //if get result, quit program; if not, retry 5 times then quit
            //break;
        }
        attempts--;
        delay(2500);
    }
    return 0;
}
