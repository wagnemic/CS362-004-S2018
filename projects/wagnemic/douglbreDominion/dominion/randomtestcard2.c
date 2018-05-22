#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "rngs.h"
#include "interface.h"

/* Random tests for the Remodel card
 *
 * The Remodel card is supposed to allow the player to trash a chosen card from their hand and gain a card from any supply that costs up to 2 more than the trashed card
 * that gained card must go in the discard pile, the trashed card removed from the game, and the played Remodel placed into the discard pile
 * if you don't have any cards to trash, Remodel is played and discarded with nothing else happening (should return 0 because the card was still played successfully even though it really didn't do anything)
 * you aren't allowed to gain a card from a supply that is at 0 or not in the game (return should be -1), and -1 should be returned if card attempted to gain costs too much, and -1 should be returned if you try to tash the remodel because that's not allowed
 * the purpose of these tests is to check this functionality and get good coverage of the function using the random testing methodology
 * the random setup is as follows (but see randomizeGameForRemodelTests for implementation):
 * the game state is set to completley random bytes
 * the hand is given a random count from 1 thru 20 inclusive
 * an remodel is put in one of those spots
 * the rest of the hand is set to random cards
 * the discard gets 0 thru 20 random cards
 * the deck gets assigned almost MAX_DECK random cards because perhaps a bug puts cards in there and we need memory to do that, and the oracle checks if any cards changed in the array anyway
 * whose turn variable is set to player 0 (only running tests on 1 player, if we ran it on other player there woudln't be a difference for the remodel card)
 * numPlayers is set to a random value from 0-4, inclusive
 * the supplies of cards that are in every game are set to a random value from 0-4 inclusive, low values b/c interesting partions occur at 0 and >=1, so having high random just runs same code as lower ones and isn't useful for coverage
 * the supplies of 10 randomly chosen kingdom cars are set to a random value from 0-4, inclusive, but the remodel supply is always chsen in that set of 10 because it ahs to be for the card to be used in a valid game
 * unused kingdom cards get their supplies set to -1 to indicate they're not used in the game
 * playedCardCount is set to a random value from 0 through MAX_DECK-10, because a functionality i'm considering a bug will actually put cards in the played card array
 * and random cards are assigned to the played cards array because we'll be checking for side effects, might as well have valid cards in there
 * after game setup, the before state is saved and card effect is called on the game
 * the oracle used is one that checks the state of the game before remodel is played to the state after
 * depending on the state when card effect is called, 1 of 5 scenarios occur:
 * 1: the player had only one card in hand (which must be the remodel)
 * 2: player attempted to trash the remodel card
 * 3: player attempted to gain a card that costs too much (the cost of the card to gain is more than 2 plus the card to remodel)
 * 4: the player attempted to gain a card from a supply that's empty or not in the game
 * 5: if no other condition occurred, that means the remodel was successful
 * the comparison checks for changes between the before and after game states and reports any failures
 * see compareGameStatesRemodel for implementation of oracle, it goes over everything it checks
 *
 */

// only player 0 will be playing remodel
const int PLAYER = 0;

int compare(const void* a, const void* b);
int getCost(int cardNumber);
void cardNumToName(int card, char *name);

// fills the given array of 10 ints with the enum values for randomly chosen kingdom cards (enums are 7 thru 26 inclusive), and remodel must be one of them
// also the enums in the array will be sorted in ascending order
void chooseRandomKingdomCards(int k[10])
{
    int i, c;
    int n = 19;
    int all[19] = { adventurer, council_room, feast, gardens, mine, smithy, village, baron, great_hall, minion, steward, tribute, ambassador, cutpurse, embargo, outpost, salvager, sea_hag, treasure_map };

    // for all the enums we need to choose
    for (i = 0; i < 9; ++i)
    {
        // choose a random index from the first n in the all array
        c = rand() % n;

        // put the enum for that index in the array we'll be returning b/c that's a chosen kingdom card enum
        k[i] = all[c];

        // replace the chosen enum with the one at the end of the array
        all[c] = all[n - 1];

        // remove the final enum in the all array from a possible choice
        --n;
    }

    // must always choose remodel
    k[9] = remodel;

    // sort the array
    qsort((void*)(k), 10, sizeof(int), compare);
}

