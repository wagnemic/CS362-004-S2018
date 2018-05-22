Assignment 5 instructions:


To run all unit and random tests I created for assignment-3 and assignment-4 against my teammate's buggy dominion code, dominion_buggy.c, follow these instructions:

1. Set your current directory to this folder
2. Execute the following command to ensure the directory is clean of extra files:
    make clean
3. Execute the following command:
    make all_out_files_from_teammates_bugged_dominion
4. View the results from every unit test with the following commands, one command per set of tests:
    cat unittestresults.out
    cat randomtestadventurer.out
    cat randomtestcard1.out
    cat randomtestcard2.out
5. Note that some tests will fail because you've used the original, buggy, dominion code from my teammate
6. Optionally clean the directory to its initial state using the following command: 
    make clean

    
To run all unit and random tests I created for assignment-3 and assignment-4 against my teammate's dominion code with fixes I implemented to all bugs involved in all tests, dominion_fixed.c, follow these instructions:

1. Set your current directory to this folder
2. Execute the following command to ensure the directory is clean of extra files:
    make clean
3. Execute the following command:
    make all_out_files_from_teammates_dominion_with_bug_fixes
4. View the results from every unit test with the following commands, one command per set of tests:
    cat unittestresults.out
    cat randomtestadventurer.out
    cat randomtestcard1.out
    cat randomtestcard2.out
5. Note that no tests will fail because you've used the modified, fixed, dominion code originally from my teammate but I included fixes for all bugs involved in every test (all fixes in dominion_fixed.c are marked with a comment "FIXED")
6. Optionally clean the directory to its initial state using the following command: 
    make clean
