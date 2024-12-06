CXXFLAGS ?= -Wall -std=c++20

run-tests: mars.o
	make -C tests CXX=$(CXX) CXXFLAGS="$(CXXFLAGS)"

analyzer: analyzer.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f mars.o
	make -C tests clean

re-mars.bin: $(HOME)/.besm6/1234
	besmtool dump 1234 --start=01000 --length=1 --to-file=re-mars.bin

mars.bin: /usr/local/share/besm6/2048
	besmtool dump 2048 --start=0657 --length=1 --to-file=mars.bin

mars.o: mars.cc mars.h

analyzer.o: analyzer.cc mars.h
