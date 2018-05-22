#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"

/* Unit tests for isGameOver function
 *
 * code inspection shows that isGameOver shouldn't modify any of the game state and only checks values from the supplyCount field
 * so we only need to test for return values while varying the supplies in the game
 * a game is over if either the province pile is empty or three supplies are empty, so the tests are
 * 1. check for isGameOver modifying the game state (on a new game as well as every game we modify during the test)
 * 2. check the result against expected for 2 partitions of the province supply (1, 0), and note that -1 is a bad game state case, and this function is designed to assume it won't be given a game state like this and its results are undefined in that case
 * 3. check if any 0 through 4 supplies (not using province) provide the correct game state, and note this isn't comprehensive because there's (sum C(16,x) for x=(0,16))=65536 combinations of this scenario if we went to 16 (but we're partitioning 4-16 as just 4) which is unwieldy, and a realistic game would have likely at most 4 empty supplies at the end of a turn rather than 16
 * 4. check just 3 empty supplies (not using province), but cycling through enough such that each non province card is involved in 3 sets of 3 empty supplies, which makes it easier to identify the culprits of failed tests (it would take 560=C(16,3) tests to test all combinations of 3 empty supplies). another option would have been to find 2 supplies we know aren't an issue, and iterate over replacing the remaining 14 as the 3rd empty set, but that requires some a priori knowledge or conditionals that may not result in any tests at all if there are not at least 2 cards that don't cause an issue on their own
 *
 */

void cardNumToName(int card, char *name);

