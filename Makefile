# paths
INCLUDE = include
MODULES = modules

# compiler
CXX = g++

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CXXFLAGS = -Wall -Werror -g -I$(INCLUDE) -pthread

# Αρχεία .o
SRCS = parent.cpp $(MODULES)/aid_functions.cpp #$(MODULES)/wildcards.cpp $(MODULES)/redirection.cpp $(MODULES)/pipes.cpp $(MODULES)/execution.cpp
OBJS = parent.o $(MODULES)/aid_functions.o #$(MODULES)/wildcards.o $(MODULES)/redirection.o $(MODULES)/pipes.o $(MODULES)/execution.o

# Το εκτελέσιμο πρόγραμμα
EXEC = server

# Παράμετροι για δοκιμαστική εκτέλεση

# g++ multithread_client.cpp aid_functions.cpp aid_functions.h -o pollSwayer -pthread
# ./pollSwayer 127.0.0.1 8077 inputFile.txt


$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC) $(CXXFLAGS)

clean:
	rm -f $(OBJS) $(EXEC)



valgrind: $(EXEC)
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all --track-fds=yes ./$(EXEC) 