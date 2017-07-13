#
# Main Makefile for broadcast snapshot server project!
#  (C) Deng zhaoqi 2013.3.30
#

PROJDIR = .
CC		=  gcc
CPP	    =  g++

INCSDIR = /usr/include/mysql
CC_SOURCE_FILES  += 	$(wildcard *.c)
CC_OBJECTS 		= 	$(patsubst %.c, %.o, $(CC_SOURCE_FILES))

CPP_SOURCE_FILES 	= 	$(wildcard *.cpp)
CPP_OBJECTS 		= 	$(patsubst %.cpp, %.o, $(CPP_SOURCE_FILES))
CPP_FLAGS 		=   -I/opt/ge730c_server/ist/boost_1_55_0 -I$(INCSDIR) 
CPP_LDFLAGS     =	-lpthread  -lrt  -ldl -lm  \
					-L/opt/ge730c_server/ist/boost_1_55_0/prefix/lib  \
    				-lboost_system \
    				-lboost_thread    \
    				-L/usr/lib/mysql -lmysqlclient \
					-L/usr/lib64/mysql -lmysqlclient  \

xts: $(CPP_OBJECTS) $(CC_OBJECTS)
	$(CPP) $(CPP_FLAGS) $^  $(CPP_LDFLAGS) -o $@ 
	
%.o: %.c
	$(CC) -c $(CPP_FLAGS) -std=gnu99 -o $@ $^
%.o: %.cpp
	$(CPP)  -c $(CPP_FLAGS) -o $@ $^

clean:
	rm -f *.o xts
