CC = gcc
#DEBUG=-ggdb

#######################################################
# The following two dirs MUST be customized to your setup: 
LIBP5DIR=/your_path/to/libp5glove-20050129/trunk
PD_EXTERNALS_DIR=/your_path/to/pd-externals-cvs-20050129/externals
#######################################################

OSCDIR=$(PD_EXTERNALS_DIR)/OSCx
LIBOSCDIR=$(OSCDIR)/libOSC

LIBOSC=$(LIBOSCDIR)/libOSC.a

INC=-I$(LIBP5DIR)/src/ -I$(LIBOSCDIR)/ -I$(OSCDIR)/src

#LIBS=-lp5glove

FLAGS= -Wall $(DEBUG) 

all: p5osc 

clean:
	-rm *.o p5osc

p5osc: p5osc.c p5osc.h 
	$(CC) $(FLAGS) $(INC) -o p5osc p5osc.c \
		$(OSCDIR)/src/htmsocket.c \
		$(LIBP5DIR)/src/p5glove.o $(LIBP5DIR)/src/usb_hid_libusb.o \
		$(LIBOSC) \
		-lc -lm -lusb

#install: pd_p5
#	cp p5.pd_linux $(INSTDIR)
	
