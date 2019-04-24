all:helloFF
CC=gcc
CLIBSFLAGS=-lavformat -lavcodec -lavutil -lswresample -lz -lpthread -lm
FFMPEG=/usr/local
CFLAGS=-I$(FFMPEG)/include/
LDFLAGS = -L$(FFMPEG)/lib/
helloFF:helloFF.o
	$(CC) -o helloFF helloFF.o $(CLIBSFLAGS) $(CFLAGS) $(LDFLAGS)
helloFF.o:testFFmpeg.c
	$(CC) -o helloFF.o -c testFFmpeg.c  $(CLIBSFLAGS) $(CFLAGS) $(LDFLAGS)
clean:
	rm helloFF helloFF.o
