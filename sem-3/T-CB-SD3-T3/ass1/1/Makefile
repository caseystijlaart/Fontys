
all : ass1.c
	$(CC) -DTEST_TIME ass1.c -o ass1

test : clean ass1.c
	$(CC) ass1.c -o ass1
	@./test

perf_tests : all
	@./perf

clean : 
	rm -f ass1 out.txt
