run-tests:
	cd tests; make

re-mars.bin: $(HOME)/.besm6/1234
	besmtool dump 1234 --start=01000 --length=1 --to-file=re-mars.bin

mars.bin: /usr/local/share/besm6/2048
	besmtool dump 2048 --start=0657 --length=1 --to-file=mars.bin

