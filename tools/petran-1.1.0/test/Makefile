#
# Makefile
#

BIN=../petran/petran
TEST_EXES=esidplay.exe snus.exe
SRC_EXES=$(patsubst %.%, %.%.pe, $(TEST_EXES))
REF_EXES=$(patsubst %, %.ref, $(TEST_EXES))

# Recursive make considered harmless ?
regtest:
	@$(MAKE)  clean
	@$(MAKE)  $(TEST_EXES)
	@echo "all OK"

%.exe:
	@echo "$@: run petran, remove timestamp and compare"
	$(BIN) $@.pe $@ \
		-nocall -uid1 0x1000007a -uid2 0x00000000 -uid3 0x00000000 1>/dev/null
	rm -f a b
	dd if=$@ of=a bs=16 count=2 2>/dev/null
	dd if=$@ of=b bs=16 skip=3 2>/dev/null
	cat a b > $@.nots
	rm -f a b
	dd if=$@.ref of=a bs=16 count=2 2>/dev/null
	dd if=$@.ref of=b bs=16 skip=3 2>/dev/null
	cat a b > $@.ref.nots
	rm -f a b
	diff $@.nots $@.ref.nots &>/dev/null
	rm -f $@.nots $@.ref.nots

%_nots.exe: %.exe


clean:
	rm -f $(TEST_EXES)
