#include <stdio.h>
#include <wiringPiI2C.h>

int main(void)
{
    int fd;
    int buf[10] = {0};
    fd = wiringPiI2CSetup(0x51);
    if(fd<0){
        printf("setup i2c error:%d\n", fd);
        return 0;
    }else{    
        printf("i2c setup successful %d\n",fd);
    }
    
    //buf[0] = wiringPiI2CRead(fd);
    //printf("read fd 0x%x\n",buf[0]);
    
    

    buf[0] = wiringPiI2CReadReg8(fd, 0x2);
    buf[1] = wiringPiI2CReadReg8(fd, 0x3);
    buf[2] = wiringPiI2CReadReg8(fd, 0x4);    
    printf("Current Time is %x:%x:%x\n",buf[2],buf[1],buf[0]);

    int tmp = buf[0];

    while(1){
        buf[0] = wiringPiI2CReadReg8(fd, 0x2);
        buf[1] = wiringPiI2CReadReg8(fd, 0x3);
        buf[2] = wiringPiI2CReadReg8(fd, 0x4);

        if(tmp==buf[0]){
            continue;
        }else{        
            tmp = buf[0];
            printf("Current Time is %x:%x:%x\n",buf[2],buf[1],buf[0]);
        }
    }
}
