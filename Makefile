CPPFLAGS=-g -O3 -Werror -Wall -Wno-psabi -mtune=native -march=native -std=c++11
ifndef OPT
CPPFLAGS += -DLOGGING -DCOUNTERS -DTIMERS
endif
LDFLAGS=-g
LDLIBS=
RM=rm -f

all: bin/nqueens

bin/nqueens: src/nqueens.cc src/logging.h src/counters.h src/flags.h
	g++ $(CPPFLAGS) -o bin/nqueens src/nqueens.cc $(LDLIBS)

clean:
	$(RM) bin/nqueens
	$(RM) *~
	$(RM) */*~
