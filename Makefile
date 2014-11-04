CC=gcc
# CC=clang
CFLAGS = -std=c99
CFLAGS += -Wall 
CFLAGS += -pedantic

all: stable

debug:
	$(CC) $(CFLAGS) -g -fPIC -shared -lrt -o libinterface_msg.so interface_msg.c
	$(CC) $(CFLAGS) -g -fPIC -shared -o libinterface_fifo.so base64.c interface_fifo.c
	$(CC) $(CFLAGS) -g -ldl -o client interface_dl.c game.c visualize.c client.c
	$(CC) $(CFLAGS) -g -ldl -o server interface_dl.c game.c visualize.c server.c
stable:
	$(CC) $(CFLAGS) -fPIC -shared -lrt -o libinterface_msg.so interface_msg.c
	$(CC) $(CFLAGS) -fPIC -shared -o libinterface_fifo.so base64.c interface_fifo.c
	$(CC) $(CFLAGS) -ldl -o client interface_dl.c game.c visualize.c client.c
	$(CC) $(CFLAGS) -ldl -o server interface_dl.c game.c visualize.c server.c

docs:
	doxygen ./Doxyfile

clean:
	rm -vfr *~ client
	rm -vfr *~ server
	rm -vfr libinterface_msg.so
	rm -vfr libinterface_fifo.so 
	rm -vfr doxygen/*
