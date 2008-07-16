CC = gcc

#DEBUG=-ggdb
LIBP5DIR=/home/jason/src/libp5glove/trunk
INSTDIR=/usr/local/lib/pd/extra/

#INC = -I/usr/include
INC=-I$(LIBP5DIR)/src/
LIBS=-lp5glove

FLAGS= -Wall $(DEBUG) 

all: pd_p5

clean:
	-rm *.o p5.pd_linux

pd_p5: pd_p5.c
	$(CC) $(FLAGS) $(INC) -export_dynamic -shared -o p5.pd_linux \
		pd_p5.c $(LIBP5DIR)/src/p5glove.o $(LIBP5DIR)/src/usb_hid_libusb.o \
		-lc -lm -lusb
#	strip --strip-unneeded p5.pd_linux
	chmod 755 p5.pd_linux

install: pd_p5
	cp p5.pd_linux $(INSTDIR)
	
