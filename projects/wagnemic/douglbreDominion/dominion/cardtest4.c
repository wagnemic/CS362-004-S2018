#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "rngs.h"

/* Unit tests for the Embargo card
 *
 * The Embargo card is supposed to give the player 2 coins more than they have when they play it and add an embargo token to a supply of their choice
 * the effect of the embargo token doesn't add any curses, those are gained when an embargoed supply is gained, which would be in tests of the buyCard function, not in tests of cardEffect
 * the embargo card must be trashed when it is played
 * if the selected supply is not in the game, the effect should fail and the embargo card not be trashed
 * the purpose of these tests is to check this functionality and get good coverage of the function
 * the tests are:
 * Test 1: Use Embargo from player 0's hand position 0 to trash it, gain 2 coins (from 0 coins), and add 1 Embargo Token to the Province supply which already contains 0 Embargo Tokens
 * Test 2: Use Embargo from player 0's hand position 3 to trash it, gain 2 coins (from 0 coins), and add 1 Embargo Token to the Gardens supply which already contains 3 Embargo Tokens
 * Test 3: Use Embargo from player 0's hand position 1 to trash it, gain 2 coins (from 5 coins), and add 1 Embargo Token to the Gold supply which already contains 0 Embargo Tokens
 * Test 4: Use Embargo from player 0's hand position 2 to attempt to trash it, gain 2 coins (from 0 coins), and add 1 Embargo Token to the Sea Hag supply which is not in the game
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

// returns 0 if there's no difference in embargo tokens counts between the two game states
// returns 1 if there's any difference in embargo tokens counts between the two game states
// also ignores checking embargo tokens change for the given supply
int anyChangeInEmbargoTokensExceptOne(struct gameState *G1, struct gameState *G2, int doNotCheckThisSupply)
{
    int i;

    // return 0 when any of the embargo tokens counts (except the provided one) are not the same over all 27 supplies
    for (i = 0; i < treasure_map + 1; ++i)
    {
        if (i == doNotCheckThisSupply)
            continue;
        if (G1->embargoTokens[i] != G2->embargoTokens[i])
            return 1;
    }
    // passed all checks
    return 0;
}

// performs the tests to check for state changes between the two provided games for player 1, the supply counts, and embargo tokens (except for the provided one)
void testPlayer1AndSupplyAndEmbargoTokenChangesExceptGivenEmbargoTokens(struct gameState *G1, struct gameState *G2, int doNotCheckThisSupply)
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

    // check for any change to embargo tokens except the provided one
    if (anyChangeInEmbargoTokensExceptOne(G1, G2, doNotCheckThisSupply) == 0)
        printf("PASS when checking if all embargo tokens that were not supposed to change were unchanged\n");
    else
        printf("FAIL when checking if all embargo tokens that were not supposed to change were unchanged\n");
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

// performs check and printout comparing expected and actual return values
void testReturnValue(int cardEffectReturnExpected, int cardEffectReturnActual)
{
    if (cardEffectReturnExpected == cardEffectReturnActual)
        printf("PASS when checking cardEffect return value (expected = %d, actual = %d)\n", cardEffectReturnExpected, cardEffectReturnActual);
    else
        printf("FAIL when checking cardEffect return value (expected = %d, actual = %d)\n", cardEffectReturnExpected, cardEffectReturnActual);
}

// combines all tests for the embargo card into one function, so i don't have to copy paste them a bunch
void runEmbargoTests(struct gameState *expectedGame, struct gameState *actualGame, struct gameState *beforeGame, int supplyToEmbargo, int cardEffectReturnExpected, int cardEffectReturnActual)
{
    char name[100];
    cardNumToName(supplyToEmbargo, name); // get name of supply for output
    printf("Embargo Token count on %s supply before cardEffect call: %d\n", name, beforeGame->embargoTokens[supplyToEmbargo]);

    printf("Coins before cardEffect call: %d\n", beforeGame->coins);

    testPlayer0PileContents(expectedGame, actualGame, beforeGame); // run tests on player 0 state

    // check for expected vs actual on embargo tokens
    if (expectedGame->embargoTokens[supplyToEmbargo] == actualGame->embargoTokens[supplyToEmbargo])
        printf("PASS when checking Embargo Token count on %s supply (expected = %d, actual = %d)\n", name, expectedGame->embargoTokens[supplyToEmbargo], actualGame->embargoTokens[supplyToEmbargo]);
    else
        printf("FAIL when checking Embargo Token count on %s supply (expected = %d, actual = %d)\n", name, expectedGame->embargoTokens[supplyToEmbargo], actualGame->embargoTokens[supplyToEmbargo]);

    // check for expected vs actual coins
    if (expectedGame->coins == actualGame->coins)
        printf("PASS when checking coins (expected = %d, actual = %d)\n", expectedGame->coins, actualGame->coins);
    else
        printf("FAIL when checking coins (expected = %d, actual = %d)\n", expectedGame->coins, actualGame->coins);
    
    testReturnValue(cardEffectReturnExpected, cardEffectReturnActual); // test return values

    // boiler-plate for checking if side-effects occurred
    testPlayer1AndSupplyAndEmbargoTokenChangesExceptGivenEmbargoTokens(beforeGame, actualGame, supplyToEmbargo);
    testBuysChange(beforeGame, actualGame);
    testActionsChange(beforeGame, actualGame);
}

int main()
{
    int seed = 68;
    int i, supplyToEmbargo; // supplyToEmbargo is choice1 for embargo
    int cardEffectReturnExpected, cardEffectReturnActual;
    struct gameState cleanGame, actualGame, beforeGame, expectedGame;

    // kingdom card set for the game (enums 7-15, inclusive and 22 for embargo)
    int k[10] = { adventurer, council_room, feast, gardens, mine, remodel, smithy, village, baron, embargo };

    printf("Unit Tests Card 4 - Embargo:\n");

    // initialize a clean game state for 2 players
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
    printf("Test 1: Use Embargo from player 0's hand position 0 to trash it, gain 2 coins (from 0 coins), and add 1 Embargo Token to the Province supply which already contains 0 Embargo Tokens\n");
    
    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    supplyToEmbargo = province; // supply to embargo for this test
    actualGame.embargoTokens[supplyToEmbargo] = 0;   // set up relavent embargo tokens
    actualGame.coins = 0; // set coins to starting value for this test

    // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = embargo;
    actualGame.hand[0][actualGame.handCount[0]++] = smithy;
    actualGame.hand[0][actualGame.handCount[0]++] = copper;
    actualGame.hand[0][actualGame.handCount[0]++] = gold;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = province;
    actualGame.deck[0][actualGame.deckCount[0]++] = gardens;
    actualGame.deck[0][actualGame.deckCount[0]++] = silver;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(embargo, supplyToEmbargo, 0, 0, &actualGame, 0, NULL); // choices 2, 3, and bonus is not used in embargo
    cardEffectReturnExpected = 0;

    // set expected state of embargo tokens and coins
    expectedGame.embargoTokens[supplyToEmbargo] = 1;
    expectedGame.coins = 2;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = smithy;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;
    expectedGame.hand[0][expectedGame.handCount[0]++] = gold;

    expectedGame.deckCount[0] = 0;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = province;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = gardens;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = silver;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;

    runEmbargoTests(&expectedGame, &actualGame, &beforeGame, supplyToEmbargo, cardEffectReturnExpected, cardEffectReturnActual);



    // test 2
    printf("Test 2: Use Embargo from player 0's hand position 3 to trash it, gain 2 coins (from 0 coins), and add 1 Embargo Token to the Gardens supply which already contains 3 Embargo Tokens\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    supplyToEmbargo = gardens; // supply to embargo for this test
    actualGame.embargoTokens[supplyToEmbargo] = 3;   // set up relavent embargo tokens
    actualGame.coins = 0; // set coins to starting value for this test

                          // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = smithy;
    actualGame.hand[0][actualGame.handCount[0]++] = copper;
    actualGame.hand[0][actualGame.handCount[0]++] = gold;
    actualGame.hand[0][actualGame.handCount[0]++] = embargo;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = province;
    actualGame.deck[0][actualGame.deckCount[0]++] = gardens;
    actualGame.deck[0][actualGame.deckCount[0]++] = silver;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(embargo, supplyToEmbargo, 0, 0, &actualGame, 3, NULL); // choices 2, 3, and bonus is not used in embargo
    cardEffectReturnExpected = 0;

    // set expected state of embargo tokens and coins
    expectedGame.embargoTokens[supplyToEmbargo] = 4;
    expectedGame.coins = 2;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = smithy;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;
    expectedGame.hand[0][expectedGame.handCount[0]++] = gold;

    expectedGame.deckCount[0] = 0;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = province;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = gardens;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = silver;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;

    runEmbargoTests(&expectedGame, &actualGame, &beforeGame, supplyToEmbargo, cardEffectReturnExpected, cardEffectReturnActual);



    // test 3
    printf("Test 3: Use Embargo from player 0's hand position 1 to trash it, gain 2 coins (from 5 coins), and add 1 Embargo Token to the Gold supply which already contains 0 Embargo Tokens\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    supplyToEmbargo = gold; // supply to embargo for this test
    actualGame.embargoTokens[supplyToEmbargo] = 0;   // set up relavent embargo tokens
    actualGame.coins = 5; // set coins to starting value for this test

                          // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = smithy;
    actualGame.hand[0][actualGame.handCount[0]++] = embargo;
    actualGame.hand[0][actualGame.handCount[0]++] = copper;
    actualGame.hand[0][actualGame.handCount[0]++] = gold;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = province;
    actualGame.deck[0][actualGame.deckCount[0]++] = gardens;
    actualGame.deck[0][actualGame.deckCount[0]++] = silver;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(embargo, supplyToEmbargo, 0, 0, &actualGame, 1, NULL); // choices 2, 3, and bonus is not used in embargo
    cardEffectReturnExpected = 0;

    // set expected state of embargo tokens and coins
    expectedGame.embargoTokens[supplyToEmbargo] = 1;
    expectedGame.coins = 7;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = smithy;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;
    expectedGame.hand[0][expectedGame.handCount[0]++] = gold;

    expectedGame.deckCount[0] = 0;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = province;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = gardens;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = silver;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;

    runEmbargoTests(&expectedGame, &actualGame, &beforeGame, supplyToEmbargo, cardEffectReturnExpected, cardEffectReturnActual);




    // test 4
    printf("Test 4: Use Embargo from player 0's hand position 2 to attempt to trash it, gain 2 coins (from 0 coins), and add 1 Embargo Token to the Sea Hag supply which is not in the game\n");

    memcpy(&actualGame, &cleanGame, sizeof(struct gameState)); // set actualGame to a clean game

    supplyToEmbargo = sea_hag; // supply to embargo for this test
    actualGame.embargoTokens[supplyToEmbargo] = 0;   // set up relavent embargo tokens
    actualGame.coins = 0; // set coins to starting value for this test

                          // set up player 0's hand
    actualGame.handCount[0] = 0;
    actualGame.hand[0][actualGame.handCount[0]++] = smithy;
    actualGame.hand[0][actualGame.handCount[0]++] = copper;
    actualGame.hand[0][actualGame.handCount[0]++] = embargo;
    actualGame.hand[0][actualGame.handCount[0]++] = gold;

    // set up player 0's deck
    actualGame.deckCount[0] = 0;
    actualGame.deck[0][actualGame.deckCount[0]++] = province;
    actualGame.deck[0][actualGame.deckCount[0]++] = gardens;
    actualGame.deck[0][actualGame.deckCount[0]++] = silver;

    // set up player 0's discard
    actualGame.discardCount[0] = 0;
    actualGame.discard[0][actualGame.discardCount[0]++] = village;
    actualGame.discard[0][actualGame.discardCount[0]++] = baron;
    actualGame.discard[0][actualGame.discardCount[0]++] = great_hall;

    memcpy(&beforeGame, &actualGame, sizeof(struct gameState)); // save state of the "before" state of actualGame

    cardEffectReturnActual = cardEffect(embargo, supplyToEmbargo, 0, 0, &actualGame, 2, NULL); // choices 2, 3, and bonus is not used in embargo
    cardEffectReturnExpected = -1;

    // set expected state of embargo tokens and coins
    expectedGame.embargoTokens[supplyToEmbargo] = 0;
    expectedGame.coins = 0;

    // set expected state of player 0
    expectedGame.handCount[0] = 0;
    expectedGame.hand[0][expectedGame.handCount[0]++] = smithy;
    expectedGame.hand[0][expectedGame.handCount[0]++] = copper;
    expectedGame.hand[0][expectedGame.handCount[0]++] = embargo;
    expectedGame.hand[0][expectedGame.handCount[0]++] = gold;

    expectedGame.deckCount[0] = 0;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = province;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = gardens;
    expectedGame.deck[0][expectedGame.deckCount[0]++] = silver;

    expectedGame.discardCount[0] = 0;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = village;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = baron;
    expectedGame.discard[0][expectedGame.discardCount[0]++] = great_hall;

    runEmbargoTests(&expectedGame, &actualGame, &beforeGame, supplyToEmbargo, cardEffectReturnExpected, cardEffectReturnActual);

    return 0;
}