// given a pile of cards and the number of cards in the pile, this function returns how many of the given card are in that pile
int countCardInPile(int pile[], int pileCount, int cardToCount)
{
    int i;
    int count = 0;

    // loop over the pile
    for (i = 0; i < pileCount; ++i)
        // increment count when we encounter a cardToCount card
        if (pile[i] == cardToCount)
            count++;

    // return number of cardToCount cards in the pile to the caller
    return count;
}

// this function will modify the given game state to a random state in preparation to execute the remodel card effect on the state
// the hand and discard (deck isn't used in remodel) are given up to 20 random cards from all cards in the game
// the supplies are set up for 0-4, inclusive in each supply in the game
// supplies chosen line up with real game states (curse thru gold are in, and 10 randomly chosen kingdom cards are in, with remodel always being one of them)
// aside form values that must be set for a well-formed game that meets preconditions, all other values of the game state are set to something random (the remodel card isn't supposed to modify those)
// return value is the hand position of the remodel card in the player's hand
int randomizeGameForRemodelTests(struct gameState * G)
{
    int i, remodelPos, j;
    int k[10];

    // initialize the game to completley random values to start with
    for (i = 0; i < sizeof(struct gameState); i++)
        ((char*)G)[i] = (char)floor(rand() % 256);

    // player's hand count is set to a random value from 1 though 20 (there must be at least 1 because we need to ahve an remodel card in there)
    G->handCount[PLAYER] = 1 + (rand() % 20);

    // hand pos of the remodel card is 0 through handCount-1
    remodelPos = rand() % G->handCount[PLAYER];

    // place the remodel in the hand
    G->hand[PLAYER][remodelPos] = remodel;

    // player's hand contents are randomly generated from the 26 possible cards in the game 
    // but at least 1 needs to be a remodel because that's a precondition of the function
    for (i = 0; i < G->handCount[PLAYER]; ++i)
    {
        if (i == remodelPos) // do not overwrite the remodel card
            continue;
        G->hand[PLAYER][i] = rand() % (treasure_map + 1);
    }

    // player's discard count is set to a random value from 0 though 20
    G->discardCount[PLAYER] = rand() % 21;

    // player's discard contents are randomly generated from the 26 possible cards in the game 
    for (i = 0; i < G->handCount[PLAYER]; ++i)
        G->discard[PLAYER][i] = rand() % (treasure_map + 1);

    // note we don't need to set up the player's deck because remodel doesn't interact with it (but we can still check if it changed in side effects check)
    // we still need to put a bound that meets the memory requirments for the deck array though, but with some leeway in case some bug causes cards to be put in it, which the oracle will catch
    G->deckCount[PLAYER] = MAX_DECK-10;

    // might as well initialize to valid values for this array since we'll be checking if its content changed
    for (i = 0; i < G->deckCount[PLAYER]; ++i)
        G->deck[PLAYER][i] = rand() % (treasure_map + 1);

    // need to set the turn to the player we'll be using to play the card... this can't be random since its part of the precondition for playing the card
    G->whoseTurn = PLAYER;

    // allow a game to have 0-4 players, inclusive (lets us see if this matters... it shouldn't)
    G->numPlayers = rand() % 5;

    // remodel uses the supply, so we'll have our game use 10 randomly chosen kingdom supplies (and all the other supplies that are always used)
    // all supplies are given 0-4, inclusive, supply because interesting values are 0 and >=1, anything more doesn't have any different games state

    // set supplies for curse thru gold to 0-4 because these are always in the game
    for (i = curse; i <= gold; ++i)
        G->supplyCount[i] = rand() % 5;

    // set supplies for 10 randomly chosen kingdom cards to a random number from 0-4, other 10 kingdom cards get -1 to be considered not in the game
    chooseRandomKingdomCards(k);
    j = 0;
    for (i = adventurer; i <= treasure_map; ++i)
        if (k[j] == i) // if enum i is in the array of chosen enums (this works because k is sorted)
        {
            G->supplyCount[i] = rand() % 5;
            ++j;
        }
        else
            G->supplyCount[i] = -1;

    // this is here because of a bug (or at least what i assume to be a bug based on my interpretation of the dominion code)
    // the bug in discardCard uses this value and it can't be garbage otherwise it may crash the program
    // little space available at end of the array to let the game put cards in there... (it will b/c of bug if discardCard is called)
    G->playedCardCount = rand() % (MAX_DECK - 9);

    // might as well initialize to valid values for this array since we'll be checking if its content changed
    for (i = 0; i < G->playedCardCount; ++i)
        G->playedCards[i] = rand() % (treasure_map + 1);

    // caller of this funciton needs the position of the remodel to play
    return remodelPos;
}

