AR=ar
CC=gcc
SRC=rsocket.c
OBJ=rsocket.o

all: clean librsocket.a

librsocket.a: $(OBJ)
	$(AR) rsc librsocket.a rsocket.o
$(OBJ):$(SRC)
	$(CC) -c $(SRC) -o $(OBJ)
clean:
	-@rm *.a *.o
