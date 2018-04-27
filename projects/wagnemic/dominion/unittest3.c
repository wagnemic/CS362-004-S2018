#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"

/*
 * Unit tests for scoreFor function
 *
 * code inspection of scoreFor shows that it only checks for pile counts and contents of each pile (it's also not supposed to modify the game state), which means that's the only data we need to vary between tests
 * so, we only need to check expected score against actual score (and make sure game state was not modified)
 * the tests used are
 * 1. test the 18 cases where the player has only 1 of each type of 6 cards in each of the 3 possible locations
 * 2. test the 6 cases where there's 1 of the 6 victory cards in every location
 * 3. test the 6 cases where there's a differing number of the 6 victory cards in every location
 * 4. test the 3 cases where there's 1 of all victory cards in each of the 3 locations
 * 5. test the 3 cases where there's a differing number of victory cards in each of the 3 locations
 * 6. test the 4 cases where there's 0,1,2, and 3 of each victory card in every location
 * 7. test the 9 cases where there's 1 garden in each of the 3 locations, and 8, 9, and 10 extra cards (which results in 9,10,11 total cards)
 *
 */

const int VICORY_CARD_VALUES[5] = { -1, 1, 3, 6, 1 }; // gardens is not a constant value, will be calculated on the spot
const int VICTORY_CARDS[6] = { curse, estate, duchy, province, great_hall, gardens };
const char * VICTORY_CARD_NAMES[6] = { "Curse", "Estate", "Duchy", "Province", "Great Hall", "Gardens" };
const char * CARD_LOCATION_NAMES[3] = { "hand", "discard", "deck" };

struct ScoreForTestResultData
{
    int expectedScore; // expected score for the test
    int actualScore; // actual score for the test
    int sideEffectsTest; // 1 if no side effects, 0 if side effects
};

// returns the total number of cards in numVictoryCards and numExtraCards
int calcTotalCards(int numVictoryCards[3][6], int numExtraCards[3])
{
    int i, j;
    int totalCards = 0;

    for (j = 0; j < 3; ++j)
    {
        for (i = 0; i < 6; ++i)
            totalCards += numVictoryCards[j][i];
        totalCards += numExtraCards[j];
    }

    return totalCards;
}

// accepts array of the number of victory cards that the player has in the 3 locations (see def of victoryCards for proper indexing of the cards for the 6-index)
// and a number of extra cards (non victory point cards) in the 3 locations that the player has
// and calculates the expected score in the return value
int calcExpectedScore(int numVictoryCards[3][6], int numExtraCards[3])
{
    int expectedScore = 0;
    int i, j;

    // add score from constant victory cards
    for (j = 0; j < 3; ++j) // for the 3 locations of cards
        for (i = 0; i < 5; ++i) // for all 5 static victory cards
            expectedScore += VICORY_CARD_VALUES[i] * numVictoryCards[j][i];

    // add score for each garden
    expectedScore += (numVictoryCards[0][5] + numVictoryCards[1][5] + numVictoryCards[2][5])* (calcTotalCards(numVictoryCards, numExtraCards) / 10);

    return expectedScore;
}

// fills the player's hand discard and deck with the number of victory cards defined by numVictoryCards
// also puts adventurers into hand discard and deck to match the values provided in numExtraCards
void insertCards(int numVictoryCards[3][6], int numExtraCards[3], int player, struct gameState *G)
{
    int i, j;

    // put victory cards in hand
    for (i = 0; i < 6; ++i)
        for (j = 0; j < numVictoryCards[0][i]; ++j)
            G->hand[player][G->handCount[player]++] = VICTORY_CARDS[i];

    // put extra cards in hand
    for (j = 0; j < numExtraCards[0]; ++j)
        G->hand[player][G->handCount[player]++] = adventurer;


    // put victory cards in discard
    for (i = 0; i < 6; ++i)
        for (j = 0; j < numVictoryCards[1][i]; ++j)
            G->discard[player][G->discardCount[player]++] = VICTORY_CARDS[i];

    // put extra cards in discard
    for (j = 0; j < numExtraCards[1]; ++j)
        G->discard[player][G->discardCount[player]++] = adventurer;

    // put victory cards in deck
    for (i = 0; i < 6; ++i)
        for (j = 0; j < numVictoryCards[2][i]; ++j)
            G->deck[player][G->deckCount[player]++] = VICTORY_CARDS[i];

    // put extra cards in deck
    for (j = 0; j < numExtraCards[2]; ++j)
        G->deck[player][G->deckCount[player]++] = adventurer;
}

