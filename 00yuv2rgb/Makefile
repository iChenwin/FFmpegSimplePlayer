all:yuv2rgb
CC=gcc
CLIBSFLAGS=-lavformat -lavcodec -lavutil -lswresample -lswscale -lz -lpthread -lm
FFMPEG=/usr/local
CFLAGS=-I$(FFMPEG)/include/
LDFLAGS = -L$(FFMPEG)/lib/
yuv2rgb:yuv2rgb.o
	$(CC) -o yuv2rgb yuv2rgb.o $(CLIBSFLAGS) $(CFLAGS) $(LDFLAGS)
yuv2rgb.o:yuv2rgb.c
	$(CC) -o yuv2rgb.o -c yuv2rgb.c  $(CLIBSFLAGS) $(CFLAGS) $(LDFLAGS)
clean:
	rm yuv2rgb yuv2rgb.o
