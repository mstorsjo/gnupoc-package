#
# Makefile
#
VERSION=1.1.0
PROJECT=petran

all clean install uninstall:
	$(MAKE) $@ -C petran
	$(MAKE) $@ -C e32uid

tarball: clean
	cd .. && tar -czvf $(PROJECT)-$(VERSION).tar.gz $(PROJECT)-$(VERSION) --exclude .svn

regtest:
	$(MAKE) all -C petran
	$(MAKE) regtest -C test
