#
# Makefile
#
VERSION=1.1.0
PROJECT=bmconv

all clean install uninstall:
	make $@ -sC src

tarball: clean
	debclean
	cd .. && tar -czvf $(PROJECT)-$(VERSION).tar.gz $(PROJECT)-$(VERSION) --exclude .svn

regtest:
	make all -sC src
	make clean -sC test
	make regtest -sC test
