#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "rngs.h"

/* Unit tests for the Adventurer card
 *
 * The Adventurer card is supposed to put two treasures from the player's deck into their hand and discard all non treasures drawn
 * if the deck becomes empty, the function continues by shuffling the discard pile and creating a new deck (but not including the previously drawn non treausure cards)
 * if it gets through the entire deck and discard without finding 2 treasures, the player just gets as many treasures as they could find
 * then adventurer gets discarded and the non treasure cards also finally get placed into the discard pile
 * the purpose of these tests is to check this functionality and get good coverage of the function
 * the tests are:
 * Test 1: Use Adventurer from player 0's hand position 1 to draw a Copper and Silver from the top of the deck
 * Test 2: Use Adventurer from player 0's hand position 0 to draw a Gold and Silver from the bottom of the deck
 * Test 3: Use Adventurer from player 0's hand position 2 to draw a Copper from their deck and one from the discard after a shuffle (this test depends on seed value)
 * Test 4: Use Adventurer from player 0's hand position 2 to draw a Copper from their deck, but a second treasure does not exist in their deck or disard
 * Test 5: Use Adventurer from player 0's hand position 2 to attempt to draw a treasure that doesn't exist, but also set numBuys to 6 (Gold) to ensure the buggy function doesn't loop infinitely
 *
 */

void cardNumToName(int card, char *name);
int compare(const void* a, const void* b);

// puts the player's hand from game state into the buffer like [card1][card2] ... etc
void storeHandContents(int player, struct gameState*G, char * buffer)
{
    int i;
    char name[100];
    char temp[100];

    buffer[0] = 0; // clear out c-str by making first byte a null byte

    // when pile is empty
    if (G->handCount[player] <= 0)
    {
        sprintf(buffer, "%s", "[]");
        return;
    }

    for (i = 0; i < G->handCount[player]; ++i)
    {
        cardNumToName(G->hand[player][i], name); // save name in name
        sprintf(temp, "[%s]", name); // temp is now [Card Name]
        strcat(buffer, temp); // concat [Card Name] to end of buffer
    }
}

// puts the player's deck from game state into the buffer like [card1][card2] ... etc
void storeDeckContents(int player, struct gameState*G, char * buffer)
{
    int i;
    char name[100];
    char temp[100];

    buffer[0] = 0; // clear out c-str by making first byte a null byte

    // when pile is empty
    if (G->deckCount[player] <= 0)
    {
        sprintf(buffer, "%s", "[]");
        return;
    }

    for (i = 0; i < G->deckCount[player]; ++i)
    {
        cardNumToName(G->deck[player][i], name); // save name in name
        sprintf(temp, "[%s]", name); // temp is now [Card Name]
        strcat(buffer, temp); // concat [Card Name] to end of buffer
    }
}

// puts the player's discard from game state into the buffer like [card1][card2] ... etc
void storeDiscardContents(int player, struct gameState*G, char * buffer)
{
    int i;
    char name[100];
    char temp[100];

    buffer[0] = 0; // clear out c-str by making first byte a null byte

    // when pile is empty
    if (G->discardCount[player] <= 0)
    {
        sprintf(buffer, "%s", "[]");
        return;
    }

    for (i = 0; i < G->discardCount[player]; ++i)
    {
        cardNumToName(G->discard[player][i], name); // save name in name
        sprintf(temp, "[%s]", name); // temp is now [Card Name]
        strcat(buffer, temp); // concat [Card Name] to end of buffer
    }
}

// return 1 if player's unordered hand contents are different (including different counts)
// returns 0 if player's unordered hand contents are the same
int areHandsDifferent(int player, struct gameState *G1, struct gameState *G2)
{
    int i;
    int tempPile1[MAX_DECK];
    int tempPile2[MAX_DECK];

    // check for count difference
    if (G1->handCount[player] != G2->handCount[player])
        return 1;

    // fill temp piles
    for (i = 0; i < G1->handCount[player]; ++i)
    {
        tempPile1[i] = G1->hand[player][i];
        tempPile2[i] = G2->hand[player][i];
    }

    // sort temp piles
    qsort((void*)(tempPile1), G1->handCount[player], sizeof(int), compare);
    qsort((void*)(tempPile2), G2->handCount[player], sizeof(int), compare);

    // check for contents difference
    for (i = 0; i < G1->handCount[player]; ++i)
        if (tempPile1[i] != tempPile2[i])
            return 1;

    return 0;
}

