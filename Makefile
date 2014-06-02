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

$(OBJ) : %.o : %.cc %.h global.h helper.h
	$(CC) -c $(CFLAG) $< -o $@

run: 
	make && $(TARGET)

.PHONY: clean cleanall
	
clean : cleanobj
cleanall: cleanobj cleanexe cleanbak

cleanexe:
	-rm $(TARGET)

cleanobj: 
	-rm $(OBJ) 

cleanbak:
	-find . -type f -name '*~' -delete
