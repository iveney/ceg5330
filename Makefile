#----------------------------------------------------------------------
CC = g++

#MACHINE = sun4
MACHINE = linux
SIS = .
SISHDR = ./lib/sis_header

PSRC	= main.stdio.cpp sis_interface.cpp imply.cpp bnetwork.cpp rewire.cpp

POBJ	= $(PSRC:.cpp=.o)

PHDR	= igl.h debug.h array.h graph.h node.h edge.h bnetwork.h ignetwork.h imply.h rewire.h
MISC    = $(P).1 $(P).doc Makefile

TARGET	= rewire

LIBS	= $(SIS)/lib/libsis.a

XXLIBS	= $(SIS)/lib/libsis.a 

LINTLIBS= $(SIS)/lib/llib-lsis.ln	

INCLUDE	= -I$(SISHDR)
#INCLUDE	= -I$(SISHDR) -I/usr/local/mpatrol 
CFLAGS	= $(INCLUDE) -g
CXXFLAGS	= $(CFLAGS)
FFLAGS  = -g
LDFLAGS	= -ltermcap -lreadline -g
#LDFLAGS	= -g -ltermcap -lreadline -L/usr/local/lib -lmpatrol -lbfd -liberty
LINTFLAGS = $(INCLUDE)


#----------------------------------------------------------------------

$(TARGET): $(POBJ) $(LIBS) $(INC)
	$(CC) $(LDFLAGS) -o $(TARGET) $(POBJ) $(LIBS) -lm

#rewire/librewire.a: rewire/*.c rewire/*.h
#	cd rewire && make librewire.a && cd ..

%.o: %.cpp $(PHDR)
	$(CC) -c $< $(CXXFLAGS) -o $@

ctags:
	ctags -u $(PSRC)

clean:
	rm $(TARGET) *.o