// return 1 if player's unordered deck contents are different (including different counts)
// returns 0 if player's unordered deck contents are the same
int areDecksDifferent(int player, struct gameState *G1, struct gameState *G2)
{
    int i;
    int tempPile1[MAX_DECK];
    int tempPile2[MAX_DECK];

    // check for count difference
    if (G1->deckCount[player] != G2->deckCount[player])
        return 1;

    // fill temp piles
    for (i = 0; i < G1->deckCount[player]; ++i)
    {
        tempPile1[i] = G1->deck[player][i];
        tempPile2[i] = G2->deck[player][i];
    }

    // sort temp piles
    qsort((void*)(tempPile1), G1->deckCount[player], sizeof(int), compare);
    qsort((void*)(tempPile2), G2->deckCount[player], sizeof(int), compare);

    // check for contents difference
    for (i = 0; i < G1->deckCount[player]; ++i)
        if (tempPile1[i] != tempPile2[i])
            return 1;

    return 0;
}

// return 1 if player's unordered discard contents are different (including different counts)
// returns 0 if player's unordered discard contents are the same
int areDiscardsDifferent(int player, struct gameState *G1, struct gameState *G2)
{
    int i;
    int tempPile1[MAX_DECK];
    int tempPile2[MAX_DECK];

    // check for count difference
    if (G1->discardCount[player] != G2->discardCount[player])
        return 1;

    // fill temp piles
    for (i = 0; i < G1->discardCount[player]; ++i)
    {
        tempPile1[i] = G1->discard[player][i];
        tempPile2[i] = G2->discard[player][i];
    }

    // sort temp piles
    qsort((void*)(tempPile1), G1->discardCount[player], sizeof(int), compare);
    qsort((void*)(tempPile2), G2->discardCount[player], sizeof(int), compare);

    // check for contents difference
    for (i = 0; i < G1->discardCount[player]; ++i)
        if (tempPile1[i] != tempPile2[i])
            return 1;

    return 0;
}

// returns 0 if the state of the player in both game states is the same, and 1 if anything is different
// a "player state" is the cards in the player's deck, hand, and discard as well as their counts for those piles
int anyChangeInPlayerState(int player, struct gameState *G1, struct gameState *G2)
{
    if (areDecksDifferent(player, G1, G2) == 1)
        return 1;

    if (areHandsDifferent(player, G1, G2) == 1)
        return 1;

    if (areDiscardsDifferent(player, G1, G2) == 1)
        return 1;

    // passed all checks
    return 0;
}

// returns 0 if there's no difference in supply counts between the two game states
// returns 1 if there's any difference in supply counts between the two game states
int anyChangeInSupplies(struct gameState *G1, struct gameState *G2)
{
    int i;

    // return 0 when any of the supply counts are not the same over all 27 supplies
    for (i = 0; i < treasure_map + 1; ++i)
        if (G1->supplyCount[i] != G2->supplyCount[i])
            return 1;

    // passed all checks
    return 0;
}

// runs the test for checking player 1 state change between games as well as supply state change between games
void testPlayer1AndSupplyStateChanges(struct gameState *G1, struct gameState *G2)
{
    // check for any change to player 1 state
    if (anyChangeInPlayerState(1, G1, G2) == 0)
        printf("PASS when checking if player 1's state did not change\n");
    else
        printf("FAIL when checking if player 1's state did not change\n");

    // check for any change to supplies
    if (anyChangeInSupplies(G1, G2) == 0)
        printf("PASS when checking if all card supplies were unchanged\n");
    else
        printf("FAIL when checking if all card supplies were unchanged\n");
}

