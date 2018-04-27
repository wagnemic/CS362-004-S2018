#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"

/*
 * Unit tests for updateCoins function
 *
 * code inspection of updateCoins shows that it only uses one player's hand from the game state, so the only game state we need to modify that array (and the count) during the tests
 * we also need to have tests using the bonus parameter, and we should make sure it works for both players
 * also, the code inspection reveals there is one return statement with no previous conditionals so it always will return that value (0) and there's no need to test that because its clearly guarunteed
 * the updateCoins function's purpose is to set the state's coins variable to the treasure value in the player's hand plus the amount in the bonus parameter
 * which means we need to make sure that value is correct and the rest of the game state is not modified
 * the tests used are
 * 1. test with 0 for both player's hands but a bunch of arbitary treasure in their discard and deck, and no bonus
 * 2. test with 0-5 non-treasure in the tested player's hand and a bunch of arbitrary treasure in the other player's hand
 * 3. test with only 0-5 copper, only 0-5 silver, only 0-5 gold in player's hand and only 0-5 bonus coins, and a lot of treasure in the other player's hand
 * 4. test with 0-5 copper silver gold and bonus coins, and a lot of treasure in the other player's hand
 *
 */

struct UpdateCoinsTestResultData
{
    int expectedCoins; // expected coins for the test
    int actualCoins; // actual coins for the test
    int sideEffectsTest; // 1 if no side effects, 0 if side effects
};

// return coin value based on number of treasure and bonus coins provided
int calcExpectedCoins(int numCopper, int numSilver, int numGold, int bonus)
{
    return numCopper + numSilver * 2 + numGold * 3 + bonus;
}

// puts the given amount of copper, silver, gold and non-treasure cards in the player's pile location (0=hand,1=discard,2=deck)
void setPile(int player, struct gameState * G, int numCopper, int numSilver, int numGold, int numNonTreasure, int location)
{
    int i;

    switch (location)
    {
    case 0:
        // put copper in pile
        for (i = 0; i < numCopper; ++i)
            G->hand[player][G->handCount[player]++] = copper;

        // put silver in pile
        for (i = 0; i < numSilver; ++i)
            G->hand[player][G->handCount[player]++] = silver;

        // put gold in pile
        for (i = 0; i < numGold; ++i)
            G->hand[player][G->handCount[player]++] = gold;

        // put adventurer (non treasure) in pile
        for (i = 0; i < numNonTreasure; ++i)
            G->hand[player][G->handCount[player]++] = adventurer;
        break;
    case 1:
        // put copper in pile
        for (i = 0; i < numCopper; ++i)
            G->discard[player][G->discardCount[player]++] = copper;

        // put silver in pile
        for (i = 0; i < numSilver; ++i)
            G->discard[player][G->discardCount[player]++] = silver;

        // put gold in pile
        for (i = 0; i < numGold; ++i)
            G->discard[player][G->discardCount[player]++] = gold;

        // put adventurer (non treasure) in pile
        for (i = 0; i < numNonTreasure; ++i)
            G->discard[player][G->discardCount[player]++] = adventurer;
        break;
    case 2:
        // put copper in pile
        for (i = 0; i < numCopper; ++i)
            G->deck[player][G->deckCount[player]++] = copper;

        // put silver in pile
        for (i = 0; i < numSilver; ++i)
            G->deck[player][G->deckCount[player]++] = silver;

        // put gold in pile
        for (i = 0; i < numGold; ++i)
            G->deck[player][G->deckCount[player]++] = gold;

        // put adventurer (non treasure) in pile
        for (i = 0; i < numNonTreasure; ++i)
            G->deck[player][G->deckCount[player]++] = adventurer;
        break;
    }
}

