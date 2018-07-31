CC = clang
CFLAGS = -O2

%.ir: %.c default.profdata
	$(CC) $(CFLAGS) -S -emit-llvm -fprofile-instr-use=default.profdata -o $@ $<

hadamard: CFLAGS += -fprofile-instr-generate

hadamard: hadamard.o hadamard_main.o
	$(CC) $(CFLAGS) -o $@ $^

default.profraw: hadamard
	./hadamard

default.profdata: default.profraw
	llvm-profdata merge -o $@ $<

run: hadamard.ir
	../bb-printer.sh hadamard.ir

clean:
	rm -f *.o hadamard *.ir *.gv *.profraw *.profdata