// carries out the test to check if the expected games state for player 0 is the same as in the actual game, includes print outs
void testPlayer0PileContents(struct gameState *expectedGame, struct gameState *actualGame, struct gameState *beforeGame)
{
    char handBuffer_expected[1000];
    char deckBuffer_expected[1000];
    char discardBuffer_expected[1000];
    char handBuffer_actual[1000];
    char deckBuffer_actual[1000];
    char discardBuffer_actual[1000];
    char handBuffer_before[1000];
    char deckBuffer_before[1000];
    char discardBuffer_before[1000];

    storeHandContents(0, actualGame, handBuffer_actual); // get c-str of actual hand
    storeDeckContents(0, actualGame, deckBuffer_actual); // get c-str of actual deck
    storeDiscardContents(0, actualGame, discardBuffer_actual); // get c-str of actual discard
    storeHandContents(0, expectedGame, handBuffer_expected); // get c-str of expected hand
    storeDeckContents(0, expectedGame, deckBuffer_expected); // get c-str of expected deck
    storeDiscardContents(0, expectedGame, discardBuffer_expected); // get c-str of expected discard
    storeHandContents(0, beforeGame, handBuffer_before); // get c-str of before hand
    storeDeckContents(0, beforeGame, deckBuffer_before); // get c-str of before deck
    storeDiscardContents(0, beforeGame, discardBuffer_before); // get c-str of before discard

    // print out the before state of player 0 cards
    printf("Player 0's hand before cardEffect call: %s\n", handBuffer_before);
    printf("Player 0's deck before cardEffect call: %s\n", deckBuffer_before);
    printf("Player 0's discard before cardEffect call: %s\n", discardBuffer_before);

    // check contents and count of hand
    if (expectedGame->handCount[0] == actualGame->handCount[0])
        printf("PASS when checking player 0's hand count (expected = %d, actual = %d)\n", expectedGame->handCount[0], actualGame->handCount[0]);
    else
        printf("FAIL when checking player 0's hand count (expected = %d, actual = %d)\n", expectedGame->handCount[0], actualGame->handCount[0]);

    if (areHandsDifferent(0, actualGame, expectedGame) == 0)
        printf("PASS when checking player 0's unordered hand contents (expected = %s, actual = %s)\n", handBuffer_expected, handBuffer_actual);
    else
        printf("FAIL when checking player 0's unordered hand contents (expected = %s, actual = %s)\n", handBuffer_expected, handBuffer_actual);

    // check contents and count of deck
    if (expectedGame->deckCount[0] == actualGame->deckCount[0])
        printf("PASS when checking player 0's deck count (expected = %d, actual = %d)\n", expectedGame->deckCount[0], actualGame->deckCount[0]);
    else
        printf("FAIL when checking player 0's deck count (expected = %d, actual = %d)\n", expectedGame->deckCount[0], actualGame->deckCount[0]);

    if (areDecksDifferent(0, actualGame, expectedGame) == 0)
        printf("PASS when checking player 0's unordered deck contents (expected = %s, actual = %s)\n", deckBuffer_expected, deckBuffer_actual);
    else
        printf("FAIL when checking player 0's unordered deck contents (expected = %s, actual = %s)\n", deckBuffer_expected, deckBuffer_actual);

    // check contents and count ofdiscard
    if (expectedGame->discardCount[0] == actualGame->discardCount[0])
        printf("PASS when checking player 0's discard count (expected = %d, actual = %d)\n", expectedGame->discardCount[0], actualGame->discardCount[0]);
    else
        printf("FAIL when checking player 0's discard count (expected = %d, actual = %d)\n", expectedGame->discardCount[0], actualGame->discardCount[0]);

    if (areDiscardsDifferent(0, actualGame, expectedGame) == 0)
        printf("PASS when checking player 0's unordered discard contents (expected = %s, actual = %s)\n", discardBuffer_expected, discardBuffer_actual);
    else
        printf("FAIL when checking player 0's unordered discard contents (expected = %s, actual = %s)\n", discardBuffer_expected, discardBuffer_actual);
}