// runs a unit test on the game state cleanGame (which this function won't modify) using the given coin bonus and number of copper, silver, gold, and non treasure cards in the player's hand
// the first index of the number of cards is used for player 0, and the second index of number of cards is used for player 1 (regardless if 0 or 1 is given as the player param)
// set addToDiscardOrDeck to 1 to put a bunch of treasure in both player's discard
// set addToDiscardOrDeck to 2 to put a bunch of treasure in both player's deck
void runUpdateCoinsUnitTest(int player, struct gameState *cleanGame, int numCopper[2], int numSilver[2], int numGold[2], int numNonTreasure[2], int bonus, int addToDiscardOrDeck, struct UpdateCoinsTestResultData * testResults)
{
    int i;
    struct gameState testGame, holdGame;

    memcpy(&testGame, cleanGame, sizeof(struct gameState)); // set testGame to a clean game

    for (i = 0; i < 2; ++i)
    {
        setPile(i, &testGame, numCopper[i], numSilver[i], numGold[i], numNonTreasure[i], 0); // fill player i's hand with the number of cards provided
        if (addToDiscardOrDeck == 1)
            setPile(i, &testGame, 10, 20, 30, 40, 1); // fill player i's discard with the number of cards provided
        else if (addToDiscardOrDeck == 2)
            setPile(i, &testGame, 10, 20, 30, 40, 2); // fill player i's deck with the number of cards provided
    }

    memcpy(&holdGame, &testGame, sizeof(struct gameState)); // before call to updateCoins, save state of the testGame (which has 0 coins) to compare state after calling updateCoins

    updateCoins(player, &testGame, bonus); // execute updateCoins

    testResults->actualCoins = testGame.coins; // store actual coins calculated by function

    testResults->expectedCoins = calcExpectedCoins(numCopper[player], numSilver[player], numGold[player], bonus); // calc expected coins for the player

    testGame.coins = 0; // set coins back to zero because that is the one thing we expected to change from game state and we want to compare state with holdState to make sure nothing else changed (hold state has 0 coins)

    // make sure testGame is the same state as holdGame after execution of updateCoins
    if (memcmp(&testGame, &holdGame, sizeof(struct gameState)) == 0)
        testResults->sideEffectsTest = 1;
    else
        testResults->sideEffectsTest = 0;
}

