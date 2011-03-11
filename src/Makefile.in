#!/bin/bash
# Makefile for giis package
#

VERSION="\"0.4.4\""
prefix ?=/usr
bindir = ${prefix}/bin
datadir =/giis
sysconfdir =/etc

FLAGS = -Wall -O2 -I/usr/include -DVERSION=${VERSION} 
CFLAGS  = -g
LDADD   = -lm
CC      = gcc ${FLAGS} 

bin_PROGRAMS = giis
giis_SOURCES = main.c  dir.c  file.c  get_it_i_say.c  group.c  init.c  inode.c  searchnupdate.c  uninit.c
giis_OBJS = ${giis_SOURCES:.c=.o}
giis_LDADD = ${LDADD}
giis_CFLAGS = ${CFLAGS}

.PHONY: clean explain

all : ${bin_PROGRAMS}
	@echo "All Done." 

${bin_PROGRAMS}: ${giis_OBJS}
	@echo -n "Linking $@ "
	@if ${CC} $^ -o $@ ${giis_LDADD} ;\
	then echo -e  "\tOK" ; \
	else echo -e  "\tFailed"; \
	fi;
	
%.o: %.c giis.h
	@echo -n "Compile $< "
	@if $(CC) $(giis_CFLAGS) -c $< ;\
	then echo -e  "\tOK" ; \
	else echo -e  "\tFailed"; \
	fi;

clean:
	@rm -fv *.o *~ ${bin_PROGRAMS}
	@echo "Clean Done."     

install:  ${bin_PROGRAMS}
	@install -v $^ ${bindir}
#	giis -i
#	@install -v -d /giis
#	@install -v -d /giis/sys
#	@install -v -d /giis/got_it
#	@install -v -d /giis/unrecovered
#	@install -v -b -m 0764 ../config/hai ${sysconfdir}/giis.conf
#	@install -v -b -m 0764 ../config/hai ${datadir}
#	@install -v -b -m 0764 ../config/quotes ${datadir}
#	@if wvar=`grep -c giis ${sysconfdir}/inittab` ; \
#	    then echo "config Found"; \
#	    else /bin/cat ../config/giis.config.in >> ${sysconfdir}/inittab ; \
#	 fi;
#	@if wvar=`grep -c giis ${sysconfdir}/crontab` ; \
#	    then echo "cron Found"; \
#	    else /bin/cat ../config/giis.daemon >> ${sysconfdir}/crontab ; \
#	 fi;	
	@echo "Install Done."

explain :
	@echo "Command Name : $(bin_PROGRAMS)" 
	@echo "Source       : $(giis_SOURCES)"
	@echo "Object       : $(giis_OBJS)"
