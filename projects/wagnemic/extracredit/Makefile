CFLAGS = -Wall -fpic -coverage -lm

tarantula.out: tarantula.c rngs.o interface.o dominion.c unittest1.c unittest2.c unittest3.c unittest4.c cardtest4.c randomtestadventurer.c randomtestcard1.c randomtestcard2.c dominion.h dominion_helpers.h
	gcc -o tarantula tarantula.c $(CFLAGS)
	tarantula > tarantula.out
    
tarantula.html: ansi2html.py tarantula.out
	python ansi2html.py > tarantula.html

rngs.o: rngs.h rngs.c
	gcc -c rngs.c -g  $(CFLAGS)

dominion.o: dominion.h dominion.c rngs.o
	gcc -c dominion.c -g  $(CFLAGS)

interface.o: interface.h interface.c
	gcc -c interface.c -g  $(CFLAGS)

clean:
	rm -f *.o playdom.exe playdom player player.exe *.gcov *.gcda *.gcno *.so *.out testDrawCard testDrawCard.exe unittest1 unittest2 unittest3 unittest4 cardtest1 cardtest2 cardtest3 cardtest4 randomtestadventurer randomtestcard1 randomtestcard2 tarantula tarantula.html *.pyc ./ansi2html/*.pyc
