all: bin/Debug/hcc

obj/Debug/actvar.o: actvar.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c actvar.cpp     -o obj/Debug/actvar.o
obj/Debug/genasm.o: genasm.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c genasm.cpp     -o obj/Debug/genasm.o
obj/Debug/lex.o: lex.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c lex.cpp        -o obj/Debug/lex.o
obj/Debug/main.o: main.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c main.cpp       -o obj/Debug/main.o
obj/Debug/opt.o : opt.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c opt.cpp        -o obj/Debug/opt.o
obj/Debug/opt_dag.o : opt_dag.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c opt_dag.cpp    -o obj/Debug/opt_dag.o
obj/Debug/semantics.o : semantics.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c semantics.cpp  -o obj/Debug/semantics.o
obj/Debug/syntax.o : syntax.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c syntax.cpp     -o obj/Debug/syntax.o
obj/Debug/util.o : util.cpp stdafx.h semantics.h actvar.h
	g++ -std=c++11  -c util.cpp       -o obj/Debug/util.o
bin/Debug/hcc : obj/Debug/actvar.o obj/Debug/genasm.o obj/Debug/lex.o obj/Debug/main.o obj/Debug/opt.o obj/Debug/opt_dag.o obj/Debug/semantics.o obj/Debug/syntax.o obj/Debug/util.o
	g++ -o bin/Debug/hcc obj/Debug/*.o

clean :
	rm obj/Debug/*.o
	rm bin/Debug/hcc

check:
	bin/Debug/hcc tests/multitest.c >/dev/null
	bin/Debug/hcc tests/1.ascii.c >/dev/null
	bin/Debug/hcc tests/2.calc.c >/dev/null
	bin/Debug/hcc tests/3.fibo.c >/dev/null
	bin/Debug/hcc tests/4.opt.c >/dev/null
	bin/Debug/hcc tests/5.syn.c >/dev/null

	! bin/Debug/hcc tests/7.calc_err.c >/dev/null
	! bin/Debug/hcc tests/8.fibo_err.c >/dev/null
	! bin/Debug/hcc tests/9.opt_err.c >/dev/null
	! bin/Debug/hcc tests/10.syn_err.c >/dev/null