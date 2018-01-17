#  #可用工程库
##  ##->raspberrypi_oled_clock，
注意:fonts是目前的字库，1206，1608是完整的ASCII码表，1616，3216目前只有数字。没有汉字码表。
没有使用wiringPi库，而是使用了标准的linux I2C驱动，ioctl. 
可以画位图。
gcc main.c ssd1306.c fonts.c  -o oled

##  ##->DHT11 dislay
工程：dht_oled
gcc main.c ssd1306.c fonts.c  dht11.c  -lwiringPi -o oled

##  ##->DHT11纯驱动库
工程：dht11

##  ##->pcf8563
工程：pcf8563
读取pcf中的时分秒，并显示