// performs the test to check if buys changed before/after the cardEffect call
void testBuysChange(struct gameState *expectedGame, struct gameState *actualGame)
{
    if (expectedGame->numBuys == actualGame->numBuys)
        printf("PASS when checking if numBuys was unmodified (expected = %d, actual = %d)\n", expectedGame->numBuys, actualGame->numBuys);
    else
        printf("FAIL when checking if numBuys was unmodified (expected = %d, actual = %d)\n", expectedGame->numBuys, actualGame->numBuys);
}

// performs the test to check if actions changed before/after the cardEffect call
void testActionsChange(struct gameState *expectedGame, struct gameState *actualGame)
{
    if (expectedGame->numActions == actualGame->numActions)
        printf("PASS when checking if numActions was unmodified (expected = %d, actual = %d)\n", expectedGame->numActions, actualGame->numActions);
    else
        printf("FAIL when checking if numActions was unmodified (expected = %d, actual = %d)\n", expectedGame->numActions, actualGame->numActions);
}

// performs the test to check if coins changed before/after the cardEffect call
void testCoinsChange(struct gameState *expectedGame, struct gameState *actualGame)
{
    if (expectedGame->coins == actualGame->coins)
        printf("PASS when checking if coins was unmodified (expected = %d, actual = %d)\n", expectedGame->coins, actualGame->coins);
    else
        printf("FAIL when checking if coins was unmodified (expected = %d, actual = %d)\n", expectedGame->coins, actualGame->coins);
}

// performs check and printout comparing expected and actual return values
void testReturnValue(int cardEffectReturnExpected, int cardEffectReturnActual)
{
    if (cardEffectReturnExpected == cardEffectReturnActual)
        printf("PASS when checking cardEffect return value (expected = %d, actual = %d)\n", cardEffectReturnExpected, cardEffectReturnActual);
    else
        printf("FAIL when checking cardEffect return value (expected = %d, actual = %d)\n", cardEffectReturnExpected, cardEffectReturnActual);
}

// combines all tests for the adventurer card into one function, so i don't have to copy paste them a bunch
void runAdventurerTests(struct gameState *expectedGame, struct gameState *actualGame, struct gameState *beforeGame, int cardEffectReturnExpected, int cardEffectReturnActual)
{
    testPlayer0PileContents(expectedGame, actualGame, beforeGame); // run tests on player 0 state

    testReturnValue(cardEffectReturnExpected, cardEffectReturnActual); // test return values

    // boiler-plate for checking if side-effects occurred
    testPlayer1AndSupplyStateChanges(beforeGame, actualGame);
    testBuysChange(beforeGame, actualGame);
    testActionsChange(beforeGame, actualGame);
    testCoinsChange(beforeGame, actualGame);
}

