TESTS = test-initdb test-coverage

tests: $(addsuffix .sha,$(TESTS))

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
	rm -rf *.run	;\
	rm -f $(addsuffix .exe,$(TESTS)) $(addsuffix .sha,$(TESTS))