// returns 1 if any side effect occurred between the game states when the expected effect of the card happened
// returns 0 if no side effects occurred between the game states when the expected effect of the card happened
// also prints specific failure messages when 1 is returned, nothing printed if 0 is returned
int didSideEffectsOccur(struct gameState *gameBefore, struct gameState *gameAfter)
{
    int i, j, cardCountBefore, cardCountAfter;
    int anyFailure = 0;
    char buffer[100];

    if (gameBefore->numPlayers != gameAfter->numPlayers)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to numPlayers (expected = %d, actual = %d)\n", gameBefore->numPlayers, gameAfter->numPlayers);
    }

    // notice no check for supply here, some suppleis are supposed to change and checks for supplies are in the oracle (compareGameStatesRemodel)!

    for (i = 0; i < treasure_map + 1; ++i)
        if (gameBefore->embargoTokens[i] != gameAfter->embargoTokens[i])
        {
            anyFailure = 1;
            cardNumToName(i, buffer);
            printf("FAIL when checking if no change to %s embargo tokens (expected = %d, actual = %d)\n", buffer, gameBefore->embargoTokens[i], gameAfter->embargoTokens[i]);
        }

    if (gameBefore->outpostPlayed != gameAfter->outpostPlayed)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to outpostPlayed (expected = %d, actual = %d)\n", gameBefore->outpostPlayed, gameAfter->outpostPlayed);
    }

    if (gameBefore->outpostTurn != gameAfter->outpostTurn)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to outpostTurn (expected = %d, actual = %d)\n", gameBefore->outpostTurn, gameAfter->outpostTurn);
    }

    if (gameBefore->whoseTurn != gameAfter->whoseTurn)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to whoseTurn (expected = %d, actual = %d)\n", gameBefore->whoseTurn, gameAfter->whoseTurn);
    }

    if (gameBefore->phase != gameAfter->phase)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to phase (expected = %d, actual = %d)\n", gameBefore->phase, gameAfter->phase);
    }

    if (gameBefore->numActions != gameAfter->numActions)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to numActions (expected = %d, actual = %d)\n", gameBefore->numActions, gameAfter->numActions);
    }

    if (gameBefore->coins != gameAfter->coins)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to coins (expected = %d, actual = %d)\n", gameBefore->coins, gameAfter->coins);
    }

    if (gameBefore->numBuys != gameAfter->numBuys)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to numBuys (expected = %d, actual = %d)\n", gameBefore->numBuys, gameAfter->numBuys);
    }

    // player 0 should not be part of side effect checks (its supposed to have game state for it change)
    for (j = 1; j < MAX_PLAYERS; ++j)
        if (gameBefore->handCount[j] != gameAfter->handCount[j])
        {
            anyFailure = 1;
            printf("FAIL when checking if no change to player %d's hand count (expected = %d, actual = %d)\n", j, gameBefore->handCount[j], gameAfter->handCount[j]);
        }

    for (j = 1; j < MAX_PLAYERS; ++j)
        for (i = 0; i < MAX_DECK; ++i)
            if (gameBefore->hand[j][i] != gameAfter->hand[j][i])
            {
                anyFailure = 1;
                printf("FAIL when checking if no change to player %d's hand contents\n", j);
                break; // just look for a single mismatch per player
            }

    // note that remodel shouldn't affect anyone's deck
    for (j = 0; j < MAX_PLAYERS; ++j)
        if (gameBefore->deckCount[j] != gameAfter->deckCount[j])
        {
            anyFailure = 1;
            printf("FAIL when checking if no change to player %d's deck count (expected = %d, actual = %d)\n", j, gameBefore->deckCount[j], gameAfter->deckCount[j]);
        }

    for (j = 0; j < MAX_PLAYERS; ++j)
        for (i = 0; i < MAX_DECK; ++i)
            if (gameBefore->deck[j][i] != gameAfter->deck[j][i])
            {
                anyFailure = 1;
                printf("FAIL when checking if no change to player %d's deck contents\n", j);
                break; // just look for a single mismatch per player
            }

    // player 0 should not be part of side effect checks (its supposed to have game state for it change)
    for (j = 1; j < MAX_PLAYERS; ++j)
        if (gameBefore->discardCount[j] != gameAfter->discardCount[j])
        {
            anyFailure = 1;
            printf("FAIL when checking if no change to player %d's discard count (expected = %d, actual = %d)\n", j, gameBefore->discardCount[j], gameAfter->discardCount[j]);
        }

    for (j = 1; j < MAX_PLAYERS; ++j)
        for (i = 0; i < MAX_DECK; ++i)
            if (gameBefore->discard[j][i] != gameAfter->discard[j][i])
            {
                anyFailure = 1;
                printf("FAIL when checking if no change to player %d's discard contents\n", j);
                break; // just look for a single mismatch per player
            }

    if (gameBefore->playedCardCount != gameAfter->playedCardCount)
    {
        anyFailure = 1;
        printf("FAIL when checking if no change to playedCardCount (expected = %d, actual = %d)\n", gameBefore->playedCardCount, gameAfter->playedCardCount);
    }

    // the played pile shuold not ever be affected (according to my interpretation of the inteded functionality of the dominoin code)
    // so we'll go through all possible cards and count how many are in played pile before and compare to after
    for (i = curse; i <= treasure_map; ++i)
    {
        cardCountBefore = countCardInPile(gameBefore->playedCards, gameBefore->playedCardCount, i);
        cardCountAfter = countCardInPile(gameAfter->playedCards, gameAfter->playedCardCount, i);
        if (cardCountAfter != cardCountBefore)
        {
            anyFailure = 1;
            cardNumToName(i, buffer);
            printf("FAIL when checking count of %s in played pile (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, cardCountBefore, cardCountAfter, cardCountBefore);
        }
    }

    return anyFailure;
}