int main()
{
    int seed = 67;
    int i;
    int cardEffectReturnExpected, cardEffectReturnActual;
    struct gameState cleanGame, actualGame, beforeGame, expectedGame;

    // kingdom card set for the game (enums 7-16, inclusive)
    int k[10] = { adventurer, council_room, feast, gardens, mine, remodel, smithy, village, baron, great_hall };

    printf("Unit Tests Card 1 - Adventurer:\n");

    // initialize a clean game state for kingdom set s and 2 players
    memset(&cleanGame, 0, sizeof(struct gameState));
    initializeGame(2, k, seed, &cleanGame);

    // set all cards for player 0 to 26 (treasure map, so if we start seeing this that means the function looked past what it was supposed to)
    for (i = 0; i < MAX_HAND; ++i)
    {
        cleanGame.hand[0][i] = 26;
        cleanGame.discard[0][i] = 26;
        cleanGame.deck[0][i] = 26;
    }


    // test 1
    printf("Test 1: Use Adventurer from player 0's hand position 1 to draw a Copper and Silver from the top of the deck\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = feast;
    actualGame.hand[0][actualGame.handCount[0]++] = adventurer;
    actualGame.hand[0][actualGame.handCount[0]++] = mine;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = remodel;
    actualGame.deck[0][actualGame.deckCount[0]++] = gardens;
    actualGame.deck[0][actualGame.deckCount[0]++] = silver;
    actualGame.deck[0][actualGame.deckCount[0]++] = copper;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;


    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(adventurer, 0, 0, 0, &actualGame, 1, NULL); // choices and bonus are not used in adventurer
    cardEffectReturnExpected = 0;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = feast;
    expectedGame.hand[0][expectedGame.handCount[0]++] = mine;
    expectedGame.hand[0][expectedGame.handCount[0]++] = silver;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;

    expectedGame.deckCount[0] = 0;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = remodel;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = gardens;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = adventurer;

    runAdventurerTests(&expectedGame, &actualGame, &beforeGame, cardEffectReturnExpected, cardEffectReturnActual);


    // test 2
    printf("Test 2: Use Adventurer from player 0's hand position 0 to draw a Gold and Silver from the bottom of the deck\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = adventurer;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = gold;
    actualGame.deck[0][actualGame.deckCount[0]++] = silver;
    actualGame.deck[0][actualGame.deckCount[0]++] = remodel;
    actualGame.deck[0][actualGame.deckCount[0]++] = gardens;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(adventurer, 0, 0, 0, &actualGame, 0, NULL); // choices and bonus are not used in adventurer
    cardEffectReturnExpected = 0;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = gold;
    expectedGame.hand[0][expectedGame.handCount[0]++] = silver;

    expectedGame.deckCount[0] = 0;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = remodel;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = gardens;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = adventurer;

    runAdventurerTests(&expectedGame, &actualGame, &beforeGame, cardEffectReturnExpected, cardEffectReturnActual);



    // test 3 
    printf("Test 3: Use Adventurer from player 0's hand position 2 to draw a Copper from their deck and one from the discard after a shuffle (this test depends on seed value)\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = feast;
    actualGame.hand[0][actualGame.handCount[0]++] = mine;
    actualGame.hand[0][actualGame.handCount[0]++] = adventurer;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = remodel;
    actualGame.deck[0][actualGame.deckCount[0]++] = copper;
    actualGame.deck[0][actualGame.deckCount[0]++] = gardens;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = copper;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(adventurer, 0, 0, 0, &actualGame, 2, NULL); // choices and bonus are not used in adventurer
    cardEffectReturnExpected = 0;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = feast;
    expectedGame.hand[0][expectedGame.handCount[0]++] = mine;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;

    expectedGame.deckCount[0] = 0;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = great_hall;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = village;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = remodel;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = gardens;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = adventurer;

    runAdventurerTests(&expectedGame, &actualGame, &beforeGame, cardEffectReturnExpected, cardEffectReturnActual);



    // test 4 
    printf("Test 4: Use Adventurer from player 0's hand position 2 to draw a Copper from their deck, but a second treasure does not exist in their deck or disard\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = feast;
    actualGame.hand[0][actualGame.handCount[0]++] = mine;
    actualGame.hand[0][actualGame.handCount[0]++] = adventurer;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = copper;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(adventurer, 0, 0, 0, &actualGame, 2, NULL); // choices and bonus are not used in adventurer
    cardEffectReturnExpected = 0;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = feast;
    expectedGame.hand[0][expectedGame.handCount[0]++] = mine;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;

    expectedGame.deckCount[0] = 0;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = adventurer;

    runAdventurerTests(&expectedGame, &actualGame, &beforeGame, cardEffectReturnExpected, cardEffectReturnActual);




    // test 5 
    printf("Test 5: Use Adventurer from player 0's hand position 2 to attempt to draw a treasure that doesn't exist, but also set numBuys to 6 (Gold) to ensure the buggy function doesn't loop infinitely\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = feast;
    actualGame.hand[0][actualGame.handCount[0]++] = mine;
    actualGame.hand[0][actualGame.handCount[0]++] = adventurer;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    // set numBuys to gold value
    actualGame.numBuys = gold;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(adventurer, 0, 0, 0, &actualGame, 2, NULL); // choices and bonus are not used in adventurer
    cardEffectReturnExpected = 0;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = feast;
    expectedGame.hand[0][expectedGame.handCount[0]++] = mine;

    expectedGame.deckCount[0] = 0;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = adventurer;

    runAdventurerTests(&expectedGame, &actualGame, &beforeGame, cardEffectReturnExpected, cardEffectReturnActual);

    return 0;
}
