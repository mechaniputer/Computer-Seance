EXEC = testcomp
SOURCES = test_computer.cpp computer.cpp raquette.cpp lvdc.cpp

raq:
	g++ -D USE_RAQ -g -Wall -o $(EXEC) $(SOURCES) -lncurses

raqtest:
	g++ -D USE_RAQTEST -g -Wall -o $(EXEC) $(SOURCES) -lncurses

lvdc:
	g++ -D USE_LVDC -g -Wall -o $(EXEC) $(SOURCES) -lncurses

nocomputer:
	g++ -D USE_NOCOMPUTER -g -Wall -o $(EXEC) $(SOURCES) -lncurses

clean:
	rm $(EXEC)
