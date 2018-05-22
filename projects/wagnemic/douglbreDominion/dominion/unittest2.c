#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"

/*
 * Unit tests for gainCard function
 *
 * code inspection of gainCard shows that the only parts of the game state that get modified are a player's discard pile, deck, or hand, and the supply of the gained card
 * so, those and the return value for failure cases are the only things we need to test for
 * a gained card to any pile should reduce that supply by 1 and add the particular card to the top of the pile to which it was gained
 * gainCard should fail when the supply is empty or not used in the game
 * so, the tests used are 
 * 1. check for -1 return value when card is attempted to be gained from an empty or unused supply, and 0 if the supply is in the game and nonzero
 * 2. check if the supply was decremented properly (when the return is 0 only)
 * 3. check if each player's discard, hand, and deck have the expected count value and contents (when the return is 0 only)
 *
 */

void cardNumToName(int card, char *name);

int main()
{
    int seed = 68;
    int numPlayers = 2;
    int curPlayer, curCard, curFlag, i, l, actualReturn, expectedReturn;
    int countAfter[3][MAX_PLAYERS]; // the 3 index is for gaining to discard, deck or hand (0, 1, 2, respectively)
    int expectedCountAfter[3][MAX_PLAYERS];
    int pileAfter[3][MAX_PLAYERS][MAX_HAND]; // MAX_HAND must equal MAX_DECK for this to work, which it does
    int expectedPileAfter[3][MAX_PLAYERS][MAX_HAND];
    int supplyBefore;
    int supplyAfter;
    int expectedSupplyAfter;
    char gainLocation[3][100];
    char supplyDescription[100];
    char cardName[100];
    struct gameState cleanGame, testGame, holdGame;

    // this set of kingdom cards is all thats needed because we need to use one card that's not used in the game
    int k[10] = { adventurer, council_room, feast, gardens, mine, remodel, smithy, village, baron, great_hall };

    // for output
    strcpy(gainLocation[0], "discard pile");
    strcpy(gainLocation[1], "deck");
    strcpy(gainLocation[2], "hand");

    printf("Unit Tests Function 2 - gainCard():\n");

    // initialize a clean game state for numPlayers players
    memset(&cleanGame, 0, sizeof(struct gameState));
    initializeGame(numPlayers, k, seed, &cleanGame);

    // for every player
    for (curPlayer = 0; curPlayer < numPlayers; ++curPlayer)
    {
        // for a card in the game with non empty supply, a card in the game with an empty supply, and a card not in the game (adventurer, remodel, and minion, respectivley)
        for (curCard = adventurer; curCard <= minion; curCard = curCard + 5)
        {
            // for gaining to discard, deck, or hand (0, 1, 2, respectively)
            for (curFlag = 0; curFlag <= 2; ++curFlag)
            {
                memcpy(&testGame, &cleanGame, sizeof(struct gameState)); // set testGame to a clean game

                // set remodel supply to 0
                testGame.supplyCount[remodel] = 0;

                // save the relavent parts of game state before calling gainCard for each player in the game
                for (i = 0; i < numPlayers; ++i)
                {
                    supplyBefore = testGame.supplyCount[curCard];

                    // storing these now because the expected pile is just one extra card at the top depending on what player got what card in what pile
                    memcpy(expectedPileAfter[0][i], testGame.discard[i], MAX_HAND * sizeof(int));
                    memcpy(expectedPileAfter[1][i], testGame.deck[i], MAX_HAND * sizeof(int));
                    memcpy(expectedPileAfter[2][i], testGame.hand[i], MAX_HAND * sizeof(int));
                    expectedCountAfter[0][i] = testGame.discardCount[i];
                    expectedCountAfter[1][i] = testGame.deckCount[i];
                    expectedCountAfter[2][i] = testGame.handCount[i];
                }

                cardNumToName(curCard, cardName); // save current card name

                memcpy(&holdGame, &testGame, sizeof(struct gameState)); // before call to gainCard, save state of the testGame (which has been modified from cleanGame state)

                actualReturn = gainCard(curCard, &testGame, curFlag, curPlayer); // player curPlayer gains card curCard into location id'd by curFlag

                // store expected return and a description of the supply it was gained from
                if (curCard == adventurer)
                {
                    expectedReturn = 0;
                    strcpy(supplyDescription, "with available supply");
                }
                else if (curCard == remodel)
                {
                    expectedReturn = -1;
                    strcpy(supplyDescription, "with no supply");
                }
                else // if (curCard == minion)
                {
                    expectedReturn = -1;
                    strcpy(supplyDescription, "not in the game");
                }

                // 1. check for the return value, -1 for remodel and minion, and 0 for adventurer
                if (expectedReturn == actualReturn)
                    printf("gainCard(): PASS when player %d attempted to gain a card %s into the %s (expected return = %d, actual return = %d)\n", curPlayer, supplyDescription, gainLocation[curFlag], expectedReturn, actualReturn);
                else
                    printf("gainCard(): FAIL when player %d attempted to gain a card %s into the %s (expected return = %d, actual return = %d)\n", curPlayer, supplyDescription, gainLocation[curFlag], expectedReturn, actualReturn);
                
                // also want to check if game state was unchanged due to a failure
                if (curCard == remodel || curCard == minion)
                {
                    // make sure testGame is the same state as holdGame after a failed execution of a failed gainCard
                    if (memcmp(&testGame, &holdGame, sizeof(struct gameState)) == 0)
                        printf("gainCard(): PASS when checking for unintended side effects on the game after the previous fail-return expected invocation\n");
                    else
                        printf("gainCard(): FAIL when checking for unintended side effects on the game after the previous fail-return expected invocation\n");

                    continue; // continue to next test because there's nothing else to check for remodel and minion
                }

                // now we're only working with adventurer

                // save the relavent parts of game state after calling gainCard for each player in the game
                for (i = 0; i < numPlayers; ++i)
                {
                    supplyAfter = testGame.supplyCount[curCard];
                    memcpy(pileAfter[0][i], testGame.discard[i], MAX_HAND * sizeof(int));
                    memcpy(pileAfter[1][i], testGame.deck[i], MAX_HAND * sizeof(int));
                    memcpy(pileAfter[2][i], testGame.hand[i], MAX_HAND * sizeof(int));
                    countAfter[0][i] = testGame.discardCount[i];
                    countAfter[1][i] = testGame.deckCount[i];
                    countAfter[2][i] = testGame.handCount[i];
                }


                // 4. check if supply was decremented properly

                expectedSupplyAfter = supplyBefore - 1;

                if (expectedSupplyAfter == supplyAfter)
                    printf("gainCard(): PASS when checking the supply count when player %d gained a card to their %s (expected supply = %d, actual supply = %d)\n", curPlayer, gainLocation[curFlag], expectedSupplyAfter, supplyAfter);
                else
                    printf("gainCard(): FAIL when checking the supply count when player %d gained a card to their %s (expected supply = %d, actual supply = %d)\n", curPlayer, gainLocation[curFlag], expectedSupplyAfter, supplyAfter);


                // 3. check if each player's discard, hand, and deck have the expected count value and contents

                // check all players in the game (i loops over this)
                for (i = 0; i < numPlayers; ++i)
                {
                    // this loops over the 3 possible location of cards (discard, hand, deck) for the player i
                    for (l = 0; l <= 2; ++l)
                    {
                        // if player i is the player gaining the card and location l is the locaiton to which that card was gained
                        // then their expected pile should have one adventurer more card on the top (recall we saved the expected values as the before values earlier)
                        if (i == curPlayer && l == curFlag)
                            expectedPileAfter[l][i][expectedCountAfter[l][i]++] = adventurer;

                        // check if the count of location l (discard, hand deck) for player i is what it should be
                        if (expectedCountAfter[l][i] == countAfter[l][i])
                            printf("gainCard(): PASS when checking player %d's %s count when player %d gained a card to their %s (expected count = %d, actual count = %d)\n", i, gainLocation[l], curPlayer, gainLocation[curFlag], expectedCountAfter[l][i], countAfter[l][i]);
                        else
                            printf("gainCard(): FAIL when checking player %d's %s count when player %d gained a card to their %s (expected count = %d, actual count = %d)\n", i, gainLocation[l], curPlayer, gainLocation[curFlag], expectedCountAfter[l][i], countAfter[l][i]);

                        // check if the pile l (discard, hand, deck) for player i is what it should be
                        if (memcmp(expectedPileAfter[l][i], pileAfter[l][i], MAX_HAND*sizeof(int)) == 0)
                            printf("gainCard(): PASS when checking player %d's %s state when player %d gained a card to their %s\n", i, gainLocation[l], curPlayer, gainLocation[curFlag]);
                        else
                            printf("gainCard(): FAIL when checking player %d's %s state when player %d gained a card to their %s\n", i, gainLocation[l], curPlayer, gainLocation[curFlag]);
                    }
                }
            }
        }
    }

    return 0;
}