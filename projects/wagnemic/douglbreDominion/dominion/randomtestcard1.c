#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "rngs.h"
#include "interface.h"

/* Random tests for the Smithy card
 *
 * The Smithy card is supposed draw 3 cards from the player's deck
 * if there aren't enough cards in their deck, the discard pile gets shuffled and made into a new deck and drawing continues
 * if there's not enough cards in both their discard and deck, the player just gets however many cards they can get
 * then Smithy then gets discarded
 * the purpose of these tests is to check this functionality and get good coverage of the function using the random testing methodology
 * the random setup is as follows (but see randomizeGameForSmithyTests for implementation):
 * the game state is set to completley random bytes
 * the hand is given a random count from 1 thru 5 inclusive
 * an smithy is put in one of those spots
 * the rest of the hand is set to random cards
 * the deck gets 0 thru 5 random cards
 * the discard gets 0 thru 5 random cards
 * note i'm only going to 5 cards in a pile because the interesting partitions are closer to 0 cards in pile than MAX_DECK, and all those large number of cards in pile random games won't help increase coverage
 * whose turn variable is set to player 0 (only running tests on 1 player, if we ran it on other player there woudln't be a difference for the smithy card)
 * numPlayers is set to a random value from 0-4, inclusive
 * playedCardCount is set to a random value from 0 through MAX_DECK-10, because a functionality i'm considering a bug will actually put cards in the played card array
 * and random cards are assigned to the played cards array because we'll be checking for side effects, might as well have valid cards in there
 * after game setup, the before state is saved and card effect is called on the game
 * the oracle used is one that checks the state of the game before smithy is played to the state after
 * depending on the state when card effect is called, 1 of 10 scenarios occur (scenarios 2-10 involve a shuffle):
 * 1: 3 draws from deck, 0 from discard
 * 2: 2 draws from deck, 1 from discard
 * 3: 2 draws from deck, 0 from discard
 * 4: 1 draw from deck, 2 from discard
 * 5: 1 draw from deck, 1 from discard
 * 6: 1 draw from deck, 0 from discard
 * 7: 0 draws from deck, 3 from discard
 * 8: 0 draws from deck, 2 from discard
 * 9: 0 draws from deck, 1 from discard
 * 10: 0 draws from deck, 0 from discard
 * because this action card involves drawing, we can't always predict the specific cards drawn after a shuffle
 * so the best we can do to compare is check the card counts of each pile against the expected card counts of each pile for each scenario
 * that is what the oracle is (see compareGameStatesSmithy for implementation of oracle)
 *
 */

// only player 0 will be playing smithy
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

