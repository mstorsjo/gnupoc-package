regtest:
	../src/makesis -t7d7,7,18,11,20,35 test1.pkg
	../src/makesis -t7d7,7,18,11,21,0 test2.pkg
	../src/makesis -t7d7,7,18,12,1c,8 test3.pkg
	../src/makesis -t7d7,7,18,13,8,d test4.pkg
	../src/makesis -t7d7,7,18,13,30,10 test5.pkg
	../src/makesis -ddata -t7d7,7,18,13,30,20 test6.pkg
	../src/makesis -t7d7,7,18,13,36,26 test7.pkg
	../src/makesis -t7d7,7,18,13,36,2c test8.pkg
	../src/makesis -t7d7,7,18,15,2a,2e test9.pkg
	../src/makesis -t7d7,7,19,7,20,23 test10.pkg
	../src/makesis -t7d7,7,19,7,34,12 test11.pkg
	../src/makesis -t7d7,7,19,7,36,1e test12.pkg
	../src/makesis -t7d7,7,19,8,37,1c test13.pkg
	../src/makesis -t7d7,7,19,9,22,2f test14.pkg
	../src/makesis -t7d7,7,19,a,c,30 test15.pkg
	../src/makesis -t7d7,7,19,a,26,8 test16.pkg
	../src/makesis -t7d7,7,19,b,19,39 test17.pkg
	../src/makesis -t7d7,7,19,c,3a,2d test18.pkg
	../src/makesis -t7d7,7,19,c,3b,9 test19.pkg
	../src/signsis test1.sis test1.sisx selfsigned.cer selfsigned.key
	md5sum -c md5sums