// carries out the test to check if the expected games state for player 0 is the same as in the actual game, given if the remodel should have failed, and if not, the enum of the trased card and gained card
// if remodelFailed is given as 1, then the values for trashedCard and gainedCard are not used
// use 0 for remodelFailed, and -1 for gainedCard and trashedCard to indicate that no card was gained or trashed when player played remodel w/ just the remodel in hand (i.e. the remodel is just discarded and that's all that happens)
void compareGameStatesRemodel(struct gameState *gameBefore, struct gameState *gameAfter, int remodelFailed, int trashedCard, int gainedCard)
{
    int i, found, handCountBefore, discardCountBefore, handCountAfter, discardCountAfter, cardCountBefore, cardCountAfter, diff;
    char buffer[100];

    // get hand and discard counts before any effects
    handCountBefore = gameBefore->handCount[PLAYER];
    discardCountBefore = gameBefore->discardCount[PLAYER];

    // get hand and discard counts after remodel effect
    handCountAfter = gameAfter->handCount[PLAYER];
    discardCountAfter = gameAfter->discardCount[PLAYER];


    // if the remodel effect failed, then we need to check to see if nothing changed for the player and supplies
    // note that trashedCard and gainedCard are not used in this branch, so the caller can send w/e values they want
    if (remodelFailed == 1)
    {
        // the hand count should have remained the same (same w/ deck, but deck is never modified anyway and is captured in the side effects test)
        if (handCountAfter == handCountBefore)
            printf("PASS when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore, handCountAfter, handCountBefore);
        else
            printf("FAIL when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore, handCountAfter, handCountBefore);

        // hand contents should be the same
        found = 0;
        for (i = curse; i <= treasure_map; ++i)
        {
            cardCountBefore = countCardInPile(gameBefore->hand[PLAYER], gameBefore->handCount[PLAYER], i);
            cardCountAfter = countCardInPile(gameAfter->hand[PLAYER], gameAfter->handCount[PLAYER], i);
            if (cardCountAfter != cardCountBefore)
            {
                found = 1;
                cardNumToName(i, buffer);
                printf("FAIL when checking count of %s in hand (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, cardCountBefore, cardCountAfter, cardCountBefore);
            }
        }
        if (found == 0)
            printf("PASS when checking hand contents\n");

        // the discard count should have remained the same 
        if (discardCountAfter == discardCountBefore)
            printf("PASS when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore, discardCountAfter, discardCountBefore);
        else
            printf("FAIL when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore, discardCountAfter, discardCountBefore);

        // discard contents should be the same
        found = 0;
        for (i = curse; i <= treasure_map; ++i)
        {
            cardCountBefore = countCardInPile(gameBefore->discard[PLAYER], gameBefore->discardCount[PLAYER], i);
            cardCountAfter = countCardInPile(gameAfter->discard[PLAYER], gameAfter->discardCount[PLAYER], i);
            if (cardCountAfter != cardCountBefore)
            {
                found = 1;
                cardNumToName(i, buffer);
                printf("FAIL when checking count of %s in discard (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, cardCountBefore, cardCountAfter, cardCountBefore);
            }
        }
        if (found == 0)
            printf("PASS when checking discard contents\n");

        // all supplies should be the same
        found = 0;
        for (i = curse; i <= treasure_map; ++i)
            if (gameAfter->supplyCount[i] != gameBefore->supplyCount[i])
            {
                found = 1;
                cardNumToName(i, buffer);
                printf("FAIL when checking %s supply count (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, gameBefore->supplyCount[i], gameAfter->supplyCount[i], gameBefore->supplyCount[i]);
            }
        if (found == 0)
            printf("PASS when checking all supply counts\n");
    }

    // remodel was successful
    else
    {
        // w/ these inputs to the function, that means the remodel was discarded and nothing was gained or trashed b/c the player only had the remodel in hand
        if (trashedCard == -1 && gainedCard == -1)
        {
            // the hand count should have 1 less than the before state (the discarded remodel)
            if (handCountAfter == handCountBefore - 1)
                printf("PASS when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore - 1, handCountAfter, handCountBefore);
            else
                printf("FAIL when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore - 1, handCountAfter, handCountBefore);

            // hand contents should be the same for all card counts except there should be 1 less remodel
            found = 0;
            for (i = curse; i <= treasure_map; ++i)
            {
                cardCountBefore = countCardInPile(gameBefore->hand[PLAYER], gameBefore->handCount[PLAYER], i);
                cardCountAfter = countCardInPile(gameAfter->hand[PLAYER], gameAfter->handCount[PLAYER], i);
                if (i == remodel)
                    diff = 1;
                else
                    diff = 0;
                if (cardCountAfter != cardCountBefore - diff)
                {
                    found = 1;
                    cardNumToName(i, buffer);
                    printf("FAIL when checking count of %s in hand (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, cardCountBefore - diff, cardCountAfter, cardCountBefore);
                }
            }
            if (found == 0)
                printf("PASS when checking hand contents\n");

            // the discard count should be one more than the count before
            if (discardCountAfter == discardCountBefore + 1)
                printf("PASS when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore + 1, discardCountAfter, discardCountBefore);
            else
                printf("FAIL when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore + 1, discardCountAfter, discardCountBefore);

            // discard contents should be the same, except count of remodel should be 1 more than count before
            found = 0;
            for (i = curse; i <= treasure_map; ++i)
            {
                cardCountBefore = countCardInPile(gameBefore->discard[PLAYER], gameBefore->discardCount[PLAYER], i);
                cardCountAfter = countCardInPile(gameAfter->discard[PLAYER], gameAfter->discardCount[PLAYER], i);
                if (i == remodel)
                    diff = 1;
                else
                    diff = 0;
                if (cardCountAfter != cardCountBefore + diff)
                {
                    found = 1;
                    cardNumToName(i, buffer);
                    printf("FAIL when checking count of %s in discard (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, cardCountBefore + diff, cardCountAfter, cardCountBefore);
                }
            }
            if (found == 0)
                printf("PASS when checking discard contents\n");

            // all supplies should be the same
            found = 0;
            for (i = curse; i <= treasure_map; ++i)
                if (gameAfter->supplyCount[i] != gameBefore->supplyCount[i])
                {
                    found = 1;
                    cardNumToName(i, buffer);
                    printf("FAIL when checking %s supply count (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, gameBefore->supplyCount[i], gameAfter->supplyCount[i], gameBefore->supplyCount[i]);
                }
            if (found == 0)
                printf("PASS when checking all supply counts\n");
        }

        // remodel was successful in trashing a trashedCard from hand,  gaining a gainedCard from its supply, and discarding the remodel
        else
        {
            // the hand count should have 2 less than the before state (the discarded remodel and trashed card)
            if (handCountAfter == handCountBefore - 2)
                printf("PASS when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore - 2, handCountAfter, handCountBefore);
            else
                printf("FAIL when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore - 2, handCountAfter, handCountBefore);

            // hand contents should be the same for all card counts except there should be 1 less remodel and 1 less of trashedCard
            found = 0;
            for (i = curse; i <= treasure_map; ++i)
            {
                cardCountBefore = countCardInPile(gameBefore->hand[PLAYER], gameBefore->handCount[PLAYER], i);
                cardCountAfter = countCardInPile(gameAfter->hand[PLAYER], gameAfter->handCount[PLAYER], i);
                if (i == remodel && remodel == trashedCard) // if a remodel was the card trashed, there should be 2 less in hand (the one discarded after being played AND the one trashed)
                    diff = 2;
                else if (i == remodel || i == trashedCard)
                    diff = 1;
                else
                    diff = 0;
                if (cardCountAfter != cardCountBefore - diff)
                {
                    found = 1;
                    cardNumToName(i, buffer);
                    printf("FAIL when checking count of %s in hand (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, cardCountBefore - diff, cardCountAfter, cardCountBefore);
                }
            }
            if (found == 0)
                printf("PASS when checking hand contents\n");

            // the discard count should be 2 more than the count before (the remodel and the gained card)
            if (discardCountAfter == discardCountBefore + 2)
                printf("PASS when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore + 2, discardCountAfter, discardCountBefore);
            else
                printf("FAIL when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore + 2, discardCountAfter, discardCountBefore);

            // discard contents should be the same, except count of remodel and the gainedCard should be 1 more than count before
            found = 0;
            for (i = curse; i <= treasure_map; ++i)
            {
                cardCountBefore = countCardInPile(gameBefore->discard[PLAYER], gameBefore->discardCount[PLAYER], i);
                cardCountAfter = countCardInPile(gameAfter->discard[PLAYER], gameAfter->discardCount[PLAYER], i);
                if (i == remodel && remodel == gainedCard) // if a remodel was the card gained, there should be 2 more in discard (the one discarded after being played AND the one gained)
                    diff = 2;
                else if (i == remodel || i == gainedCard)
                    diff = 1;
                else
                    diff = 0;
                if (cardCountAfter != cardCountBefore + diff)
                {
                    found = 1;
                    cardNumToName(i, buffer);
                    printf("FAIL when checking count of %s in discard (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, cardCountBefore + diff, cardCountAfter, cardCountBefore);
                }
            }
            if (found == 0)
                printf("PASS when checking discard contents\n");

            // all supplies should be the same, except gained card supply should be one less
            found = 0;
            for (i = curse; i <= treasure_map; ++i)
            {
                if (i == gainedCard)
                    diff = 1;
                else
                    diff = 0;
                if (gameAfter->supplyCount[i] != gameBefore->supplyCount[i] - diff)
                {
                    found = 1;
                    cardNumToName(i, buffer);
                    printf("FAIL when checking %s supply count (expected = %d, actual = %d, before cardEffect = %d)\n", buffer, gameBefore->supplyCount[i] - diff, gameAfter->supplyCount[i], gameBefore->supplyCount[i]);
                }
            }
            if (found == 0)
                printf("PASS when checking all supply counts\n");
        }
    }
}

// executes the full set of code to run one test for player 0 to use one remodel on a randomly set up game, includes pass/fail printouts
void runOneRandomTestForRemodelEffect()
{
    int cardEffectReturnActual, cardEffectReturnExpected, remodelHandPos, handPosToRemodel, supplyPosToGain;
    char cardToGainName[100];
    char cardToTrashName[100];
    struct gameState gameBefore, gameAfter;

    // randomize the game state and get position of remodel from the state
    remodelHandPos = randomizeGameForRemodelTests(&gameBefore);

    // make both before and after the same
    memcpy(&gameAfter, &gameBefore, sizeof(struct gameState));

    // choose a random card in hand to remodel (could even be the remodel being played, and the card effect should handle that)
    handPosToRemodel = rand() % gameBefore.handCount[PLAYER];

    // choose a random supply to gain (choose any supply, even ones not in game because the card effect should handle that)
    supplyPosToGain = rand() % (treasure_map + 1);

    // call the function under test on the after game
    cardEffectReturnActual = cardEffect(remodel, handPosToRemodel, supplyPosToGain, 0, &gameAfter, remodelHandPos, NULL); // choice1 is hand# of card to remodel, choice2 is supply#, choice 3 and bonus are not used in remodel

    // depending on the state of the game before playing remodel, there's a few scenarios that could occur (see comments in the upcoming if-else tree)


    // scenario 1: the player had only one card in hand (which must be the remodel)
    if (gameBefore.handCount[PLAYER] == 1)
    {
        // the only thing that should happen is the remodel gets discarded
        cardNumToName(supplyPosToGain, cardToGainName);
        printf("Results from a scenario 1 game (no other cards in hand to trash in an attempt to gain %d-cost %s):\n", getCost(supplyPosToGain), cardToGainName);
        compareGameStatesRemodel(&gameBefore, &gameAfter, 0, -1, -1); // the -1s indicate remodel discarded and nothing was trashed or gained
        cardEffectReturnExpected = 0;
    }

    // scenario 2: player attempted to trash the remodel card
    else if (handPosToRemodel == remodelHandPos)
    {
        // nothing should have changed for the game state, and a -1 should have been returned
        cardNumToName(supplyPosToGain, cardToGainName);
        printf("Results from a scenario 2 game (attempt to trash the played 4-cost remodel in an attempt to gain %d-cost %s):\n", getCost(supplyPosToGain), cardToGainName);
        compareGameStatesRemodel(&gameBefore, &gameAfter, 1, -1, -1); // last 2 args are not used when the 3rd to last is 1
        cardEffectReturnExpected = -1;
    }

    // scenario 3: player attempted to gain a card that costs too much (the cost of the card to gain is more than 2 plus the card to remodel)
    else if (getCost(supplyPosToGain) > 2 + getCost(gameBefore.hand[PLAYER][handPosToRemodel]))
    {
        // nothing should have changed for the game state, and a -1 should have been returned
        cardNumToName(gameBefore.hand[PLAYER][handPosToRemodel], cardToTrashName);
        cardNumToName(supplyPosToGain, cardToGainName);
        printf("Results from a scenario 3 game (attempt to trash %d-cost %s in an attempt to gain %d-cost %s but it costs too much):\n", getCost(gameBefore.hand[PLAYER][handPosToRemodel]), cardToTrashName, getCost(supplyPosToGain), cardToGainName);
        compareGameStatesRemodel(&gameBefore, &gameAfter, 1, -1, -1); // last 2 args are not used when the 3rd to last is 1
        cardEffectReturnExpected = -1;
    }

    // scenario 4: the player attempted to gain a card from a supply that's empty or not in the game
    else if (gameBefore.supplyCount[supplyPosToGain] <= 0)
    {
        // nothing should have changed for the game state, and a -1 should have been returned
        cardNumToName(gameBefore.hand[PLAYER][handPosToRemodel], cardToTrashName);
        cardNumToName(supplyPosToGain, cardToGainName);
        printf("Results from a scenario 4 game (attempt trash %d-cost %s in an attempt to gain %d-cost %s from an empty or unused supply):\n", getCost(gameBefore.hand[PLAYER][handPosToRemodel]), cardToTrashName, getCost(supplyPosToGain), cardToGainName);
        compareGameStatesRemodel(&gameBefore, &gameAfter, 1, -1, -1); // last 2 args are not used when the 3rd to last is 1
        cardEffectReturnExpected = -1;
    }

    // scenario 5: if no other condition occurred, that means the remodel was successful
    else
    {
        // the gained card should be in discard, the played remodel should be in discard, the trashed card should be removed from hand, the supply of gained card should be reduced by 1
        cardNumToName(gameBefore.hand[PLAYER][handPosToRemodel], cardToTrashName);
        cardNumToName(supplyPosToGain, cardToGainName);
        printf("Results from a scenario 5 game (trashed %d-cost %s and gained %d-cost %s):\n", getCost(gameBefore.hand[PLAYER][handPosToRemodel]), cardToTrashName, getCost(supplyPosToGain), cardToGainName);
        compareGameStatesRemodel(&gameBefore, &gameAfter, 0, gameBefore.hand[PLAYER][handPosToRemodel], supplyPosToGain);
        cardEffectReturnExpected = 0;
    }

    // compare return value to expected 
    if (cardEffectReturnActual == cardEffectReturnExpected)
        printf("PASS when checking cardEffect return value (expected = %d, actual = %d)\n", cardEffectReturnExpected, cardEffectReturnActual);
    else
        printf("FAIL when checking cardEffect return value (expected = %d, actual = %d)\n", cardEffectReturnExpected, cardEffectReturnActual);

    // check if any side effects occurred
    if (didSideEffectsOccur(&gameBefore, &gameAfter) == 0)
        printf("PASS when checking if no side effects occurred\n");
    // else condition stuff gets printed within didSideEffectsOccur  
}

int main()
{
    printf("Random Tests Card 2 - Remodel:\n");

    int i;

    // initialize random generator
    SelectStream(2);
    PutSeed(3);

    // run a bunch of random tests on remodel effect (includes printouts for pass/fail)
    for (i = 0; i < 1000; ++i)
        runOneRandomTestForRemodelEffect();

    return 0;
}
