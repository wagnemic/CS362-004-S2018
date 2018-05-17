#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "rngs.h"
#include "interface.h"

/* Random tests for the Adventurer card
 *
 * The Adventurer card is supposed to put two treasures from the player's deck into their hand and discard all non treasures drawn
 * if the deck becomes empty, the function continues by shuffling the discard pile and creating a new deck (but not including the previously drawn non treausure cards)
 * if it gets through the entire deck and discard without finding 2 treasures, the player just gets as many treasures as they could find
 * then adventurer gets discarded and the non treasure cards also finally get placed into the discard pile
 * the purpose of these tests is to check this functionality and get good coverage of the function using the random testing methodology
 * the random setup is as follows (but see randomizeGameForAdventurerTests for implementation):
 * the game state is set to completley random bytes
 * the hand is given a random count from 1 thru 20 inclusive
 * an adventurer is put in one of those spots
 * the rest of the hand is set to random cards
 * the deck gets 0 thru 20 random cards
 * the discard gets 0 thru 20 random cards
 * whose turn variable is set to player 0 (only running tests on 1 player, if we ran it on other player there woudln't be a difference for the adventurer card)
 * numPlayers is set to a random value from 0-4, inclusive
 * numBuys is set to 6 (gold) because there can be an infinite loop caused by a bug that we want to not let happen (will still get failures in those cases to indicate a bug in those cases)
 * playedCardCount is set to a random value from 0 through MAX_DECK-10, because a functionality i'm considering a bug will actually put cards in the played card array
 * and random cards are assigned to the played cards array because we'll be checking for side effects, might as well have valid cards in there
 * after game setup, the before state is saved and card effect is called on the game
 * the oracle used is one that checks the state of the game before adventurer is played to the state after
 * depending on the state when card effect is called, 1 of 3 scenarios occur:
 * 1: 1 treasure card moved into hand from desk/discard
 * 2: 2 treasure cards moved into hand from desk/discard
 * 3: 0 treasure cards moved into hand from desk/discard
 * because this action card involves drawing, we can't always predict the specific cards drawn after a shuffle
 * so the best we can do to compare is check the card counts of adventurer, treasure cards, and any card for the player's hand and discard/deck in the before and after states
 * depending on which scenario happens we can calculate the expected counts of all the above items and compare to what we got
 * that is what the oracle is (see compareGameStatesAdventurer for implementation of oracle)
 *
 */

 /* MODIFIED FOR TARANTULA
 *
 * the caller of this program provides an integer argument as the first arg
 * this refers to the unit test to be run using this program
 * this program can run 1000 unit tests (so give it an integer 1-1000, inclusive)
 * if the given unit test passes, this program returns 0
 * if the given unit test passes, this program returns 1
 * if you gave it an out of bounds integer this program returns 2
 * no checks for not giving an integer or not giving a first arg, so program may crash in those cases
 * 
 */
 
// only player 0 will be playing adventurer
const int PLAYER = 0;

void cardNumToName(int card, char *name);

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

