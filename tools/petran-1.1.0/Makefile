#
# Makefile
#
VERSION=1.1.0
PROJECT=petran

all clean install uninstall:
	make $@ -C petran
	make $@ -C e32uid

tarball: clean
	cd .. && tar -czvf $(PROJECT)-$(VERSION).tar.gz $(PROJECT)-$(VERSION) --exclude .svn

regtest:
	make all -C petran
	make regtest -C test