int main()
{
    int seed = 68;
    int i, p, d, n, c, b;
    int numCopper[2];
    int numSilver[2];
    int numGold[2];
    int numNonTreasure[2];
    struct gameState cleanGame;
    struct UpdateCoinsTestResultData testResults; // to hold results from the function that runs the test

    // it doesn't matter what kingdom cards we use to test this function
    int k[10] = { adventurer, council_room, feast, gardens, mine, remodel, smithy, village, baron, great_hall };

    // initialize a clean game state for 2 players
    memset(&cleanGame, 0, sizeof(struct gameState));
    initializeGame(2, k, seed, &cleanGame);
    // also set a clean game to have both players have empty piles (we'll manually put cards in there before the test)
    for (i = 0; i < 2; ++i)
    {
        cleanGame.handCount[i] = 0;
        cleanGame.discardCount[i] = 0;
        cleanGame.deckCount[i] = 0;
        memset(cleanGame.hand[i], 0, MAX_HAND * sizeof(int));
        memset(cleanGame.discard[i], 0, MAX_HAND * sizeof(int));
        memset(cleanGame.deck[i], 0, MAX_HAND * sizeof(int));
    }
    cleanGame.coins = 0; // guaruntees coin count starts at 0 for all tests

    printf("Unit Tests Function 4 - updateCoins():\n");


    // 1. test with 0 for both player's hands but a bunch of arbitary treasure in their discard and deck, and no bonus, to make sure coin count is only affected by the cards in hand

    for (p = 0; p < 2; ++p)
    {
        for (d = 1; d <= 2; ++d)
        {
            numCopper[p] = 0; numSilver[p] = 0; numGold[p] = 0; numNonTreasure[p] = 0; // player getting coins checked
            numCopper[1 - p] = 0; numSilver[1 - p] = 0; numGold[1 - p] = 0; numNonTreasure[1 - p] = 0; // other player

            runUpdateCoinsUnitTest(p, &cleanGame, numCopper, numSilver, numGold, numNonTreasure, 0, d, &testResults);

            if (testResults.expectedCoins == testResults.actualCoins)
                printf("updateCoins(): PASS when checking player %d's coins using no bonus coins and when both players have lots of treasure cards in only their %s (expected coins = %d, actual coins = %d)\n", p, (d == 1) ? "discard" : "deck", testResults.expectedCoins, testResults.actualCoins);
            else
                printf("updateCoins(): FAIL when checking player %d's coins using no bonus coins and when both players have lots of treasure cards in only their %s (expected coins = %d, actual coins = %d)\n", p, (d == 1) ? "discard" : "deck", testResults.expectedCoins, testResults.actualCoins);

            if (testResults.sideEffectsTest == 1)
                printf("updateCoins(): PASS when checking for unintended side effects on the game from the previous test\n");
            else
                printf("updateCoins(): FAIL when checking for unintended side effects on the game from the previous test\n");
        }
    }


    // 2. test with 0-5 non-treasure in the tested player's hand and a bunch of arbitrary treasure in the other player's hand

    for (p = 0; p < 2; ++p)
    {
        for (n = 0; n <= 5; ++n)
        {
            numCopper[p] = 0; numSilver[p] = 0; numGold[p] = 0; numNonTreasure[p] = n; // player getting coins checked
            numCopper[1 - p] = 10; numSilver[1 - p] = 20; numGold[1 - p] = 30; numNonTreasure[1 - p] = 40; // other player

            runUpdateCoinsUnitTest(p, &cleanGame, numCopper, numSilver, numGold, numNonTreasure, 0, 0, &testResults);

            if (testResults.expectedCoins == testResults.actualCoins)
                printf("updateCoins(): PASS when checking player %d's coins using no bonus coins and when player %d has %d non-treasure card(s) in hand while player %d has lots of treasure in hand (expected coins = %d, actual coins = %d)\n", p, p, n, 1 - p, testResults.expectedCoins, testResults.actualCoins);
            else
                printf("updateCoins(): FAIL when checking player %d's coins using no bonus coins and when player %d has %d non-treasure card(s) in hand while player %d has lots of treasure in hand (expected coins = %d, actual coins = %d)\n", p, p, n, 1 - p, testResults.expectedCoins, testResults.actualCoins);

            if (testResults.sideEffectsTest == 1)
                printf("updateCoins(): PASS when checking for unintended side effects on the game from the previous test\n");
            else
                printf("updateCoins(): FAIL when checking for unintended side effects on the game from the previous test\n");
        }
    }


    // 3. test with only 0-5 copper, only 0-5 silver, only 0-5 gold in player's hand and only 0-5 bonus coins, and a lot of treasure in the other player's hand

    for (p = 0; p < 2; ++p)
    {
        for (c = 1; c <= 4; ++c)
        {
            for (n = 0; n <= 5; ++n)
            {
                numCopper[p] = n*((c == 1) ? 1 : 0); numSilver[p] = n*((c == 2) ? 1 : 0); numGold[p] = n*((c == 3) ? 1 : 0); numNonTreasure[p] = 0;  // player getting coins checked
                numCopper[1 - p] = 10; numSilver[1 - p] = 20; numGold[1 - p] = 30; numNonTreasure[1 - p] = 40; // other player
                b = n*((c == 4) ? 1 : 0); // bonus

                runUpdateCoinsUnitTest(p, &cleanGame, numCopper, numSilver, numGold, numNonTreasure, b, 0, &testResults);

                if (testResults.expectedCoins == testResults.actualCoins)
                    printf("updateCoins(): PASS when checking player %d's coins when player %d has only %d %s, while player %d has lots of treasure in hand (expected coins = %d, actual coins = %d)\n", p, p, n, (c == 1) ? "Copper" : ((c == 2) ? "Silver" : ((c == 3) ? "Gold" : "bonus coin(s)")), 1 - p, testResults.expectedCoins, testResults.actualCoins);
                else
                    printf("updateCoins(): FAIL when checking player %d's coins when player %d has only %d %s, while player %d has lots of treasure in hand (expected coins = %d, actual coins = %d)\n", p, p, n, (c == 1) ? "Copper" : ((c == 2) ? "Silver" : ((c == 3) ? "Gold" : "bonus coin(s)")), 1 - p, testResults.expectedCoins, testResults.actualCoins);

                if (testResults.sideEffectsTest == 1)
                    printf("updateCoins(): PASS when checking for unintended side effects on the game from the previous test\n");
                else
                    printf("updateCoins(): FAIL when checking for unintended side effects on the game from the previous test\n");
            }
        }
    }


    // 4. test with 0-5 copper silver gold and bonus coins, and a lot of treasure in the other player's hand

    for (p = 0; p < 2; ++p)
    {
        for (n = 0; n <= 5; ++n)
        {
            numCopper[p] = n; numSilver[p] = n; numGold[p] = n; numNonTreasure[p] = 0;  // player getting coins checked
            numCopper[1 - p] = 10; numSilver[1 - p] = 20; numGold[1 - p] = 30; numNonTreasure[1 - p] = 40; // other player
            b = n; // bonus

            runUpdateCoinsUnitTest(p, &cleanGame, numCopper, numSilver, numGold, numNonTreasure, b, 0, &testResults);

            if (testResults.expectedCoins == testResults.actualCoins)
                printf("updateCoins(): PASS when checking player %d's coins when player %d has %d Copper, %d Silver, %d Gold, and %d bonus coin(s), while player %d has lots of treasure in hand (expected coins = %d, actual coins = %d)\n", p, p, n, n, n, n, 1 - p, testResults.expectedCoins, testResults.actualCoins);
            else
                printf("updateCoins(): FAIL when checking player %d's coins when player %d has %d Copper, %d Silver, %d Gold, and %d bonus coin(s), while player %d has lots of treasure in hand (expected coins = %d, actual coins = %d)\n", p, p, n, n, n, n, 1 - p, testResults.expectedCoins, testResults.actualCoins);

            if (testResults.sideEffectsTest == 1)
                printf("updateCoins(): PASS when checking for unintended side effects on the game from the previous test\n");
            else
                printf("updateCoins(): FAIL when checking for unintended side effects on the game from the previous test\n");
        }
    }

    return 0;
}