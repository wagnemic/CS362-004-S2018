#ifndef _DOMINION_HELPERS_H
#define _DOMINION_HELPERS_H

#include "dominion.h"

int drawCard(int player, struct gameState *state);
int updateCoins(int player, struct gameState *state, int bonus);
int discardCard(int handPos, int currentPlayer, struct gameState *state, 
		int trashFlag);
int gainCard(int supplyPos, struct gameState *state, int toFlag, int player);
int getCost(int cardNumber);
int cardEffect(int card, int choice1, int choice2, int choice3, 
	       struct gameState *state, int handPos, int *bonus);

// prototypes for the 6 re-factored card effects (1 more than the original 5 because I chose to test a different effect than those I chose to refactor in week 2)
int smithyEffect(int smithyHandPos, struct gameState *state);
int adventurerEffect(struct gameState *state);
int villageEffect(int villageHandPos, struct gameState *state);
int councilRoomEffect(int councilRoomHandPos, struct gameState *state);
int embargoEffect(int embargoHandPos, struct gameState *state, int supplyToEmbargo);
int remodelEffect(int remodelHandPos, struct gameState *state, int handPosToRemodel, int supplyPosToGain);

#endif