// this function will modify the given game state to a random state in preparation to execute the smithy card effect on the state
// the deck, hand, and discard are given up to 5 random cards from all cards in the game (all our partisions are near 0, so it makes sense to have a low upper bound as the cases with more cards in each pile are the same as ones with less)
// aside form values that must be set for a well-formed game that meets preconditions, all other values of the game state are set to something random (the smithy card isn't supposed to modify those)
// return value is the hand position of the smithy card in the player's hand
int randomizeGameForSmithyTests(struct gameState * G)
{
    int i, smithyPos;

    // initialize the game to completley random values to start with
    for (i = 0; i < sizeof(struct gameState); i++)
        ((char*)G)[i] = (char)floor(rand() % 256);

    // player's hand count is set to a random value from 1 though 5 (there must be at least 1 because we need to ahve an smithy card in there)
    G->handCount[PLAYER] = 1 + (rand() % 5);

    // hand pos of the smithy card is 0 through handCount-1
    smithyPos = rand() % G->handCount[PLAYER];

    // place the smithy in the hand
    G->hand[PLAYER][smithyPos] = smithy;

    // player's hand contents are randomly generated from the 26 possible cards in the game 
    // but at least 1 needs to be a smithy because that's a precondition of the function
    for (i = 0; i < G->handCount[PLAYER]; ++i)
    {
        if (i == smithyPos) // do not overwrite the smithy card
            continue;
        G->hand[PLAYER][i] = rand() % (treasure_map + 1);
    }

    // player's discard count is set to a random value from 0 though 5
    G->discardCount[PLAYER] = rand() % 6;

    // player's discard contents are randomly generated from the 26 possible cards in the game 
    for (i = 0; i < G->handCount[PLAYER]; ++i)
        G->discard[PLAYER][i] = rand() % (treasure_map + 1);

    // player's deck count is set to a random value from 0 though 5
    G->deckCount[PLAYER] = rand() % 6;

    // player's discard contents are randomly generated from the 26 possible cards in the game 
    for (i = 0; i < G->handCount[PLAYER]; ++i)
        G->deck[PLAYER][i] = rand() % (treasure_map + 1);

    // need to set the turn to the player we'll be using to play the card... this can't be random since its part of the precondition for playing the card
    G->whoseTurn = PLAYER;

    // allow a game to have 0-4 players, inclusive (lets us see if this matters... it shouldn't)
    G->numPlayers = rand() % 5;

    // this is here because of a bug (or at least what i assume to be a bug based on my interpretation of the dominion code)
    // the bug in discardCard uses this value and it can't be garbage otherwise it may crash the program
    // little space available at end of the array to let the game put cards in there... (it will b/c of bug if discardCard is called)
    G->playedCardCount = rand() % (MAX_DECK - 9);

    // might as well initialize to valid values for this array since we'll be checking if its content changed
    for (i = 0; i < G->playedCardCount; ++i)
        G->playedCards[i] = rand() % (treasure_map + 1);

    // caller of this funciton needs the position of the smithy to play
    return smithyPos;
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
        anyFailure = 1;
        printf("FAIL when checking if no change to numPlayers (expected = %d, actual = %d)\n", gameBefore->numPlayers, gameAfter->numPlayers);
    }

    for (i = 0; i < treasure_map + 1; ++i)
        if (gameBefore->supplyCount[i] != gameAfter->supplyCount[i])
        {
            anyFailure = 1;
            cardNumToName(i, buffer);
            printf("FAIL when checking if no change to %s supply count (expected = %d, actual = %d)\n", buffer, gameBefore->supplyCount[i], gameAfter->supplyCount[i]);
        }

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

    // player 0 should not be part of side effect checks (its supposed to have game state for it change)
    for (j = 1; j < MAX_PLAYERS; ++j)
        if (gameBefore->deckCount[j] != gameAfter->deckCount[j])
        {
            anyFailure = 1;
            printf("FAIL when checking if no change to player %d's deck count (expected = %d, actual = %d)\n", j, gameBefore->deckCount[j], gameAfter->deckCount[j]);
        }

    for (j = 1; j < MAX_PLAYERS; ++j)
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

