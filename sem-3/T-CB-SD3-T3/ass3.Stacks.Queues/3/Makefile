
all : ass3.c
	$(CC) -DTEST_TIME ass3.c -o ass3

test : clean ass3.c
	$(CC) ass3.c -o ass3
	@./test

perf_tests : all
	@./perf

clean : 
	rm -f ass3 out.txt
