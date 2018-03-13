#可用工程库
##->SSD1306 Clock
工程：**raspberrypi_oled_clock**

注意:fonts是目前的字库，1206，1608是完整的ASCII码表，1616，3216目前只有数字。没有汉字码表。
没有使用wiringPi库，而是使用了标准的linux I2C驱动，ioctl. 
可以画位图。
gcc main.c ssd1306.c fonts.c  -o oled

##->DHT11+OLED
工程：**dht_oled**

gcc main.c ssd1306.c fonts.c  dht11.c  -lwiringPi -o oled

##->DHT11纯驱动库
工程：**dht11**

##->pcf8563
工程：**pcf8563**

读取pcf中的时分秒，并显示

##->led
工程：**led**

使用wiringPi库来控制LED的亮灭。27-blue，28-green，29-red

 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | OUT  | GPIO.27 | 27  | 16  |

 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | OUT  | GPIO.28 | 28  | 20  |

 |     |     |      0v |      |   | 39 || 40 | 0 | OUT  | GPIO.29 | 29  | 21  |

 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+

 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |


##MQTT
工程：**paho.mqtt.embedded-c**

paho.mqtt.embedded-c\MQTTPacket

编译主要使用了上述文件夹，测试用例可参考上述文件夹中的sample，paho.mqtt.embedded-c\MQTTPacket\samples\pub0sub1.c
目前已修改为可以订阅一个topic，并且使用该topic发送payload
使用test.mosquitto.org/作为MQTT broker


工程：**MQTTPacket-onenet-dp-test**

onenet MQTT datapoint 测试用例，使用类型3，简单上传固定温度和湿度。

工程：**MQTTPacket-onenet-dht**

onenet MQTT datapoint 测试用例，使用类型3，上传温度和湿度。并使用OLED显示

工程：**MQTTPacket-onenet-dht-thread**

onenet MQTT datapoint 测试用例，使用类型3，上传温度和湿度。并使用OLED显示。利用linux多线程技术，保证时间信息显示正确。

同时，该工程处理了onenet的命令回复，reply: TopicName=”$crsp/cmduuid”  
