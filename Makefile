CPPFLAGS=-g -O3 -Werror -Wall -Wno-psabi -mtune=native -march=native -std=c++11
ifndef OPT
CPPFLAGS += -DLOGGING -DCOUNTERS -DTIMERS
endif
LDFLAGS=-g
LDLIBS=
RM=rm -f

all: bin/nqueens bin/wqueens bin/langford bin/commafree

bin/nqueens: src/nqueens.cc src/logging.h src/counters.h src/flags.h
	g++ $(CPPFLAGS) -o bin/nqueens src/nqueens.cc $(LDLIBS)

bin/wqueens: src/wqueens.cc src/logging.h src/counters.h src/flags.h
	g++ $(CPPFLAGS) -o bin/wqueens src/wqueens.cc $(LDLIBS)

bin/langford: src/langford.cc src/logging.h src/counters.h src/flags.h
	g++ $(CPPFLAGS) -o bin/langford src/langford.cc $(LDLIBS)

bin/commafree: src/commafree.cc src/logging.h src/counters.h src/flags.h
	g++ $(CPPFLAGS) -o bin/commafree src/commafree.cc $(LDLIBS)

clean:
	$(RM) bin/nqueens
	$(RM) bin/wqueens
	$(RM) bin/langford
	$(RM) bin/commafree
	$(RM) *~
	$(RM) */*~
