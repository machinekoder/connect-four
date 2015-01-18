### Makefile ---
## Copyright (c) Alexander Rössler and Christian Schwarzgruber
## Author: Christian Schwarzgruber
## Created: Fri Oct  3 19:20:46 2014 (+0200)
## Description: Makefile for the Connect Four application.
######################################################################
######################################################################
CC ?= gcc

RM=rm -rf
PROGRAM = c4

######################################################################
######################################################################
SRC = c4.c base64.c game.c interface_irda.c visualize.c
#SRC = c4.c base64.c game.c interface_fifo.c visualize.c
#SRC = c4.c base64.c game.c interface_msg.c visualize.c
INC = debug.h message.h interface.h

OBJ = $(SRC:.c=.o)
CFLAGS += -O0 -std=c99
######################################################################
######################################################################
.PHONY: clean uninstall install com all romfs

all: $(PROGRAM)

%.o: %.c $(INC)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

romfs:
	$(ROMFSINST) /bin/$(PROGRAM); \

doc:
	doxygen Doxyfile

clean:
	$(RM) $(OBJ) $(PROGRAM)

real_clean:
	$(RM) $(OBJ) $(PROGRAM) doc/

help:
	@echo -e "\
------------------------------------ Usage -----------------------------------\n\
make - to create the connectfour application.\n\
make clean - to remove all object fils etc.\n\
make doc - to generate doxygen documentation.\n\
make help - to print this help message and exit.\n\
--------------------------------------------------------------------------------"