// this function will modify the given game state to a random state in preparation to execute the adventurer card effect on the state
// the deck, hand, and discard are given up to 20 random cards from all cards in the game
// aside form values that must be set for a well-formed game that meets preconditions, all other values of the game state are set to something random (the adventurer card isn't supposed to modify those)
// return value is the hand position of the adventurer card in the player's hand
int randomizeGameForAdventurerTests(struct gameState * G)
{
    int i, adventurerPos;

    // initialize the game to completley random values to start with
    for (i = 0; i < sizeof(struct gameState); i++)
        ((char*)G)[i] = (char)floor(rand() % 256);

    // player's hand count is set to a random value from 1 though 20 (there must be at least 1 because we need to ahve an adventurer card in there)
    G->handCount[PLAYER] = 1 + (rand() % 20);

    // hand pos of the adventurer card is 0 through handCount-1
    adventurerPos = rand() % G->handCount[PLAYER];

    // place the adventurer in the hand
    G->hand[PLAYER][adventurerPos] = adventurer;

    // player's hand contents are randomly generated from the 26 possible cards in the game 
    // but at least 1 needs to be an adventurer because that's a precondition of the function
    for (i = 0; i < G->handCount[PLAYER]; ++i)
    {
        if (i == adventurerPos) // do not overwrite the adventurer card
            continue;
        G->hand[PLAYER][i] = rand() % (treasure_map + 1);
    }

    // player's discard count is set to a random value from 0 though 20
    G->discardCount[PLAYER] = rand() % 21;

    // player's discard contents are randomly generated from the 26 possible cards in the game 
    for (i = 0; i < G->handCount[PLAYER]; ++i)
        G->discard[PLAYER][i] = rand() % (treasure_map + 1);

    // player's deck count is set to a random value from 0 though 20
    G->deckCount[PLAYER] = rand() % 21;

    // player's discard contents are randomly generated from the 26 possible cards in the game 
    for (i = 0; i < G->handCount[PLAYER]; ++i)
        G->deck[PLAYER][i] = rand() % (treasure_map + 1);

    // need to set the turn to the player we'll be using to play the card... this can't be random since its part of the precondition for playing the card
    G->whoseTurn = PLAYER;

    // allow a game to have 0-4 players, inclusive (lets us see if this matters... it shouldn't)
    G->numPlayers = rand() % 5;

    // to avoid looping infinitely (due to a bug) when the random game generation does not give any treasure to deck/discard, we need to set numBuys to a treasure value (if you want to know why, i explain why in my assignment 3 pdf submission)
    G->numBuys = gold;

    // this is here because of a bug (or at least what i assume to be a bug based on my interpretation of the dominion code)
    // the bug in discardCard uses this value and it can't be garbage otherwise it may crash the program
    // little space available at end of the array to let the game put cards in there... (it will b/c of bug if discardCard is called)
    G->playedCardCount = rand() % (MAX_DECK - 9);

    // might as well initialize to valid values for this array since we'll be checking if its content changed
    for (i = 0; i < G->playedCardCount; ++i)
        G->playedCards[i] = rand() % (treasure_map + 1);

    // caller of this funciton needs the position of the adventurer to play
    return adventurerPos;
}