int main()
{
    int seed = 68;
    int s, v, n, c, r;
    int selectedSupplies[3];
    int nonProvinceCards[16];
    char cardNames[3][100];
    char kingdomNames[2][100];
    struct gameState cleanGame, testGame, holdGame;

    // two sets of kingdom cards to test usage of all of them
    int k[2][10] = {
                    {adventurer, council_room, feast, gardens, mine, remodel, smithy, village, baron, great_hall},
                    {minion, steward, tribute, ambassador, cutpurse, embargo, outpost, salvager, sea_hag, treasure_map} };

    // for output
    strcpy(kingdomNames[0], "the first half of the kingdom cards");
    strcpy(kingdomNames[1], "the second half of the kingdom cards");

    // these are for varying the province supply values and testing against expected isGameOver result    
    int provinceSupplyValues[2] = { 1, 0 };
    int isGameOverExpectedResult[2] = { 0, 1 }; // no game over when 1 province left, but there is game over when 0 province left

    printf("Unit Tests Function 1 - isGameOver():\n");

    // loop over both sets of kingdom cards
    for (s = 0; s < 2; ++s)
    {
        // initialize a clean game state for kingdom set s and 4 players
        memset(&cleanGame, 0, sizeof(struct gameState));
        initializeGame(MAX_PLAYERS, k[s], seed, &cleanGame);


        // 1. test if calling isGameOver correctly does not affect the game state

        memcpy(&testGame, &cleanGame, sizeof(struct gameState)); // set testGame to a clean game

        isGameOver(&testGame); // call isGameOver to to see if testGame is modified

        // check if bytes of testGame matches bytes of cleanGame
        if (memcmp(&testGame, &cleanGame, sizeof(struct gameState)) == 0)
            printf("isGameOver(): PASS when checking for unintended side effects on a new game using %s\n", kingdomNames[s]);
        else
            printf("isGameOver(): FAIL when checking for unintended side effects on a new game using %s\n", kingdomNames[s]);


        // 2. test if isGameOver correctly detects if the game is over when there's v (1, 0) province cards left 

        // loop over the 2 province supply values we're testing
        for (v = 0; v < 2; ++v)
        {
            memcpy(&testGame, &cleanGame, sizeof(struct gameState)); // set testGame to a clean game

            testGame.supplyCount[province] = provinceSupplyValues[v]; // set province supply value

            memcpy(&holdGame, &testGame, sizeof(struct gameState)); // before call to isGameOver, save state of the testGame (which has been modified from cleanGame state)

            r = isGameOver(&testGame);

            // check actual result against expected result
            if (r == isGameOverExpectedResult[v])
                printf("isGameOver(): PASS when checking function return using a province supply of %d and %s (expected return = %d, actual return = %d)\n", provinceSupplyValues[v], kingdomNames[s], isGameOverExpectedResult[v], r);
            else
                printf("isGameOver(): FAIL when checking function return using a province supply of %d and %s (expected return = %d, actual return = %d)\n", provinceSupplyValues[v], kingdomNames[s], isGameOverExpectedResult[v], r);

            // make sure testGame is the same state as holdGame after execution of isGameOver
            if (memcmp(&testGame, &holdGame, sizeof(struct gameState)) == 0)
                printf("isGameOver(): PASS when checking for unintended side effects on the game from the previous test using %s\n", kingdomNames[s]);
            else
                printf("isGameOver(): FAIL when checking for unintended side effects on the game from the previous test using %s\n", kingdomNames[s]);

        }


        // 3. test if isGameOver detects the proper game over for 0, 1, 2, ..., 4 empty supplies
        // if we did up to 16 empty, it would be 65536=(sum C(16,x) x=(0,16)) possible combinations of 0-16 empty supplies, but we're just doing one combination of each 0-4 instead
        // with just 4, its 2517=(sum C(16,x) x=(0,4)) possible combinations of 0-4 empty supplies

        for (n = 0; n <= 4 /*16*/; ++n) // n = 0, 1, 2 ... 4 (could do 16 if we wanted) because 0-16 possible supplies to set to 0 (10 kingdom, 3 treausre, curse, estate, duchy)
        {
            memcpy(&testGame, &cleanGame, sizeof(struct gameState)); // set testGame to a clean game

            // no breaks in this switch because we want all statements below the case to also execute to get those supplies to 0
            switch (n)
            {
            case 16:
                testGame.supplyCount[k[s][9]] = 0; // kingdom cards depend on which set this iteration is on
            case 15:
                testGame.supplyCount[k[s][8]] = 0;
            case 14:
                testGame.supplyCount[k[s][7]] = 0;
            case 13:
                testGame.supplyCount[k[s][6]] = 0;
            case 12:
                testGame.supplyCount[k[s][5]] = 0;
            case 11:
                testGame.supplyCount[k[s][4]] = 0;
            case 10:
                testGame.supplyCount[k[s][3]] = 0;
            case 9:
                testGame.supplyCount[k[s][2]] = 0;
            case 8:
                testGame.supplyCount[k[s][1]] = 0;
            case 7:
                testGame.supplyCount[k[s][0]] = 0;
            case 6:
                testGame.supplyCount[gold] = 0; // gold thru curse are in all games
            case 5:
                testGame.supplyCount[silver] = 0;
            case 4:
                testGame.supplyCount[copper] = 0;
            case 3:
                testGame.supplyCount[duchy] = 0;
            case 2:
                testGame.supplyCount[estate] = 0;
            case 1:
                testGame.supplyCount[curse] = 0;
            case 0:
                break;
            }

            memcpy(&holdGame, &testGame, sizeof(struct gameState)); // before call to isGameOver, save state of the testGame (which has been modified from cleanGame state)

            r = isGameOver(&testGame);

            // n is number of empty supplies, so if its >=3, the game should be over, otherwise not over (could just put n>=3 in place of the 1 and use just one if-else, but this makes it more clear to someone reading this what's going on)
            if (n >= 3)
                if (r == 1)
                    printf("isGameOver(): PASS when checking function return using %d empty supply(ies) and %s (expected return = %d, actual return = %d)\n", n, kingdomNames[s], 1, r);
                else
                    printf("isGameOver(): FAIL when checking function return using %d empty supply(ies) and %s (expected return = %d, actual return = %d)\n", n, kingdomNames[s], 1, r);
            else
                if (r == 0)
                    printf("isGameOver(): PASS when checking function return using %d empty supply(ies) and %s (expected return = %d, actual return = %d)\n", n, kingdomNames[s], 0, r);
                else
                    printf("isGameOver(): FAIL when checking function return using %d empty supply(ies) and %s (expected return = %d, actual return = %d)\n", n, kingdomNames[s], 0, r);

            // make sure testGame is the same state as holdGame after execution of isGameOver
            if (memcmp(&testGame, &holdGame, sizeof(struct gameState)) == 0)
                printf("isGameOver(): PASS when checking for unintended side effects on the game from the previous test using %s\n", kingdomNames[s]);
            else
                printf("isGameOver(): FAIL when checking for unintended side effects on the game from the previous test using %s\n", kingdomNames[s]);

        }

        // 4. test if isGameOver detects game over 3 supplies of varying cards are 0 (excludig province)
        //    the 3 supplies will rotate enough that each supply has a chance to be in three tests, to more easily identify which cards cause the failure

        // stores the non province cards for this game in an array to loop over 
        nonProvinceCards[0] = curse;
        nonProvinceCards[1] = estate;
        nonProvinceCards[2] = duchy;
        nonProvinceCards[3] = copper;
        nonProvinceCards[4] = silver;
        nonProvinceCards[5] = gold;
        nonProvinceCards[6] = k[s][0];
        nonProvinceCards[7] = k[s][1];
        nonProvinceCards[8] = k[s][2];
        nonProvinceCards[9] = k[s][3];
        nonProvinceCards[10] = k[s][4];
        nonProvinceCards[11] = k[s][5];
        nonProvinceCards[12] = k[s][6];
        nonProvinceCards[13] = k[s][7];
        nonProvinceCards[14] = k[s][8];
        nonProvinceCards[15] = k[s][9];

        for (c = 0; c < 16; ++c) // there's 16 supplies in a game not including the provinces (10 kingdom, 3 treasure, curse, estate, duchy), so we need at least 16 iterations to allow each supply to be involved in 3 tests of 3 empty supplies
        {
            memcpy(&testGame, &cleanGame, sizeof(struct gameState)); // set testGame to a clean game

            // select the supplies to empty for this iteration (the mod 16 is there so when we get to c>=14, it loops back to 0
            selectedSupplies[0] = nonProvinceCards[c % 16];
            selectedSupplies[1] = nonProvinceCards[(c + 1) % 16];
            selectedSupplies[2] = nonProvinceCards[(c + 2) % 16];

            // empty the supply for each selected supply and store the card name for helpful output
            for (n = 0; n < 3; ++n)
            {
                testGame.supplyCount[selectedSupplies[n]] = 0;
                cardNumToName(selectedSupplies[n], cardNames[n]);
            }

            memcpy(&holdGame, &testGame, sizeof(struct gameState)); // before call to isGameOver, save state of the testGame (which has been modified from cleanGame state)

            r = isGameOver(&testGame);

            if (r == 1) // should always be a game over with any 3 empty supplies
                printf("isGameOver(): PASS when checking function return using empty %s, %s, and %s supplies and %s (expected return = %d, actual return = %d)\n", cardNames[0], cardNames[1], cardNames[2], kingdomNames[s], 1, r);
            else
                printf("isGameOver(): FAIL when checking function return using empty %s, %s, and %s supplies and %s (expected return = %d, actual return = %d)\n", cardNames[0], cardNames[1], cardNames[2], kingdomNames[s], 1, r);
            // make sure testGame is the same state as holdGame after execution of isGameOver
            if (memcmp(&testGame, &holdGame, sizeof(struct gameState)) == 0)
                printf("isGameOver(): PASS when checking for unintended side effects on the game from the previous test using %s\n",kingdomNames[s]);
            else
                printf("isGameOver(): FAIL when checking for unintended side effects on the game from the previous test using %s\n", kingdomNames[s]);
        }

    }

    return 0;
}