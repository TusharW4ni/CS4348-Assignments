CC = /bin/g++
CCFLAGS = -g -std=c++11
INCLUDES =
LIBRARIES = -lboost_system -lboost_thread -lpthread -lrt
# LIBRARIES = -lpthread
EXECUTABLES = prefix-sum

prefix-sum: prefix-sum.o barrier.o
	$(CC) $(CCFLAGS) $(INCLUDES) -o prefix-sum prefix-sum.o barrier.o $(LIBRARIES)

barrier.o: barrier.h

# Rule for generating .o file from .cpp file
%.o: %.cpp
	$(CC) $(CCFLAGS) $(INCLUDES) -c $^ 

# All files to be generated
all: prefix-sum 

# Clean the directory
clean: 
	rm -f $(EXECUTABLES)  *.o