// returns 1 if any side effect occurred between the game states when the expected effect of the card happened
// returns 0 if no side effects occurred between the game states when the expected effect of the card happened
int didSideEffectsOccur(struct gameState *gameBefore, struct gameState *gameAfter)
{
    int i, j, cardCountBefore, cardCountAfter;
    int anyFailure = 0;
    char buffer[100];

    if (gameBefore->numPlayers != gameAfter->numPlayers)
    {
        anyFailure = 1; return 1;
    }

    for (i = 0; i < treasure_map + 1; ++i)
        if (gameBefore->supplyCount[i] != gameAfter->supplyCount[i])
        {
            anyFailure = 1; return 1;
            cardNumToName(i, buffer);
        }

    for (i = 0; i < treasure_map + 1; ++i)
        if (gameBefore->embargoTokens[i] != gameAfter->embargoTokens[i])
        {
            anyFailure = 1; return 1;
            cardNumToName(i, buffer);
        }

    if (gameBefore->outpostPlayed != gameAfter->outpostPlayed)
    {
        anyFailure = 1; return 1;
    }

    if (gameBefore->outpostTurn != gameAfter->outpostTurn)
    {
        anyFailure = 1; return 1;
    }

    if (gameBefore->whoseTurn != gameAfter->whoseTurn)
    {
        anyFailure = 1; return 1;
}

    if (gameBefore->phase != gameAfter->phase)
    {
        anyFailure = 1; return 1;
    }

    if (gameBefore->numActions != gameAfter->numActions)
    {
        anyFailure = 1; return 1;
    }

    if (gameBefore->coins != gameAfter->coins)
    {
        anyFailure = 1; return 1;
    }

    if (gameBefore->numBuys != gameAfter->numBuys)
    {
        anyFailure = 1; return 1;
    }

    // player 0 should not be part of side effect checks (its supposed to have game state for it change)
    for (j = 1; j < MAX_PLAYERS; ++j)
        if (gameBefore->handCount[j] != gameAfter->handCount[j])
        {
            anyFailure = 1; return 1;
        }

    for (j = 1; j < MAX_PLAYERS; ++j)
        for (i = 0; i < MAX_DECK; ++i)
            if (gameBefore->hand[j][i] != gameAfter->hand[j][i])
            {
                anyFailure = 1; return 1;
                break; // just look for a single mismatch per player
            }

    // player 0 should not be part of side effect checks (its supposed to have game state for it change)
    for (j = 1; j < MAX_PLAYERS; ++j)
        if (gameBefore->deckCount[j] != gameAfter->deckCount[j])
        {
            anyFailure = 1; return 1;
        }

    for (j = 1; j < MAX_PLAYERS; ++j)
        for (i = 0; i < MAX_DECK; ++i)
            if (gameBefore->deck[j][i] != gameAfter->deck[j][i])
            {
                anyFailure = 1; return 1;
                break; // just look for a single mismatch per player
            }

    // player 0 should not be part of side effect checks (its supposed to have game state for it change)
    for (j = 1; j < MAX_PLAYERS; ++j)
        if (gameBefore->discardCount[j] != gameAfter->discardCount[j])
        {
            anyFailure = 1; return 1;
        }

    for (j = 1; j < MAX_PLAYERS; ++j)
        for (i = 0; i < MAX_DECK; ++i)
            if (gameBefore->discard[j][i] != gameAfter->discard[j][i])
            {
                anyFailure = 1; return 1;
                break; // just look for a single mismatch per player
            }

    if (gameBefore->playedCardCount != gameAfter->playedCardCount)
    {
        anyFailure = 1; return 1;
    }

    // the played pile shuold not ever be affected (according to my interpretation of the inteded functionality of the dominoin code)
    // so we'll go through all possible cards and count how many are in played pile before and compare to after
    for (i = curse; i <= treasure_map; ++i)
    {
        cardCountBefore = countCardInPile(gameBefore->playedCards, gameBefore->playedCardCount, i);
        cardCountAfter = countCardInPile(gameAfter->playedCards, gameAfter->playedCardCount, i);
        if (cardCountAfter != cardCountBefore)
        {
            anyFailure = 1; return 1;
            cardNumToName(i, buffer);
        }
    }

    return anyFailure;
}