// carries out the test to check if the expected games state for player 0 is the same as in the actual game, given how many cards should have been drawn and from what piles
void compareGameStatesSmithy(struct gameState *gameBefore, struct gameState *gameAfter, int cardsDrawnFromDeck, int cardsDrawnAfterShuffle)
{
    int handCountBefore, discardCountBefore, deckCountBefore, handCountAfter, discardCountAfter, deckCountAfter;

    // because there's a random shuffle involved, we can't expect an exact game state (like checking for specific cards in specific piles)
    // the best we can do is check counts of piles to make sure they're what they shuold be when cardsDrawnFromDeck are drawn from deck and cardsDrawnAfterShuffle are drawn from new deck after shuffling discard into new deck)
    // unlike adventuerer, we can't compare number of smithy cards in hand before/after b/c its possible that a smithy was 1 or more of the cards drawn
    // w/ adventurer, only treasure are drawn and never an adventurer so that comparison is possible

    // get hand, discard, deck counts before any effects
    handCountBefore = gameBefore->handCount[PLAYER];
    discardCountBefore = gameBefore->discardCount[PLAYER];
    deckCountBefore = gameBefore->deckCount[PLAYER];

    // get hand, discard, deck counts after smithy effect
    handCountAfter = gameAfter->handCount[PLAYER];
    discardCountAfter = gameAfter->discardCount[PLAYER];
    deckCountAfter = gameAfter->deckCount[PLAYER];

    // the hand count in the after state should be cardsDrawnFromDeck + cardsDrawnAfterShuffle - 1 (the -1 is for the lost smithy) more than hand count before 
    if (handCountAfter == handCountBefore + cardsDrawnFromDeck + cardsDrawnAfterShuffle - 1)
        printf("PASS when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore + cardsDrawnFromDeck + cardsDrawnAfterShuffle - 1, handCountAfter, handCountBefore);
    else
        printf("FAIL when checking hand count (expected = %d, actual = %d, before cardEffect = %d)\n", handCountBefore + cardsDrawnFromDeck + cardsDrawnAfterShuffle - 1, handCountAfter, handCountBefore);

    if (cardsDrawnFromDeck == 3) // no shuffle occurred in this scenario
    {
        // discard count should have 1 more card that before (the discarded smithy)
        if (discardCountAfter == discardCountBefore + 1)
            printf("PASS when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore + 1, discardCountAfter, discardCountBefore);
        else
            printf("FAIL when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore + 1, discardCountAfter, discardCountBefore);

        // cardsDrawnFromDeck (must be 3 at this point) cards should be taken from the deck
        if (deckCountAfter == deckCountBefore - cardsDrawnFromDeck)
            printf("PASS when checking deck count (expected = %d, actual = %d, before cardEffect = %d)\n", deckCountBefore - cardsDrawnFromDeck, deckCountAfter, deckCountBefore);
        else
            printf("FAIL when checking deck count (expected = %d, actual = %d, before cardEffect = %d)\n", deckCountBefore - cardsDrawnFromDeck, deckCountAfter, deckCountBefore);
    }
    else // a shuffle occurred in these scenarios
    {
        // discard count must be 1 in these scenarios (the smithy discarded after drawing the cards, whatever else was in the discard was moved into the deck when reshuffle occurred)
        if (discardCountAfter == 1)
            printf("PASS when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", 1, discardCountAfter, discardCountBefore);
        else
            printf("FAIL when checking discard count (expected = %d, actual = %d, before cardEffect = %d)\n", 1, discardCountAfter, discardCountBefore);

        // the count of the deck in the after state must the count of the before discard less the cards drawn after the shuffle (cards drawn from deck are in the hand)
        if (deckCountAfter == discardCountBefore - cardsDrawnAfterShuffle)
            printf("PASS when checking deck count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore - cardsDrawnAfterShuffle, deckCountAfter, deckCountBefore);
        else
            printf("FAIL when checking deck count (expected = %d, actual = %d, before cardEffect = %d)\n", discardCountBefore - cardsDrawnAfterShuffle, deckCountAfter, deckCountBefore);
    }

}

// executes the full set of code to run one test for player 0 to use one smithy on a randomly set up game, includes pass/fail printouts
void runOneRandomTestForSmithyEffect()
{
    int cardEffectReturnActual, cardEffectReturnExpected, smithyHandPos, numCardsInDeck, numCardsInDiscard;

    struct gameState gameBefore, gameAfter;

    // randomize the game state and get position of smithy from the state
    smithyHandPos = randomizeGameForSmithyTests(&gameBefore);

    // make both before and after the same
    memcpy(&gameAfter, &gameBefore, sizeof(struct gameState));

    // call the function under test on the after game
    cardEffectReturnActual = cardEffect(smithy, 0, 0, 0, &gameAfter, smithyHandPos, NULL); // choices and bonus are not used in smithy
    cardEffectReturnExpected = 0;

    // depending on the state of the game before playing smithy, there's a few scenarios that could occur (see comments in the upcoming if-else tree)

    // to know what to expect we need to know how many cards are in the player's deck and discard
    numCardsInDeck = gameBefore.deckCount[PLAYER];
    numCardsInDiscard = gameBefore.discardCount[PLAYER];

    // scenario 1: player has 3 or more cards in their deck
    if (numCardsInDeck >= 3)
    {
        // all 3 cards are drawn from the player's deck
        printf("Results from a scenario 1 game (3 cards drawn from deck):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 3, 0);
    }

    // scenario 2: player has 2 cards in deck and at least 1 in discard
    else if (numCardsInDeck == 2 && numCardsInDiscard >= 1)
    {
        // 2 cards are drawn from deck, discard is shuffled into new deck and 1 card is drawn from new deck
        printf("Results from a scenario 2 game (2 cards drawn from deck, 1 drawn from new deck after shuffle):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 2, 1);
    }

    // scenario 3: player has 2 cards in deck and 0 in discard
    else if (numCardsInDeck == 2 && numCardsInDiscard == 0)
    {
        // 2 cards are drawn from deck, no discard left to shuffle and draw, so player just draws the 2 cards
        printf("Results from a scenario 3 game (2 cards drawn from deck, 0 cards drawn from new deck that is empty):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 2, 0);
    }

    // scenario 4: player has 1 card in deck in deck and at least 2 in discard
    else if (numCardsInDeck == 1 && numCardsInDiscard >= 2)
    {
        // 1 card drawn from deck, discard is shuffled into new deck and 2 cards drawn from new deck
        printf("Results from a scenario 4 game (1 card drawn from deck, 2 cards drawn from new deck after shuffle):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 1, 2);
    }

    // scenario 5: player has 1 card in deck in deck and 1 in discard
    else if (numCardsInDeck == 1 && numCardsInDiscard == 1)
    {
        // 1 card drawn from deck, discard is shuffled into new deck and 1 card drawn from new deck
        printf("Results from a scenario 5 game (1 card drawn from deck, 1 card drawn from new deck after shuffle):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 1, 1);
    }

    // scenario 6: player has 1 card in deck and 0 in discard
    else if (numCardsInDeck == 1 && numCardsInDiscard == 0)
    {
        // 1 card drawn from deck, no discard left to shuffle and draw, so player just draws the 1 card
        printf("Results from a scenario 6 game (1 card drawn from deck, 0 cards drawn from new deck that is empty):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 1, 0);
    }

    // scenario 7: player has 0 cards in deck and at least 3 in discard
    else if (numCardsInDeck == 0 && numCardsInDiscard >= 3)
    {
        // 0 cards drawn from deck, discard is shuffled into new deck and 3 cards drawn from new deck
        printf("Results from a scenario 7 game (0 cards drawn from deck, 3 cards drawn from new deck after shuffle):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 0, 3);
    }

    // scenario 8: player has 0 cards in deck and 2 in discard
    else if (numCardsInDeck == 0 && numCardsInDiscard == 2)
    {
        // 0 cards drawn from deck, discard is shuffled into new deck and 2 cards drawn from new deck
        printf("Results from a scenario 8 game (0 cards drawn from deck, 2 cards drawn from new deck after shuffle):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 0, 2);
    }

    // scenario 9: player has 0 cards in deck and 1 in discard
    else if (numCardsInDeck == 0 && numCardsInDiscard == 1)
    {
        // 0 cards drawn from deck, discard is shuffled into new deck and 1 card drawn from new deck
        printf("Results from a scenario 9 game (0 cards drawn from deck, 1 card drawn from new deck after shuffle):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 0, 1);
    }

    // scenario 10: player has 0 cards in deck and 0 in discard
    else // last remaining option (because numCards in deck or dsicard can't be negative) is that both deck and discard are empty
    {
        // 0 cards drawn from deck, discard is shuffled into new deck and 1 card drawn from new deck
        printf("Results from a scenario 10 game (0 cards drawn from deck, 0 cards drawn from new deck that is empty):\n");
        compareGameStatesSmithy(&gameBefore, &gameAfter, 0, 0);
    }

    // compare return value to expected (it should always be zero, smithy can't "fail" even if it draws less than 3 cards... you just don't get cards you don't have in deck/discard)
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
    printf("Random Tests Card 1 - Smithy:\n");

    int i;

    // initialize random generator
    SelectStream(2);
    PutSeed(3);

    // run a bunch of random tests on smithy effect (includes printouts for pass/fail)
    for (i = 0; i < 1000; ++i)
        runOneRandomTestForSmithyEffect();

    return 0;
}
