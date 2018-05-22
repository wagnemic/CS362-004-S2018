CFLAGS = -Wall -fpic -coverage -lm

all_out_files_from_teammates_bugged_dominion: dominion_buggy.o unittest1.c unittest2.c unittest3.c unittest4.c cardtest1.c cardtest2.c cardtest3.c cardtest4.c randomtestadventurer.c randomtestcard1.c randomtestcard2.c interface.o rngs.o
	echo "Result for running all unit tests:" > unittestresults.out
	
	echo "unittest1.c:" >> unittestresults.out
	gcc -o unittest1 unittest1.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	unittest1 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out
	
	echo "unittest2.c:" >> unittestresults.out
	gcc -o unittest2 unittest2.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	unittest2 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out
	
	echo "unittest3.c:" >> unittestresults.out
	gcc -o unittest3 unittest3.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	unittest3 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out
	
	echo "unittest4.c:" >> unittestresults.out
	gcc -o unittest4 unittest4.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	unittest4 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out
	
	echo "cardtest1.c:" >> unittestresults.out
	gcc -o cardtest1 cardtest1.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	cardtest1 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out
	
	echo "cardtest2.c:" >> unittestresults.out
	gcc -o cardtest2 cardtest2.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	cardtest2 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out
	
	echo "cardtest3.c:" >> unittestresults.out
	gcc -o cardtest3 cardtest3.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	cardtest3 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out
	
	echo "cardtest4.c:" >> unittestresults.out
	gcc -o cardtest4 cardtest4.c -g dominion_buggy.o rngs.o interface.o $(CFLAGS)
	cardtest4 >> unittestresults.out
	gcov dominion_buggy.c -b -f >> unittestresults.out

	echo "randomtestadventurer.c:" >> randomtestadventurer.out
	gcc -o randomtestadventurer randomtestadventurer.c -g dominion_buggy.o interface.o rngs.o $(CFLAGS)
	randomtestadventurer >> randomtestadventurer.out
	gcov dominion_buggy.c -b -f >> randomtestadventurer.out
    
	echo "randomtestcard1.c:" >> randomtestcard1.out
	gcc -o randomtestcard1 randomtestcard1.c -g dominion_buggy.o interface.o rngs.o $(CFLAGS)
	randomtestcard1 >> randomtestcard1.out
	gcov dominion_buggy.c -b -f >> randomtestcard1.out
    
	echo "randomtestcard2.c:" >> randomtestcard2.out
	gcc -o randomtestcard2 randomtestcard2.c -g dominion_buggy.o interface.o rngs.o $(CFLAGS)
	randomtestcard2 >> randomtestcard2.out
	gcov dominion_buggy.c -b -f >> randomtestcard2.out

all_out_files_from_teammates_dominion_with_bug_fixes: dominion_fixed.o unittest1.c unittest2.c unittest3.c unittest4.c cardtest1.c cardtest2.c cardtest3.c cardtest4.c randomtestadventurer.c randomtestcard1.c randomtestcard2.c interface.o rngs.o
	echo "Result for running all unit tests:" > unittestresults.out
	
	echo "unittest1.c:" >> unittestresults.out
	gcc -o unittest1 unittest1.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	unittest1 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out
	
	echo "unittest2.c:" >> unittestresults.out
	gcc -o unittest2 unittest2.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	unittest2 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out
	
	echo "unittest3.c:" >> unittestresults.out
	gcc -o unittest3 unittest3.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	unittest3 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out
	
	echo "unittest4.c:" >> unittestresults.out
	gcc -o unittest4 unittest4.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	unittest4 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out
	
	echo "cardtest1.c:" >> unittestresults.out
	gcc -o cardtest1 cardtest1.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	cardtest1 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out
	
	echo "cardtest2.c:" >> unittestresults.out
	gcc -o cardtest2 cardtest2.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	cardtest2 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out
	
	echo "cardtest3.c:" >> unittestresults.out
	gcc -o cardtest3 cardtest3.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	cardtest3 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out
	
	echo "cardtest4.c:" >> unittestresults.out
	gcc -o cardtest4 cardtest4.c -g dominion_fixed.o rngs.o interface.o $(CFLAGS)
	cardtest4 >> unittestresults.out
	gcov dominion_fixed.c -b -f >> unittestresults.out

	echo "randomtestadventurer.c:" >> randomtestadventurer.out
	gcc -o randomtestadventurer randomtestadventurer.c -g dominion_fixed.o interface.o rngs.o $(CFLAGS)
	randomtestadventurer >> randomtestadventurer.out
	gcov dominion_fixed.c -b -f >> randomtestadventurer.out
    
	echo "randomtestcard1.c:" >> randomtestcard1.out
	gcc -o randomtestcard1 randomtestcard1.c -g dominion_fixed.o interface.o rngs.o $(CFLAGS)
	randomtestcard1 >> randomtestcard1.out
	gcov dominion_fixed.c -b -f >> randomtestcard1.out
    
	echo "randomtestcard2.c:" >> randomtestcard2.out
	gcc -o randomtestcard2 randomtestcard2.c -g dominion_fixed.o interface.o rngs.o $(CFLAGS)
	randomtestcard2 >> randomtestcard2.out
	gcov dominion_fixed.c -b -f >> randomtestcard2.out
    
dominion_buggy.o: dominion_buggy.c dominion.h rngs.o
	gcc -c dominion_buggy.c -g  $(CFLAGS)
    
dominion_fixed.o: dominion_fixed.c dominion.h rngs.o
	gcc -c dominion_fixed.c -g  $(CFLAGS)

rngs.o: rngs.h rngs.c
	gcc -c rngs.c -g  $(CFLAGS)

interface.o: interface.h interface.c
	gcc -c interface.c -g  $(CFLAGS)

clean:
	rm -f *.o playdom.exe playdom player player.exe *.gcov *.gcda *.gcno *.so *.out testDrawCard testDrawCard.exe unittest1 unittest2 unittest3 unittest4 cardtest1 cardtest2 cardtest3 cardtest4 randomtestadventurer randomtestcard1 randomtestcard2