// carries out the test to check if the expected games state for player 0 is the same as in the actual game, given the number of treasure that should have been added to the player's hand from deck/discard combined
int compareGameStatesAdventurer(struct gameState *gameBefore, struct gameState *gameAfter, int addedTreasure)
{
    int numAdventurerHandBefore, numAdventurerDiscardDeckBefore, numTreasureHandBefore, numTreasureDiscardDeckBefore,
        numAdventurerHandAfter, numAdventurerDiscardDeckAfter, numTreasureHandAfter, numTreasureDiscardDeckAfter,
        handCountBefore, discardDeckCountBefore,
        handCountAfter, discardDeckCountAfter;

    // because there's a random shuffle involved, we can't expect an exact game state
    // the best we can do is make sure the required number of treasure cards were added to the player's hand
    // and that the same amount of treasure were removed from the player's deck and discard collectivley
    // also the adventurer card should have been put in the discard (but we must use total count between discard and deck bcause a shffle may add an adventurer from deck to discard)
    // and we can check if the total hand count is what it should be and total discard/deck is what it should be
    // we can't check individual cards because of the random shuffle mechanic, unfortunatley (... well, its technically possible if we severely constrain the input, but then this just approahces unit testing anyway)

    // count how many adventurers are in the player's hand and discard/deck before any effects
    numAdventurerHandBefore = countCardInPile(gameBefore->hand[PLAYER], gameBefore->handCount[PLAYER], adventurer);
    numAdventurerDiscardDeckBefore = countCardInPile(gameBefore->discard[PLAYER], gameBefore->discardCount[PLAYER], adventurer) + countCardInPile(gameBefore->deck[PLAYER], gameBefore->deckCount[PLAYER], adventurer);

    // count how many adventurers are in the player's hand and discard/deck in the after game
    numAdventurerHandAfter = countCardInPile(gameAfter->hand[PLAYER], gameAfter->handCount[PLAYER], adventurer);
    numAdventurerDiscardDeckAfter = countCardInPile(gameAfter->discard[PLAYER], gameAfter->discardCount[PLAYER], adventurer) + countCardInPile(gameAfter->deck[PLAYER], gameAfter->deckCount[PLAYER], adventurer);

    // count how many treasure are in the player's hand and discard/deck before any effects
    numTreasureHandBefore = countCardInPile(gameBefore->hand[PLAYER], gameBefore->handCount[PLAYER], copper) + countCardInPile(gameBefore->hand[PLAYER], gameBefore->handCount[PLAYER], silver) + countCardInPile(gameBefore->hand[PLAYER], gameBefore->handCount[PLAYER], gold);
    numTreasureDiscardDeckBefore = countCardInPile(gameBefore->discard[PLAYER], gameBefore->discardCount[PLAYER], copper) + countCardInPile(gameBefore->discard[PLAYER], gameBefore->discardCount[PLAYER], silver) + countCardInPile(gameBefore->discard[PLAYER], gameBefore->discardCount[PLAYER], gold) + countCardInPile(gameBefore->deck[PLAYER], gameBefore->deckCount[PLAYER], copper) + countCardInPile(gameBefore->deck[PLAYER], gameBefore->deckCount[PLAYER], silver) + countCardInPile(gameBefore->deck[PLAYER], gameBefore->deckCount[PLAYER], gold);

    // count how many treasure are in the player's hand and discard/deck in the after game
    numTreasureHandAfter = countCardInPile(gameAfter->hand[PLAYER], gameAfter->handCount[PLAYER], copper) + countCardInPile(gameAfter->hand[PLAYER], gameAfter->handCount[PLAYER], silver) + countCardInPile(gameAfter->hand[PLAYER], gameAfter->handCount[PLAYER], gold);
    numTreasureDiscardDeckAfter = countCardInPile(gameAfter->discard[PLAYER], gameAfter->discardCount[PLAYER], copper) + countCardInPile(gameAfter->discard[PLAYER], gameAfter->discardCount[PLAYER], silver) + countCardInPile(gameAfter->discard[PLAYER], gameAfter->discardCount[PLAYER], gold) + countCardInPile(gameAfter->deck[PLAYER], gameAfter->deckCount[PLAYER], copper) + countCardInPile(gameAfter->deck[PLAYER], gameAfter->deckCount[PLAYER], silver) + countCardInPile(gameAfter->deck[PLAYER], gameAfter->deckCount[PLAYER], gold);

    // get hand and deck/discard size before any effects
    handCountBefore = gameBefore->handCount[PLAYER];
    discardDeckCountBefore = gameBefore->discardCount[PLAYER] + gameBefore->deckCount[PLAYER];

    // get hand and deck/discard size in the after game
    handCountAfter = gameAfter->handCount[PLAYER];
    discardDeckCountAfter = gameAfter->discardCount[PLAYER] + gameAfter->deckCount[PLAYER];

    // the adventurers in hand in the after state should be 1 less than the adventurers in hand in the before state
    if (numAdventurerHandAfter == numAdventurerHandBefore - 1)
        ;
    else
        return 1;

    // the adventurers in deck/discard in the after state should be 1 more than the adventurers in deck/discard in the before state
    if (numAdventurerDiscardDeckAfter == numAdventurerDiscardDeckBefore + 1)
        ;
    else
        return 1;

    // the treasure in hand in the after state should be addedTreasure more than the treasure in hand in the before state
    if (numTreasureHandAfter == numTreasureHandBefore + addedTreasure)
        ;
    else
        return 1;

    // the treasure in deck/discard in the after state should be addedTreasure less than the treasure in deck/discard in the before state
    if (numTreasureDiscardDeckAfter == numTreasureDiscardDeckBefore - addedTreasure)
        ;
    else
        return 1;

    // the hand count in the after state should be addedTreasure-1 (added addedTreasure to hand and lost 1 adventurer) more thanhte hand count before
    if (handCountAfter == handCountBefore + addedTreasure - 1)
        ;
    else
        return 1;

    // the deck/discard count in the after state should be 1-addedTreasure (added an adventurer and lost addedTreasure) more than the disk/discard count before
    if (discardDeckCountAfter == discardDeckCountBefore + 1 - addedTreasure)
        ;
    else
        return 1;
    
    return 0;

}

