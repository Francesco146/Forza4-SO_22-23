CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -O3 -flto -fvisibility=hidden -fvisibility-inlines-hidden -std=c99
SRC = src/*.c
INC = include
OBJ = obj/*.o
BIN = bin/Forza4
# if you want to use the debug version, uncomment the following line
CFLAGS = -g


all: $(BIN)

$(BIN): $(OBJ)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)


$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -I $(INC) -c $(SRC)
	mkdir -p obj
	mv *.o obj/

clean:
	/bin/rm -rf $(OBJ) $(BIN) $(BIN).dSYM

tarball:
	tar -cvzf forza4.tar.gz Makefile $(INC) $(SRC)