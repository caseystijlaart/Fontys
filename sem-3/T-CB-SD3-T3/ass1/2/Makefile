SRC = ass2.c
TARGET = ass2

all : $(SRC)
	$(CC) -DTEST_TIME $(SRC) -o $(TARGET)

test : clean $(SRC)
	$(CC) $(SRC) -o $(TARGET)
	@./test

perf_tests : all
	@./perf

clean : 
	rm -f $(TARGET) out.txt
