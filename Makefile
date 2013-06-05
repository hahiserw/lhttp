PROGRAM=lhttpd


CC=gcc
CFLAGS=-I.

LIBS=-lpthread


ODIR=obj


OBJ=main.o lsocket.o lrequest.o lresponse.o
DEPS:=$(wildcard *.h)
#OBJ=$($DEPS, %.c, %o)


CONFIG_FILE=lhttpd.conf

CONFIG_DIR=/etc
INTSTALL_DIR=/usr/bin



all: $(PROGRAM)
	@echo "Skompilowane! :D"

$(PROGRAM): $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

# $(ODIR)/
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<


.PHONY: clean

clean:
	rm $(PROGRAM) $(OBJ)


# install: $(PROGRAM) $(CONFIG_FILE)
# 	cp $(PROGRAM) $(INTSTALL_DIR)
# 	cp $(CONFIG_FILE) $(CONFIG_DIR)

# remove:
# 	rm $(INTSTALL_DIR)/$(PROGRAM)
# 	rm $(CONFIG_DIR)/$(CONFIG_FILE)
