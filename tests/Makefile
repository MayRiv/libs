COMPILER=g++ -m32
LINKER=g++ -m32
make=gmake

OS_TYPE=$(shell uname)
LD=..
ifndef BSD
  ifeq ($(OS_TYPE),FreeBSD)
    BSD=yes
  endif
endif

ifndef LINUX
  ifeq ($(OS_TYPE),Linux)
    LINUX=yes
  endif
endif

ifdef BSD
LIBS=-lmysqlclient -lbz2 -pthread -lm -liconv
LIBDIRS=-L/usr/local/lib/mysql -L/usr/local/lib
CPPFLAGS=-DBSD -D_HTTP_WITH_POST=1 -D_XOPEN_SOURCE=600 -D_BSD_SOURCE=1 -D_THREAD_SAFE
INCLUDEDIRS=-I/usr/local/include/mysql -I$(LD) -I/usr/local/include
endif


ifdef LINUX
LIBS=-lpthread -lmysqlclient 
LIBDIRS=-L/usr/local/lib -L/usr/lib/mysql
INCLUDEDIRS=-I/usr/include/mysql -I$(LD) -I./
CPPFLAGS=-DLINUX -D_HTTP_WITH_POST=1 -D_XOPEN_SOURCE=600 -D_BSD_SOURCE=1 -D_GNU_SOURCE=1 -D_THREAD_SAFE
endif
HOSTNAME=$(shell hostname -s)
ifndef DEV
    ifeq ($(HOSTNAME),dev)
        DEV=yes
    endif
endif

ifdef DEV
CPPFLAGS+=-D_DEV
endif



LINK=$(LINKER)
LOBJ=$(LD)/ips.o $(LD)/socket.o $(LD)/icharset.o $(LD)/log.o $(LD)/string.o $(LD)/mutexsync.o $(LD)/mysql.o $(LD)/tools.o \
$(LD)/thread.o $(LD)/storage.o $(LD)/time.o $(LD)/dir.o $(LD)/chars.o $(LD)/charset.o $(LD)/limit.o $(LD)/dfile.o 


ifdef GPROF
GPROF_FLAGS=-pg
endif

ifdef OPT
OP= -g -O3
else
OP= -g
endif

CC=$(COMPILER) $(INCLUDEDIRS) -MD -fexceptions ${OP}   -Wall -D_REENTRANT -D_DEBUG $(GPROF_FLAGS)
LF=$(GPROF_FLAGS)
#-D_DEBUG
#CC=$(COMPILER) $(INCLUDEDIRS) -MD -fno-rtti -O3 -Wall -D_REENTRANT


LIBS+=-lboost_unit_test_framework-mt

TEST_OBJ=test.cpp string_test.o  $(LOBJ)

PROGRAMS=$(BIN_PROGRAMS) $(TEST_PROGRAMS)

all: test
	./test

test: $(TEST_OBJ)
	$(LINKER) $(OP) -o test $(TEST_OBJ) $(LIBDIRS) $(LIBS)
	
%.o: %.cpp
	$(CC) $(OP) -c $< -o $@ $(CPPFLAGS) $(LOGFLAGS)

 
include $(wildcard *.d)

clean::
	find . -name "*.core" -exec rm {} \;
	find . -name "*.d" -exec rm {} \;
	find . -name "*.o" -exec rm {} \;
	find .. -name "*.o" -exec rm {} \;
	find .. -name "*.gch" -exec rm {} \;
	
update::
	cd ..;git pull origin master

