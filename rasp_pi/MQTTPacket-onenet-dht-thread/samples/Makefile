CC=gcc
AR=ar
LD=ld

CFLAGS= -I ../src
LDFLAGS= -lpaho-embed-mqtt3c -lwiringPi -lpthread


SOURCES_CLIBS =  \
	pub0sub1.c  \
	transport.c \
	ssd1306.c \
	fonts.c \
	dht11.c \
	oled_display.c

SOURCES = $(SOURCES_CLIBS)
OBJS := $(SOURCES:.c=.o)

%.o : %.c 
	$(CC) $(CFLAGS) -c $^ -o $@

TARGETS = pub0sub1

all: $(TARGETS)
	echo C Files: $(SOURCES)
	echo OBJS: $(OBJS)
	echo $(TARGETS)


pub0sub1: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@


clean:
	rm -rf  $(OBJS)  $(TARGETS)