// executes the full set of code to run one test for player 0 to use one adventurer on a randomly set up game, includes pass/fail printouts
int runOneRandomTestForAdventurerEffect()
{
    int cardEffectReturnActual, cardEffectReturnExpected, adventurerHandPos, numTreasureInDeck, numTreasureInDiscard;

    struct gameState gameBefore, gameAfter;

    // randomize the game state and get position of adventurer from the state
    adventurerHandPos = randomizeGameForAdventurerTests(&gameBefore);

    // make both before and after the same
    memcpy(&gameAfter, &gameBefore, sizeof(struct gameState));

    // call the function under test on the after game
    cardEffectReturnActual = cardEffect(adventurer, 0, 0, 0, &gameAfter, adventurerHandPos, NULL); // choices and bonus are not used in adventurer
    cardEffectReturnExpected = 0;

    // depending on the state of the game before playing adventurer, there's a few scenarios that could occur (see comments in the upcoming if-else tree)

    // to know what to expect we need to know how many treasure are in the player's deck and discard
    numTreasureInDeck = countCardInPile(gameBefore.deck[PLAYER], gameBefore.deckCount[PLAYER], copper) + countCardInPile(gameBefore.deck[PLAYER], gameBefore.deckCount[PLAYER], silver) + countCardInPile(gameBefore.deck[PLAYER], gameBefore.deckCount[PLAYER], gold);
    numTreasureInDiscard = countCardInPile(gameBefore.discard[PLAYER], gameBefore.discardCount[PLAYER], copper) + countCardInPile(gameBefore.discard[PLAYER], gameBefore.discardCount[PLAYER], silver) + countCardInPile(gameBefore.discard[PLAYER], gameBefore.discardCount[PLAYER], gold);

    // scenario 1: player had exactly 1 treasure card in their hand and deck collectivley
    if (numTreasureInDeck + numTreasureInDiscard == 1)
    {
        // only 1 treasure could be added to hand in this case
        if (1 == compareGameStatesAdventurer(&gameBefore, &gameAfter, 1)) return 1;

    }

    // scenario 2: player had 2 or more treasure cards in their hand and deck collectivley
    else if (numTreasureInDeck + numTreasureInDiscard >= 2)
    {
        // 2 treasure get added to hand in this case
        if (1 == compareGameStatesAdventurer(&gameBefore, &gameAfter, 2)) return 1;

    }

    // scenario 3: player had exactly 0 treasure cards in their hand and deck collectivley
    else // (only remaining logic here is that numTreasureInDeck + numTreasureInDiscard is 0 or less, but negative isn't possible, so we're just left w/ 0)
    {
        // no treasure added to hand in this case
        if (1 == compareGameStatesAdventurer(&gameBefore, &gameAfter, 0)) return 1;

    }

    // compare return value to expected (it should always be zero, adventurer can't "fail" even if it draws no treasure, it didn't fail it just didn't do anything useful)
    if (cardEffectReturnActual == cardEffectReturnExpected)
        ;
    else
        return 1;

    // check if any side effects occurred
    if (didSideEffectsOccur(&gameBefore, &gameAfter) == 0)
        ;
    else
        return 1;
    
    return 0;
}

int main(int argc, char *argv[])
{
    int chosenTestCase = strtol(argv[1], 0, 10);
    
    if (chosenTestCase < 1 || chosenTestCase > 1000)
        return 2;

    //int i;

    // initialize random generator
    // to avoid n^2 behavior on this, we'll use a different seed each time, intead of having "the same" results of each 1000 and running 1-i each time we call this program
    //SelectStream(chosenTestCase);
    //PutSeed(chosenTestCase+1);
    srand(chosenTestCase);

    // run a bunch of random tests on smithy effect (includes printouts for pass/fail)
    //for (i = 1; i <= 1000; ++i)
    //{
        return runOneRandomTestForAdventurerEffect();
    //}

    return 2;
}
