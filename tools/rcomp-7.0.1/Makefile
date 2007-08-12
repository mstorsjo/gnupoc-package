#
# Makefile
#
all clean install uninstall:
	make $@ -sC src

regtest:
	make all -sC src
	make regtest -sC tsrc
