CC := g++
SRC := $(shell ls *.cc)
OBJ := $(patsubst %.cc, %.o, $(SRC))
INCPATH := 
CFLAG := -g -Wall $(INCPATH) -std=c++0x
LFLAG := -lpthread
TARGET := ./runner

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LFLAG) 

$(OBJ) : %.o : %.cc %.h global.h 
	$(CC) -c $(CFLAG) $< -o $@

run: 
	make && $(TARGET)

.PHONY: clean do_cleanall
	
clean : do_cleanobj
do_cleanall: do_cleanobj do_cleanexe do_cleanbak

do_cleanexe:
	-rm $(TARGET)

do_cleanobj: 
	-rm $(OBJ) 

do_cleanbak:
	-find . -type f -name '*~' -delete
