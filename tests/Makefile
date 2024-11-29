#
# Prerequisites:
#   sudo apt install googletest     -- on Ubuntu
#   brew install googletest         -- on MacOS
#
CXXFLAGS        ?= -std=c++20
TESTS           = test-initdb test-coverage
LONGTESTS       = test-maxsize
TESTS_SHA       = $(addsuffix .sha,$(TESTS))
TESTS_EXE       = $(addsuffix .exe,$(TESTS))
LONGTESTS_SHA   = $(addsuffix .sha,$(LONGTESTS))
OBJS            = test-smoke.o
GTEST           = gtest-all.o gtest_main.o

# Find googletest sources
ifneq (,$(wildcard /usr/src/googletest/googletest/src/gtest-all.cc))
    GTEST_DIR   = /usr/src/googletest/googletest
else ifneq (,$(wildcard /usr/local/opt/googletest/include/googletest/googletest/src/gtest-all.cc))
    GTEST_DIR   = /usr/local/opt/googletest/include/googletest/googletest
    CXXFLAGS    += -I/usr/local/opt/googletest/include
else ifneq (,$(wildcard /opt/homebrew/opt/googletest/include/googletest/googletest/src/gtest-all.cc))
    GTEST_DIR   = /opt/homebrew/opt/googletest/include/googletest/googletest
    CXXFLAGS    += -I/opt/homebrew/opt/googletest/include
else
    $(error Please install googletest)
endif

tests:          gtests $(TESTS_SHA)
		./gtests

longtests:      $(LONGTESTS_SHA)

%.sha: %.exe %.gold
		test=`basename $< .exe`;\
		rm -rf $$test.run      ;\
		mkdir $$test.run       ;\
		cd $$test.run          ;\
		../$<                  ;\
		sha1sum 52*[0-7] | tee ../$@ | cmp - ../$$test.gold || rm ../$@

.PRECIOUS: %.exe ../mars.o

%.exe: %.o ../mars.o
		$(CXX) -o $@ $^

.cc.o:
		$(CXX) $(CXXFLAGS) -Wall -c -I.. -o $@ $<

clean:
		rm -rf *.run
		rm -f $(TESTS_EXE) $(TESTS_SHA)
		rm -f *.o ../*.o gtests

gtests:         $(OBJS) $(GTEST)
		$(CXX) $(LDFLAGS) $(OBJS) $(GTEST) -o $@

gtest-all.o:    $(GTEST_DIR)/src/gtest-all.cc
		$(CXX) -c $(CXXFLAGS) -I$(GTEST_DIR) -o $@ $?

gtest_main.o:   $(GTEST_DIR)/src/gtest_main.cc
		$(CXX) -c $(CXXFLAGS) -o $@ $?
