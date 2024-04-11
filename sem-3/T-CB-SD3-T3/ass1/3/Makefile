SRC = ass3.c
TARGET = ass3

all : $(SRC)
	$(CC) -g $(SRC) -o $(TARGET)

test : clean $(SRC)
	$(CC) -g $(SRC) -o $(TARGET)
	@./test

clean : 
	rm -f $(TARGET) out*
