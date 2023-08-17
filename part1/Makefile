CXX   ?= g++-8
EXE = iplC
CXXDEBUG = -g -Wall
CXXSTD = -std=c++11


.PHONY: all
all: parser lexer 	
	$(CXX) $(CXXDEBUG) -o iplC driver.cpp parser.o scanner.o 

.PHONY: parser
parser: parser.yy scanner.hh
	bison -d -v $<
	$(CXX) $(CXXDEBUG) -c parser.tab.cc -o parser.o 

.PHONY: lexer
lexer: scanner.l scanner.hh parser.tab.hh parser.tab.cc	
	flex++ --outfile=scanner.yy.cc  $<
	$(CXX)  $(CXXDEBUG) -c scanner.yy.cc -o scanner.o

./PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.cc
	rm -rf FL.h
	rm -rf location.hh
	rm -rf parser.output
	rm -rf parser.tab.hh
	rm -rf position.hh
	rm -rf stack.hh