// run a unit test on the given game with the given victory cards in the 3 locations and given non-VP cards for the given player
void runScoreForUnitTest(int numVictoryCards[3][6], int numExtraCards[3], int player, struct gameState *cleanGame, struct ScoreForTestResultData *testResults)
{
    struct gameState testGame, holdGame;

    memcpy(&testGame, cleanGame, sizeof(struct gameState)); // set testGame to a clean game

    insertCards(numVictoryCards, numExtraCards, player, &testGame); // put given cards int player's hand, discard, deck in the testGame

    memcpy(&holdGame, &testGame, sizeof(struct gameState)); // before call to scoreFor, save state of the testGame to compare state after calling scoreFor

    testResults->actualScore = scoreFor(player, &testGame); // calc actual score for player

    testResults->expectedScore = calcExpectedScore(numVictoryCards, numExtraCards); // calc expected score based on number of victory and extra cards

    // make sure testGame is the same state as holdGame after execution of scoreFor
    if (memcmp(&testGame, &holdGame, sizeof(struct gameState)) == 0)
        testResults->sideEffectsTest = 1;
    else
        testResults->sideEffectsTest = 0;

}

int main()
{
    int seed = 68;
    int v, l, e, t;
    int numVictoryCards[3][6]; // the 3 is for each location possible of victory cards (hand, discard, deck is 0, 1, 2) so [1][2] is the number of duchy to put in the player's discard pile
    int numExtraCards[3]; // number of adventurers put in a players hand, discard, deck
    int numPlayers = 2;
    int scoredPlayer = 0; // only need to do score for one player to test the function
    struct gameState cleanGame;
    struct ScoreForTestResultData testResults; // to hold results from the function that runs the test

    // just need to use this set of kingdom cards because only gardens and great_hall add victory points
    int k[10] = { adventurer, council_room, feast, gardens, mine, remodel, smithy, village, baron, great_hall };

    printf("Unit Tests Function 3 - scoreFor():\n");

    // initialize a clean game state for numPlayers players
    memset(&cleanGame, 0, sizeof(struct gameState));
    initializeGame(numPlayers, k, seed, &cleanGame);
    // also set a clean game to have the scored player have 0 cards in hand, discard, deck (we'll fill them manually)
    cleanGame.handCount[scoredPlayer] = 0;
    cleanGame.discardCount[scoredPlayer] = 0;
    cleanGame.deckCount[scoredPlayer] = 0;


    // 1. test the 18 cases where the player has only 1 of each type of 6 cards in each of the 3 possible locations
    for (l = 0; l < 3; ++l)
        for (v = 0; v < 6; ++v)
        {
            // just one victory card in the [l][v] index
            memset(numVictoryCards, 0, 3 * 6 * sizeof(int));
            numVictoryCards[l][v] = 1;

            memset(numExtraCards, 0, 3 * sizeof(int)); // no extra cards

            memset(&testResults, 0, sizeof(struct ScoreForTestResultData)); // clear test results (unit test function assigns every field anyway)

            runScoreForUnitTest(numVictoryCards, numExtraCards, scoredPlayer, &cleanGame, &testResults);

            if (testResults.expectedScore == testResults.actualScore)
                printf("scoreFor(): PASS when checking player score with 1 %s in their %s (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[v], CARD_LOCATION_NAMES[l], testResults.expectedScore, testResults.actualScore);
            else
                printf("scoreFor(): FAIL when checking player score with 1 %s in their %s (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[v], CARD_LOCATION_NAMES[l], testResults.expectedScore, testResults.actualScore);

            if (testResults.sideEffectsTest == 1)
                printf("scoreFor(): PASS when checking for unintended side effects on the game from the previous test\n");
            else
                printf("scoreFor(): FAIL when checking for unintended side effects on the game from the previous test\n");

        }

    // 2. test the 6 cases where there's 1 of the 6 victory cards in every location
    for (v = 0; v < 6; ++v)
    {
        // 3 total victory cards in the [:][v] column (1 in each location)
        memset(numVictoryCards, 0, 3 * 6 * sizeof(int));
        for (l = 0; l < 3; ++l)
            numVictoryCards[l][v] = 1;

        memset(numExtraCards, 0, 3 * sizeof(int)); // no extra cards

        memset(&testResults, 0, sizeof(struct ScoreForTestResultData)); // clear test results (unit test function assigns every field anyway)

        runScoreForUnitTest(numVictoryCards, numExtraCards, scoredPlayer, &cleanGame, &testResults);

        if (testResults.expectedScore == testResults.actualScore)
            printf("scoreFor(): PASS when checking player score with 1 %s each of the 3 piles (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[v], testResults.expectedScore, testResults.actualScore);
        else
            printf("scoreFor(): FAIL when checking player score with 1 %s each of the 3 piles (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[v], testResults.expectedScore, testResults.actualScore);

        if (testResults.sideEffectsTest == 1)
            printf("scoreFor(): PASS when checking for unintended side effects on the game from the previous test\n");
        else
            printf("scoreFor(): FAIL when checking for unintended side effects on the game from the previous test\n");

    }


    // 3. test the 6 cases where there's a differing number of the 6 victory cards in every location
    for (v = 0; v < 6; ++v)
    {
        // 6 total victory cards in the [:][v] column (1 in hand, 2 in discard, 3 in deck)
        memset(numVictoryCards, 0, 3 * 6 * sizeof(int));
        for (l = 0; l < 3; ++l)
            numVictoryCards[l][v] = l + 1;

        memset(numExtraCards, 0, 3 * sizeof(int)); // no extra cards

        memset(&testResults, 0, sizeof(struct ScoreForTestResultData)); // clear test results (unit test function assigns every field anyway)

        runScoreForUnitTest(numVictoryCards, numExtraCards, scoredPlayer, &cleanGame, &testResults);

        if (testResults.expectedScore == testResults.actualScore)
            printf("scoreFor(): PASS when checking player score with 1 %s in their %s, 2 in their %s, and 3 in their %s (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[v], CARD_LOCATION_NAMES[0], CARD_LOCATION_NAMES[1], CARD_LOCATION_NAMES[2], testResults.expectedScore, testResults.actualScore);
        else
            printf("scoreFor(): FAIL when checking player score with 1 %s in their %s, 2 in their %s, and 3 in their %s (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[v], CARD_LOCATION_NAMES[0], CARD_LOCATION_NAMES[1], CARD_LOCATION_NAMES[2], testResults.expectedScore, testResults.actualScore);

        if (testResults.sideEffectsTest == 1)
            printf("scoreFor(): PASS when checking for unintended side effects on the game from the previous test\n");
        else
            printf("scoreFor(): FAIL when checking for unintended side effects on the game from the previous test\n");

    }


    // 4. test the 3 cases where there's 1 of all victory cards in each of the 3 locations
    for (l = 0; l < 3; ++l)
    {
        // 6 total victory cards in the [l][:] row
        memset(numVictoryCards, 0, 3 * 6 * sizeof(int));
        for (v = 0; v < 6; ++v)
            numVictoryCards[l][v] = 1;

        memset(numExtraCards, 0, 3 * sizeof(int)); // no extra cards

        memset(&testResults, 0, sizeof(struct ScoreForTestResultData)); // clear test results (unit test function assigns every field anyway)

        runScoreForUnitTest(numVictoryCards, numExtraCards, scoredPlayer, &cleanGame, &testResults);

        if (testResults.expectedScore == testResults.actualScore)
            printf("scoreFor(): PASS when checking player score with 1 each of the 6 VP cards in their %s (expected score = %d, actual score = %d)\n", CARD_LOCATION_NAMES[l], testResults.expectedScore, testResults.actualScore);
        else
            printf("scoreFor(): FAIL when checking player score with 1 each of the 6 VP cards in their %s (expected score = %d, actual score = %d)\n", CARD_LOCATION_NAMES[l], testResults.expectedScore, testResults.actualScore);

        if (testResults.sideEffectsTest == 1)
            printf("scoreFor(): PASS when checking for unintended side effects on the game from the previous test\n");
        else
            printf("scoreFor(): FAIL when checking for unintended side effects on the game from the previous test\n");

    }

    // 5. test the 3 cases where there's a differing number of victory cards in each of the 3 locations
    for (l = 0; l < 3; ++l)
    {
        // 21 total victory cards in the [l][:] row (1 curse, 2 duchy... 6 gardens)
        memset(numVictoryCards, 0, 3 * 6 * sizeof(int));
        for (v = 0; v < 6; ++v)
            numVictoryCards[l][v] = v + 1;

        memset(numExtraCards, 0, 3 * sizeof(int)); // no extra cards

        memset(&testResults, 0, sizeof(struct ScoreForTestResultData)); // clear test results (unit test function assigns every field anyway)

        runScoreForUnitTest(numVictoryCards, numExtraCards, scoredPlayer, &cleanGame, &testResults);

        if (testResults.expectedScore == testResults.actualScore)
            printf("scoreFor(): PASS when checking player score with 1 %s, 2 %s, 3 %s, 4 %s, 5 %s, and 6 %s in their %s (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[0], VICTORY_CARD_NAMES[1], VICTORY_CARD_NAMES[2], VICTORY_CARD_NAMES[3], VICTORY_CARD_NAMES[4], VICTORY_CARD_NAMES[5], CARD_LOCATION_NAMES[l], testResults.expectedScore, testResults.actualScore);
        else
            printf("scoreFor(): FAIL when checking player score with 1 %s, 2 %s, 3 %s, 4 %s, 5 %s, and 6 %s in their %s (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[0], VICTORY_CARD_NAMES[1], VICTORY_CARD_NAMES[2], VICTORY_CARD_NAMES[3], VICTORY_CARD_NAMES[4], VICTORY_CARD_NAMES[5], CARD_LOCATION_NAMES[l], testResults.expectedScore, testResults.actualScore);

        if (testResults.sideEffectsTest == 1)
            printf("scoreFor(): PASS when checking for unintended side effects on the game from the previous test\n");
        else
            printf("scoreFor(): FAIL when checking for unintended side effects on the game from the previous test\n");

    }


    // 6. test the 4 cases where there's 0,1,2, and 3 of each victory card in every location
    for (t = 0; t < 4; ++t)
    {
        // 18*t total victory cards, t in each index of the array
        memset(numVictoryCards, 0, 3 * 6 * sizeof(int));
        for (v = 0; v < 6; ++v)
            for (l = 0; l < 3; ++l)
                numVictoryCards[l][v] = t;

        memset(numExtraCards, 0, 3 * sizeof(int)); // no extra cards

        memset(&testResults, 0, sizeof(struct ScoreForTestResultData)); // clear test results (unit test function assigns every field anyway)

        runScoreForUnitTest(numVictoryCards, numExtraCards, scoredPlayer, &cleanGame, &testResults);

        if (testResults.expectedScore == testResults.actualScore)
            printf("scoreFor(): PASS when checking player score with %d of each VP card in each pile (expected score = %d, actual score = %d)\n", t, testResults.expectedScore, testResults.actualScore);
        else
            printf("scoreFor(): FAIL when checking player score with %d of each VP card in each pile (expected score = %d, actual score = %d)\n", t, testResults.expectedScore, testResults.actualScore);

        if (testResults.sideEffectsTest == 1)
            printf("scoreFor(): PASS when checking for unintended side effects on the game from the previous test\n");
        else
            printf("scoreFor(): FAIL when checking for unintended side effects on the game from the previous test\n");

    }

    // 7. test the 9 cases where there's 1 garden in each of the 3 locations, and 8, 9, and 10 extra cards (which results in 9,10,11 total cards)

    for (l = 0; l < 3; ++l)
    {
        for (e = 0; e < 3; ++e)
        {
            // 1 garden in location l
            memset(numVictoryCards, 0, 3 * 6 * sizeof(int));

            numVictoryCards[l][5] = 1;

            // distribute 8, 9, and 10 cards for each of the 3 cases into extra cards
            numExtraCards[0] = 3 + ((e > 1) ? 1 : 0); // add one when e is 2
            numExtraCards[1] = 3;
            numExtraCards[2] = 2 + ((e > 0) ? 1 : 0); // add 1 when e is 1 or 2

            memset(&testResults, 0, sizeof(struct ScoreForTestResultData)); // clear test results (unit test function assigns every field anyway)

            runScoreForUnitTest(numVictoryCards, numExtraCards, scoredPlayer, &cleanGame, &testResults);

            if (testResults.expectedScore == testResults.actualScore)
                printf("scoreFor(): PASS when checking player score with 1 %s in their %s and %d non-VP cards distributed among all piles, which is %d total cards (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[5], CARD_LOCATION_NAMES[l], numExtraCards[0] + numExtraCards[1] + numExtraCards[2], calcTotalCards(numVictoryCards, numExtraCards), testResults.expectedScore, testResults.actualScore);
            else
                printf("scoreFor(): FAIL when checking player score with 1 %s in their %s and %d non-VP cards distributed among all piles, which is %d total cards (expected score = %d, actual score = %d)\n", VICTORY_CARD_NAMES[5], CARD_LOCATION_NAMES[l], numExtraCards[0] + numExtraCards[1] + numExtraCards[2], calcTotalCards(numVictoryCards, numExtraCards), testResults.expectedScore, testResults.actualScore);

            if (testResults.sideEffectsTest == 1)
                printf("scoreFor(): PASS when checking for unintended side effects on the game from the previous test\n");
            else
                printf("scoreFor(): FAIL when checking for unintended side effects on the game from the previous test\n");
        }
    }

    return 0;
}