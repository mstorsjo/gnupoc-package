regtest:
	../src/makesis test.pkg
	../src/makesis -da test2.pkg
	md5sum test*.{sis,SIS}
	md5sum -c md5sums
