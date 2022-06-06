interpreter: interpret.o test.o
	g++ $^ -o $@ -Wall

%.o: %.c
	g++ -g -c $< -o $@ -Wall

clean:
	rm -f *.o interpreter
