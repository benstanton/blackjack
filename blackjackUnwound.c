/* 	Program: 		blackjackUnwound!
	Author:			Benjamin Francis Stanton
	Created on:		17/05/2021
	Last modified:	20/05/2021
	Description:	A fully fleshed out game of blackjack vs a computer dealer, with betting,
	.				highscore, and save/load functionality! Player must attempt to build a 
	.				winning hand from the cards dealt. Picture cards all have a value of 10,
	.				with the exception of ACE, which can be ONE or ELEVEN.
	.				
	.				Hands in order as follows: 
	.				BLACKJACK (ace and 10 value card), 
	.				FIVE CARD TRICK (hand of five with total value under 21)
	.				TWENTY ONE (exactly 21 from a non ACE-TEN combination)
	.				HIGHCARD (less than 21)
	.				BUST (more than 21)
	.				
	.				BLACKJACK and FIVE CARD TRICK are unique in that the winner receives
	.				double the staked bet.
	.				
	.				Both player and dealer are dealt one card face up, at which point the 
	.				player chooses his initial bet, between 1 and 10.
	.				
	.				Player and dealer are then dealt a second card face down. If the dealer 
	.				has BLACKJACK this is immediately made clear, and unless player has 
	.				BLACKJACK, the dealer wins twice the bet.
	.				
	.				Player then choses from the following:
	.				BUY: draw new card, increasing bet between initial bet and 2x initial bet
	.				TWIST: receive a new card without increasing bet
	.				STICK: receive no further cards, wait for dealer to play
	.				
	.				If player goes BUST from drawing new cards, he loses.
	.				
	.				Once the player is happy with his hand he sticks, and it is the dealer's
	.				turn. The dealer must continue to draw cards until he either goes BUST or
	.				his hand is equal or greater than 17.
	.				
	.				If neither player goes BUST, the higher valued hand wins. If the player
	.				and the dealer both have the same valued hand, the dealer wins the round.
	.				
	.				The rules have been adapted from https://www.pagat.com/banking/pontoon.html
*/
	
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

enum suitEnum {DIAMONDS=1, HEARTS, CLUBS, SPADES};
enum kindEnum {ACE=1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING};
enum handRankingEnum {BLACKJACK = 1, FIVE_CARD_TRICK, TWENTYONE, NOT_BUST, BUST}; //represents a card ranking

struct card{ //represents a card
	enum suitEnum suit;
	enum kindEnum kind;
};

struct player{ //used to save player scores to leaderboard
	char name[16];
	long score;
};

struct gameVars{ //used to save game variables
	long money;
	int handNumber;
	int drawPosition;
	int initialBet;
	int totalBet;
	int firstBuy;
	int gameState;
};

//reading card king/suit string from struct->enum
const char* cardKind(struct card);
const char* cardSuit(struct card);

//deck maniuplation functions
void deckDisplay(struct card *deck); //only used for debugging
void deckPopulate(struct card deck[52]); //populates deck with unshuffled cards
void deckShuffle(struct card *deck); //fisher yates shuffle (see https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle)
void resetCards(struct card *deck, struct card *playerHand, struct card *dealerHand); //empties hands, reshuffles deck 

//display hand functions
void displayFirstCard(struct card hand[5]); //for first round with dealer
void displayHand(struct card *hand);
void topDraw(struct card *hand, struct card *deck, int *position);

//calculating details of dealer/player hand
int handValue(struct card *hand);
int handSize(struct card *hand);
enum handRankingEnum handResolve(struct card *hand); //returns the ranking of hand (blackjack, bust, etc)

////functions to save/load////
//only when user chooses "LOAD GAME"
void loadGame(FILE **fIO, struct player *leaderboard, struct player *currentPlayer, struct card *deck, struct card *playerHand, struct card *dealerHand, struct gameVars *loadVars);
int validFile(FILE **fIO);

//called at the start in case player starts new game, only leaderboard carries over
void loadLeaderboard(FILE **fIO, struct player *leaderboard); 
void saveGame(FILE **fIO, struct player *leaderboard, struct player currentPlayer, struct card *deck, struct card *playerHand, struct card *dealerHand, struct gameVars saveVars);

//title menu function
int titleMenu();
void displayInfo();

//sets up a new game (assign variables etc)
void newGame(struct card *deck, struct card *playerHand, struct card *dealerHand, struct player *currentPlayer);

/////quality of life functions////
	//art of https://patorjk.com/software/taag
void printArt();
void printGameOver();
void printHeader(const char *playerName, int handNumber, long money, int initialBet, int totalBet, long score);
void printHighScore();
void enterToContinue(); //simple press enter to continue function

//swapping functions via pointers
void swapCard(struct card *i,struct card *j); //swap card used in shuffle
void swapPlayer(struct player *i, struct player *j); //swaping players during bubble sort

//functions for getting numeric input 
int multiDigitInput(int lowerLimit, int upperLimit);
int singleDigitInput(char lowerLimit, char upperLimit);


// leaderboard functions
void resetLeaderboard(struct player *leaderboard); // initializing leaderboard array prior to filling
void updateLeaderboard(struct player *leaderboard, struct player currentPlayer); //updates leaderboard 

int main(){

	srand(time(NULL)); //randomize seed
	
	//initializing program variablles
	int exitProgram = 0;
	int exitGame = 1;
	int titleChoice = 0;
	int userInput = -1;
	
	FILE *fIO; //file input
	
	//game variables
	struct player leaderboard[10];
	struct player currentPlayer;
	currentPlayer.score = 0;
	struct card deck[52];
	struct card playerHand[5];
	struct card dealerHand[5];
	struct gameVars gameVariables; //only used for saving/loading
	long money = 0; 
	int handNumber = 0;
	int drawPosition = 0;
	int initialBet = 0;
	int totalBet = 0;
	int firstBuy = 0;
	int gameState = 0;
	
	//initializing leaderboard values
	resetLeaderboard(leaderboard);
	
	//main exit loop
	while (!exitProgram){					// C7: LOOP TO START
		
		fIO = fopen("save.txt", "r"); //checking for file C3: FILE INPUT
		if (fIO != NULL){
		
		
		if (!validFile(&fIO)){ //checking save file has correct # of lines
			printf("ERROR: \"save.txt\" LIKELY TAMPERED WITH\nABORTING PROGRAM\n");
			enterToContinue();
			return(1); //main returns 1(error)
		}
		
		//loading leaderboard
		loadLeaderboard(&fIO, leaderboard);
		
		//title menu stuff
		titleChoice = titleMenu();
		switch (titleChoice){ 				// C1: SELECTION 
			case 1: //new game

				//newgame variable assignment
				currentPlayer.score = 0;
				handNumber = 0;
				money = 100;
				exitGame = 0;
				gameState = 0; 
				newGame(deck, playerHand, dealerHand, &currentPlayer); //setting up deck, hands, score
				break;
			case 2: //load game
				
				//loading game variables
				loadGame(&fIO, leaderboard, &currentPlayer, deck, playerHand, dealerHand, &gameVariables);
				
				if (gameVariables.money <= 0){ //if old save money < 0, starts new game instead
					//newgame variable assignment
					currentPlayer.score = 0;
					handNumber = 0;
					money = 100;
					exitGame = 0;
					gameState = 0; 
					newGame(deck, playerHand, dealerHand, &currentPlayer); //setting up deck, hands, score
				} else{
					//otherwise loads game variables from gameVariable struct
					drawPosition = gameVariables.drawPosition;	
					firstBuy = gameVariables.firstBuy;
					gameState = gameVariables.gameState;
					handNumber = gameVariables.handNumber;
					initialBet = gameVariables.initialBet;
					money = gameVariables.money;
					totalBet = gameVariables.totalBet;
					exitGame = 0;
				}
				break;	
				
			case 3: //leaderboard
				system("cls");
				printArt();
				printHighScore();
				for (int i = 0; i < 10; i++){
					if(i<9) //formating with leading 0
						printf("0%d) %s %d\n", i+1, leaderboard[i].name, leaderboard[i].score);
					else 
						printf("%d) %s %d\n", i+1, leaderboard[i].name, leaderboard[i].score);
				}
				enterToContinue();
				break;
			
			case 4: //info
				displayInfo();
				break;
				
			case 5: //quit
				exitProgram = 1;
				break;		
		}
		
		//game playing loop
		while (!exitGame){ 					// C7: LOOP	
			switch (gameState){ 			// C1: SELECTION
				
				case 0: //start of round
					system("cls"); 
					
					//setting new round variables
					drawPosition = 0;
					initialBet = 0;
					totalBet = 0;
					firstBuy = -1;
					handNumber++;
					resetCards(deck, playerHand, dealerHand);
					
					printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
					
					//dealing opening cards
					playerHand[0] = deck[drawPosition++];
					dealerHand[0] = deck[drawPosition++];
						
					//displaying hands
					printf("DEARLER'S HAND:\n");
					displayHand(dealerHand);
					printf("YOUR HAND:\n");
					displayHand(playerHand);
					
					//placing initial bet
					printf("How much is your initial bet? Must be between $1 and $10\n");
					userInput = -1;
					while (userInput==-1){
						userInput = multiDigitInput(1, 10);
						if (userInput ==-1){
							printf("Correct input only, please\n");
						}
					}			
					initialBet = userInput;
					totalBet += initialBet;
					printf("You're betting $%d!\n", userInput);	
					
					//drawing second card
					playerHand[1] = deck[drawPosition++];
					dealerHand[1] = deck[drawPosition++];
					
					gameState = 1;
					break;
					
				case 1: //checking if dealer or player has blackjack
					
					
					if (handResolve(dealerHand) == BLACKJACK && handResolve(playerHand) == BLACKJACK){
						//both player hand dealer have blackjack, "pass" round
						
						system("cls");
						printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
						printf("DEARLER'S HAND:\n");
						displayHand(dealerHand);
						printf("YOUR HAND:\n");
						displayHand(playerHand);
						printf("You were both dealt blackjack!\nNothing is won or lost.\n");
						
						enterToContinue();
						gameState = 6;
					} else if (handResolve(dealerHand) == BLACKJACK && handResolve(playerHand) != BLACKJACK){
						//only dealer has blackjack, instant loss
						
						system("cls");
						money -= (2*initialBet);
						printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
						
						printf("DEARLER'S HAND:\n");
						displayHand(dealerHand);
						printf("YOUR HAND:\n");
						displayHand(playerHand);
						printf("Dealer was dealt blackjack!\nYou lose $%d.\n", (2*initialBet) );
						
						enterToContinue();
						gameState = 6;
					} else if (handResolve(playerHand) == BLACKJACK){ //player has blackjack, goes straight to resolving hand
						gameState = 3;
					} else{
						gameState = 2; //neither have blackjack, going to player turn
					}
					break;
				
				case 2: //player turn
					
					//showing shit
					system("cls");
					printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
					printf("DEALER'S HAND:\n");
					displayFirstCard(dealerHand);
					printf("YOUR HAND:\n");
					displayHand(playerHand);
					
					//checking if player has bought before this round
					if (firstBuy == -1){
						printf("\nYour turn:\n[1] BUY a card (between $%d, and $%d)\n[2] TWIST a card\n[3] STICK with current hand\n[4] SAVE and QUIT to title\n", initialBet, 2*initialBet);
					} else {
						printf("\nYour turn:\n[1] BUY a card (between $%d, and $%d)\n[2] TWIST a card\n[3] STICK with current hand\n[4] SAVE and QUIT to title\n", initialBet, firstBuy);
					}
					
					//getting user menu input
					userInput = -1;
					while (userInput == -1){
						userInput = singleDigitInput('1', '4');
						if (userInput == -1)
							printf("Correct input only, please\n");
					}
					
					//resolving user menu input
					switch (userInput){
						case 1: //buy
							printf("Please input how much you wish to buy for\n");
							userInput = -1;
							while (userInput==-1){
								
								//constraining buy values as appropreate (depends on if player has bought before this round)
								if (firstBuy == -1){
									userInput = multiDigitInput(initialBet, 2*initialBet);
								} else {
									userInput = multiDigitInput(initialBet, firstBuy);
								}
								
								if (userInput ==-1){
									printf("Correct input only, please\n");
								}
							}
							
							//setting first buy if appropriate
							if (firstBuy ==-1){
								firstBuy = userInput;
								totalBet += firstBuy;
							} else{
								totalBet += userInput;
							}
							
							topDraw(playerHand, deck, &drawPosition); //drawing card
							gameState = 3; //resolving hand
							break;
						case 2: //twist 
							topDraw(playerHand, deck, &drawPosition); //drawing card
							gameState = 3; //resolving hand			
							break;
						
						case 3: //stick
							printf("You've stuck with a hand value of %d.\nIt's now the dealer's turn.\n", handValue(playerHand));
							enterToContinue();
							gameState = 4;
							break;
						
						case 4: //save and quit
							
							updateLeaderboard(leaderboard, currentPlayer); //updating leaderboard(current player goes in if elegible)
							
							//setting gameVariable struct variables to save
							gameVariables.drawPosition = drawPosition; 			//C5: USE OF DRAWPOSITION VARIABLE
							gameVariables.firstBuy = firstBuy;
							gameVariables.gameState = gameState;
							gameVariables.handNumber = handNumber;
							gameVariables.initialBet = initialBet;
							gameVariables.money = money;
							gameVariables.totalBet = totalBet;							
							saveGame(&fIO, leaderboard, currentPlayer, deck, playerHand, dealerHand, gameVariables); 
							exitGame = 1;
							break;
					}
					break;
				case 3:{ //resolving player turn	
					enum handRankingEnum playerHandRanking;
					playerHandRanking = handResolve(playerHand); //function that returns playerh's hand rank
					switch (playerHandRanking)	{ //behaviour varies based on rank
						case BLACKJACK:
							//player cannot take more cards, dealer's turn
							system("cls");
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayFirstCard(dealerHand);
							printf("YOUR HAND:\n");
							displayHand(playerHand);
							printf("You've got blackjack! It's now the dealer's turn.\n");
							enterToContinue();
							gameState = 4;
							break;
						case FIVE_CARD_TRICK:
							//player cannot take more card, dealer's turn
							system("cls");
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayFirstCard(dealerHand);
							printf("YOUR HAND:\n");
							displayHand(playerHand);
							printf("You've got a five card trick! It's now the dealer's turn.\n");
							enterToContinue();
							gameState = 4;
							break;
						case TWENTYONE:
							//player cannot take more cards, dealer's turn
							system("cls");
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayFirstCard(dealerHand);
							printf("YOUR HAND:\n");
							displayHand(playerHand);
							printf("You've got 21! It's now the dealer's turn.\n");
							enterToContinue();
							gameState = 4;
							break;
							
						case NOT_BUST:
							//player can take more cards, takes another turn
							gameState = 2;
							break;

						case BUST:
							//player goes bust and loses the round
							money -= totalBet;
							system("cls");
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayFirstCard(dealerHand);
							printf("YOUR HAND:\n");
							displayHand(playerHand);
							printf("You've gone bust! You lose $%d!\n", totalBet);
							enterToContinue();
							gameState = 6;
							break;
					}
					break;
				}
					
				case 4: //dealer's turn
					system("cls");	
					printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
					printf("DEALER'S HAND:\n");
					displayHand(dealerHand);
					printf("YOUR HAND:\n");
					displayHand(playerHand);
					
					//if  dealer has more than 17 and isn't bust, he must stick
					if (handValue(dealerHand) >= 17 && handResolve(dealerHand) != BUST){
						printf("The dealer sticks with %d.\n", handValue(dealerHand));
						enterToContinue();
						gameState = 5;
					}
					
					//less than 17, dealer must hit
					while (handValue(dealerHand) < 17){
						printf("The dealer draws a new card.\n");
						enterToContinue();
						
						topDraw(dealerHand, deck, &drawPosition);
	
						if(handResolve(dealerHand) != NOT_BUST){ //if dealer has anything that isn't "not bust"
																 //i.e blackjack, FCT, 21, or BUST, game goes to resolve hands
							gameState = 5;
							break;
						} else{ //dealer isn't bust yet, taes another turn
							system("cls");	
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayHand(dealerHand);
							printf("YOUR HAND:\n");
							displayHand(playerHand);
						}
					}
					break;
				case 5:{ //resolving both hands
				
					enum handRankingEnum dealerRanking = handResolve(dealerHand); //resolving dealer hand
				
					switch (dealerRanking){
						case BUST: //dealer bust, player wins

							money += totalBet;
							currentPlayer.score +=totalBet;
							system("cls");	
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayHand(dealerHand);
							printf("YOUR HAND:\n");							
							displayHand(playerHand);						
							printf("The dealer went bust!\nYou win $%d\n", totalBet);	
							enterToContinue();	
							gameState = 6;
							break;
						
						case FIVE_CARD_TRICK:{ //dealer FCT, player only wins on blackjack

							char outString[80]; //formatting output string depending on win/loss
							if (handResolve(playerHand) == BLACKJACK){
								sprintf(outString, "Your BLACKJACK beats the dealer's FIVE CARD TRICK.\nYou win $%d!\n", 2*totalBet);
								money += 2*totalBet;
								currentPlayer.score += 2*totalBet;
							} else if (handResolve(playerHand) == FIVE_CARD_TRICK){
								sprintf(outString, "The dealer's FIVE CARD TRICK beats your FIVE CARD TRICK!\nYou lose $%d.", 2*totalBet);
								money -= 2*totalBet;
							} else if (handResolve(playerHand) == TWENTYONE || handResolve(playerHand) == NOT_BUST){
								sprintf(outString, "The dealer's FIVE CARD TRICK beats your %d!\nYou lose $%d.", handValue(playerHand), 2*totalBet);
								money -= 2*totalBet;
							}
							
							//printing screen and outstring
							system("cls");	
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayHand(dealerHand);
							printf("YOUR HAND:\n");							
							displayHand(playerHand);
							printf(outString);							
							enterToContinue();	
							gameState = 6;
							break;	
						}
						default:{ //dealer gets 21 or less, player only wins of blackjack or FCT
							
							char outString[80]; //output string to deplay, depending on player win/loss
							if (handResolve(playerHand) == BLACKJACK){ //player gets blackjack
								sprintf(outString, "Your BLACKJACK beats the dealer's %d!\nYou win $%d!\n", handValue(dealerHand), 2*totalBet);
								money += 2*totalBet;
								currentPlayer.score += 2*totalBet;
							} else if (handResolve(playerHand) == FIVE_CARD_TRICK){
								sprintf(outString, "Your FIVE CARD TRICK beats the dealer's %d!\nYou win $%d!\n", handValue(dealerHand), 2*totalBet);
								money += 2*totalBet;
								currentPlayer.score += 2*totalBet;
							} else if (handValue(playerHand) > handValue(dealerHand)){
								sprintf(outString, "Your %d beats the dealer's %d!\nYou win $%d!\n", handValue(playerHand), handValue(dealerHand), totalBet);
								money += totalBet;
								currentPlayer.score += totalBet;
							} else{
								sprintf(outString, "The dealer's %d beats your %d!\nYou lose $%d.\n", handValue(dealerHand), handValue(playerHand), totalBet);
								money-= totalBet;								
							}
							
							//displaying outstring
							system("cls");	
							printHeader(currentPlayer.name, handNumber, money, initialBet, totalBet, currentPlayer.score);
							printf("DEALER'S HAND:\n");
							displayHand(dealerHand);
							printf("YOUR HAND:\n");		
							displayHand(playerHand);
							printf(outString);
							gameState = 6;			
							enterToContinue();		
							break;	
						}
					}
					break;
					
					case 6: //GAMEOVER or prompting user to play again
						if (money <=0){ //money <0, gameover
						system("cls");
						printArt();
						printf("\n\n");
						printGameOver();
						printf("\n\nYou have $%d. Your score was: %d\n", money, currentPlayer.score);
						enterToContinue();
						
						//saving game to update/save leaderboard
						updateLeaderboard(leaderboard, currentPlayer);
						gameVariables.drawPosition = drawPosition;
						gameVariables.firstBuy = firstBuy;
						gameVariables.gameState = gameState;
						gameVariables.handNumber = handNumber;
						gameVariables.initialBet = initialBet;
						gameVariables.money = money;
						gameVariables.totalBet = totalBet;							
						saveGame(&fIO, leaderboard, currentPlayer, deck, playerHand, dealerHand , gameVariables);
						
						exitGame = 1;
						} else { //promting user to player another round
						
						
						system("cls");
						printHeader(currentPlayer.name, handNumber, money, 0, 0, currentPlayer.score);
						printf("Would you like to play another hand?\n[1] YES\n[2] SAVE and QUIT to title\n");
						
						int playerInput = -1;
						
						while (playerInput == -1){
							playerInput = singleDigitInput('1', '2');
							if (playerInput == -1)
								printf("Correct input only, please\n");
						}
						
						if (playerInput == 1){ //player plays another round
							gameState = 0;
						} else{ //player saves and quits
							updateLeaderboard(leaderboard, currentPlayer);
							gameVariables.drawPosition = drawPosition;
							gameVariables.firstBuy = firstBuy;
							gameVariables.gameState = gameState;
							gameVariables.handNumber = handNumber;
							gameVariables.initialBet = initialBet;
							gameVariables.money = money;
							gameVariables.totalBet = totalBet;							
							saveGame(&fIO, leaderboard, currentPlayer, deck, playerHand, dealerHand , gameVariables);
							
							exitGame = 1;
							break;
						}
						}
						break;
				}
			} //close gamestate switch
		} //end game while
		} //end file check
		else{
			printf("\nERROR: \"save.txt\" not found. Aborting program.\n");
			return(1); //main returns 1 (error)
		}
	} //end title while

	printf("Thanks for playing!");
	return(0);	//main returns 0 (execution okay)
} 

enum handRankingEnum handResolve(struct card *hand){ //scoring hand
	
	//blackjack
	if (handSize(hand) == 2 && handValue(hand)==21)
		return BLACKJACK;
		
	//fivecardtrick
	else if (handSize(hand) == 5 && handValue(hand) <= 21)
		return FIVE_CARD_TRICK;
	
	//twentyone
	else if (handValue(hand) == 21)
		return TWENTYONE;
	
	//not bust
	else if (handValue(hand) <= 21)
		return NOT_BUST;
	
	//bust
	else if (handValue(hand) > 21)
		return BUST;
	
	else return 0; //error enum, shouldn't occur
}

void resetCards(struct card *deck, struct card *playerHand, struct card *dealerHand ){ //shuffles deck, empties hands 
	deckShuffle(deck);	
	
	//setting all cards to null suit/kind
	for	(int i = 0; i < 5; i++){
		playerHand[i].suit = 0;
		playerHand[i].kind = 0;
		dealerHand[i].suit = 0;
		dealerHand[i].kind = 0;
	}
}

int titleMenu(){ //display title menu and resolve player selection
	system("cls");
	printArt();
	printf("Please enter the number corresponding to your selection\n[1] NEW GAME\n[2] LOAD GAME\n[3] LEADERBOARD\n[4] INFO\n[5] QUIT\n");
	
	int userInput = -1;
	while (userInput == -1){
		userInput = singleDigitInput('1','5');
		if (userInput != -1){
			return userInput;
		} else{
			printf("Correct input only, please\n");
		}
	}
}

void enterToContinue(){ //simple enter to continue function
	printf("Press ENTER to continue\n");
	getchar();
}
	
void displayFirstCard(struct card hand[5]) { //displays only first card for dealer
	printf("1) %s of %s\n2) ?\n\n", cardKind(hand[0]), cardSuit(hand[0]));
}

void displayHand(struct card hand[5]){ //displays whole hand
	
	int size = handSize(hand);	
	
	for (int i = 0; i < size; i++){
		printf("%d) %s of %s\n", i+1, cardKind(hand[i]), cardSuit(hand[i]));
	}
	
	int value = handValue(hand);
	printf("Value: %d\n\n", value);
}

int handSize(struct card hand[5]){	 //returns hand size
//counts the number of nullCards to calculate size of hand
	int nullCounter = 0;
	for (int i = 0; i < 5; i++){
		if (hand[i].suit == 0) 
			nullCounter++;
	}
	return (5-nullCounter); 
	
}


//C2: REPEITION
//function to calculate value of a hand
int handValue(struct card hand[5]){
	int value = 0;
	int size = handSize(hand);
	int aceCounter = 0;
	
	
	//iterating through each card in hand, adding 11 if ace, 10 if 10 or picture, otherwise adding value
	for (int i = 0; i < size; i++){
		if(hand[i].kind == ACE){
			value += 11;
			aceCounter ++;
		} else if(hand[i].kind > 9 && hand[i].kind < 14){
			value +=10;		
		} else{
			value += hand[i].kind;
		}
	}
	
	//for each ace in hand, if total hand value > 21, count ace value as 1 (11-10)
	for (int i = 0; i < aceCounter; i++){
		if (value > 21)
			value -= 10;
	}
	return value;
}

//function that stars new game
void newGame(struct card *deck, struct card *playerHand, struct card *dealerHand, struct player *currentPlayer){
	printf("Starting New Game\nPlease enter your name! (max 16 chars, excess will be truncated)\n");
	
	//getting player name
	int valid = 0;
	while (!valid){
		
		fgets(currentPlayer->name, 16, stdin); //reading stsdin
		currentPlayer->name[strcspn(currentPlayer->name, "\n")] = '\0'; // searching for newline character and truncating string
		
		if(strlen(currentPlayer->name) == 0){ //zero length
			printf("Invalid input\n");
		} else if (strchr(currentPlayer->name, ' ')!=NULL){ //includes space
			printf("Invalid input (no spaces)\n");
		} else{
			valid = 1;
		}
	}
	
	//populating/shuffling deck
	deckPopulate(deck);
	deckShuffle(deck);
	
	//generating empty player and dealer hand
	struct card nullCard;
	nullCard.suit = 0;
	nullCard.kind = 0;
	for (int i = 0; i < 5; i++){
		playerHand[i] = nullCard;
		dealerHand[i] = nullCard;
	}
}

//loading just the leaderboard
void loadLeaderboard(FILE **fIO, struct player *leaderboard){
	*fIO = fopen("save.txt", "r"); //opening to read
	char buffer[100];
	
	for (int i = 0; i < 10; i++){ //reading leaderboard
		if(fgets(buffer, sizeof(buffer), *fIO)){
			buffer[strcspn(buffer, "\n")] = '\0';
			strcpy(leaderboard[i].name, buffer);
		}
		if(fgets(buffer, sizeof(buffer), *fIO)){
			char *ptr;
			leaderboard[i].score = strtol(buffer, &ptr, 10); //stringtolong
		}
	}	
	
	struct player nullPlayer;
	nullPlayer.score = -1;
	
	updateLeaderboard(leaderboard, nullPlayer);
	
	fclose(*fIO);
}

//loading entire game 					//C3: input file 
void loadGame(FILE **fIO, struct player *leaderboard, struct player *currentPlayer, struct card *deck, struct card *playerHand, struct card *dealerHand, struct gameVars *loadVars){
	*fIO = fopen("save.txt", "r"); //opening to read
	
	char buffer[100];
	
	for (int i = 0; i < 10; i++){ //reading leaderboard
		if(fgets(buffer, sizeof(buffer), *fIO)){
			buffer[strcspn(buffer, "\n")] = '\0';
			strcpy(leaderboard[i].name, buffer);	//loading into array of struct 
		}
		if(fgets(buffer, sizeof(buffer), *fIO)){
			char *ptr;
			leaderboard[i].score = strtol(buffer, &ptr, 10); //stringtolong
		}
	}
	
	if(fgets(buffer, sizeof(buffer), *fIO)){ //reading name
		buffer[strcspn(buffer, "\n")] = '\0';
		strcpy(currentPlayer->name, buffer);
	}
	
	if(fgets(buffer, sizeof(buffer), *fIO)){ //reading score
		char *ptr;
		currentPlayer->score = strtol(buffer, &ptr, 10); //stringtolong
	}
	
	for(int i = 0; i<52; i++){ //reading deck
		if(fgets(buffer, sizeof(buffer), *fIO)) {
			deck[i].suit = atoi(buffer);
		}
		if(fgets(buffer, sizeof(buffer), *fIO)) {
			deck[i].kind = atoi(buffer);
		}
	}
	
	for (int i = 0; i < 5; i++){ //reading player and dealer hands
		if(fgets(buffer, sizeof(buffer), *fIO)){
			playerHand[i].suit = atoi(buffer); 
		}
		if(fgets(buffer, sizeof(buffer), *fIO)){
			playerHand[i].kind = atoi(buffer); 
		}
		if(fgets(buffer, sizeof(buffer), *fIO)){
			dealerHand[i].suit = atoi(buffer); 
		}
		if(fgets(buffer, sizeof(buffer), *fIO)){
			dealerHand[i].kind = atoi(buffer);
		}
	}
	
	
	//following vars get read into the loadVars struct
	if(fgets(buffer, sizeof(buffer), *fIO)){ 
		char *ptr;
		loadVars->money = strtol(buffer, &ptr, 10); //stringtolong
	}
	if(fgets(buffer, sizeof(buffer), *fIO)){ 
		loadVars->handNumber = atoi(buffer);
	}
	if(fgets(buffer, sizeof(buffer), *fIO)){ //
		loadVars->drawPosition = atoi(buffer);	
	}
	if(fgets(buffer, sizeof(buffer), *fIO)){
		loadVars->initialBet = atoi(buffer);
	}
	if(fgets(buffer, sizeof(buffer), *fIO)){
		loadVars->totalBet = atoi(buffer);
	}
	if(fgets(buffer, sizeof(buffer), *fIO)){
		loadVars->firstBuy = atoi(buffer);
	}
	if(fgets(buffer, sizeof(buffer), *fIO)){
		loadVars->gameState = atoi(buffer);
	}
	
	fclose(*fIO); //closing fIO
}

int validFile(FILE **fIO){ 
	int lineNumber = 1; //to ensure matches with row# in textedit
	
	*fIO = fopen("save.txt", "r"); //open to read
	char ch;
	while(!feof(*fIO)){ //while fIO isn't at end of file
	  ch = fgetc(*fIO); //get char
	  if(ch == '\n'){ //if char is newline, increment
	    lineNumber++;
		}
	}

	fclose(*fIO);
	
	if (lineNumber == 154){ //if there aren't exactly 153 lines, file likely corrupted
		return 1;		
	} else{
		return 0;
	}
	
}

//function to save current game
void saveGame(FILE **fIO, struct player *leaderboard, struct player currentPlayer, struct card *deck, struct card *playerHand, struct card *dealerHand, struct gameVars saveVars){
	*fIO = fopen("save.txt", "w"); //opening to write
	
	for (int i = 0; i < 10; i++){ //writing leaderboard
		fprintf(*fIO, "%s\n%d\n", leaderboard[i].name, leaderboard[i].score);	
	}
	
	fprintf(*fIO, "%s\n%d\n", currentPlayer.name, currentPlayer.score); //writing current name

	for(int i = 0; i<52; i++){ //writing deck
		fprintf(*fIO, "%d\n%d\n", deck[i].suit, deck[i].kind);
	}
	
	for (int i = 0; i < 5; i++){ //writing player and dealer hands
		fprintf(*fIO, "%d\n%d\n", playerHand[i].suit, playerHand[i].kind);
		fprintf(*fIO, "%d\n%d\n", dealerHand[i].suit, dealerHand[i].kind);
	}
	
	//current player's save variables (put into struct before function called)
	fprintf(*fIO, "%d\n", saveVars.money);
	fprintf(*fIO, "%d\n", saveVars.handNumber);
	fprintf(*fIO, "%d\n", saveVars.drawPosition);
	fprintf(*fIO, "%d\n", saveVars.initialBet);
	fprintf(*fIO, "%d\n", saveVars.totalBet);
	fprintf(*fIO, "%d\n", saveVars.firstBuy);
	fprintf(*fIO, "%d\n", saveVars.gameState);
	
	fclose(*fIO);
} 

//resets initializing leaderboard before loading values
void resetLeaderboard(struct player *leaderboard){
	for (int i = 0; i < 10; i++){
		strcpy(leaderboard[i].name, "");
		leaderboard[i].score = 0;
	}
}

//C4: ARRAY OF STRUCT FUNCTION
void updateLeaderboard(struct player *leaderboard, struct player currentPlayer){ //bubblesort: https://en.wikipedia.org/wiki/Bubble_sort
	// sort leaderboard biggest->smallest via bubble sort 
		//compares first element to the next, swapping if smaller
		//does so until first "bubble" reaches end
		//decreases scope of swap by 1
		//repeats process until all elements sorted
		
	for (int i = 0 ; i < 10; i++){
		for (int j = 0 ; j < 9 - i; j++) { 
			if (leaderboard[j].score < leaderboard[j+1].score){
		        swapPlayer(&leaderboard[j], &leaderboard[j+1]); //swap players
      		}
    	}
  	}
	
	if (currentPlayer.score >= leaderboard[9].score){ // if current player score is >= lowest score, replace
		leaderboard[9] = currentPlayer;
		
		for (int i = 0 ; i < 10; i++){
			for (int j = 0 ; j < 9 - i; j++) { 
				if (leaderboard[j].score < leaderboard[j+1].score){
			        swapPlayer(&leaderboard[j], &leaderboard[j+1]); //swap players
	      		}
	    	}
	  	}
	}
}

//C5 function with pointer to var type int
//draws from top of deck and updates deck draw posotion
void topDraw(struct card *hand, struct card *deck, int *position){
	int size = handSize(hand);
	hand[size] = deck[(*position)++];
}

//populates deck with one of each kind of card (not shuffled
void deckPopulate(struct card deck[52]){

	int a, b, k = 0;
	for (a = 1; a < 5; a++){
		for (b = 1; b < 14; b++){
			deck[k].suit = a;
			deck[k].kind = b;
			k++;
		}
	}
	
}

//displays deck, only for debug
void deckDisplay(struct card deck[52]){
	int a, b, k = 0;
	for (a = 1; a < 5; a++){
		for (b = 1; b < 14; b++){
			printf("%d %d %d\n", k, deck[k].suit, deck[k].kind);
			k++;
		}
	}
}

void deckShuffle(struct card *deck){ //function to shuffle deck using fisher yates algorithm
	
	//int i set to size of deck, decreases range of swap on each loop iteration
	for (int i = 51; i > 0; i --){
		int j = rand() % (i + 1); //picking a random element between 0 and i
		//swap (deck, i, j);
		swapCard(&deck[i], &deck[j]);
	}
}

void swapCard(struct card *i,struct card *j){ //swapping two cards via address
	struct card temp = *i;
	*i = *j;
	*j = temp;
}

void swapPlayer(struct player *i, struct player *j){ //swapping two players via address
	struct player temp = *i;
	*i = *j;
	*j = temp;
}

//fuction that returns string from enum
const char* cardKind(struct card cardIn){
	switch (cardIn.kind){
		case ACE:
			return "ACE";
			break;
		case TWO:
			return "TWO";
			break;
		case THREE:
			return "THREE";
			break;
		case FOUR:
			return "FOUR";
			break;
		case FIVE:
			return "FIVE";
			break;
		case SIX:
			return "SIX";
			break;
		case SEVEN:
			return "SEVEN";
			break;
		case EIGHT:
			return "EIGHT";
			break;
		case NINE:
			return "NINE";
			break;
		case TEN:
			return "TEN";
			break;
		case JACK:
			return "JACK";
			break;
		case QUEEN:
			return "QUEEN";
			break;
		case KING:
			return "KING";
			break;
		default:
			return "NULL"; 
			break;
	}
}

//returns string from enum
const char* cardSuit(struct card cardIn){
	switch (cardIn.suit){
		case DIAMONDS:
			return "DIAMONDS";
			break;
		case HEARTS:
			return "HEARTS";
			break;
		case CLUBS:
			return "CLUBS";
			break;
		case SPADES:
			return "SPADES";
			break;
		default:
			return "NULL"; 
			break;
	}
}

//C6: USER INPUT
//checks for singledigitinput only between two numbers
int singleDigitInput(char lowerLimit, char upperLimit){
	
	int limit = 24; // buffer limit
    char text[limit]; //throwaway buffer
    int count = 0; 
    int overflow = 0;
    char t = '\0'; //initializing char
    
    //read an entire string
    while( t != '\n'){
        t = getchar();
        text[count] = t;
        count++;
        if (count >= limit){
            overflow = 1;
            count = 0;  // reset buffer
        }
    }

    //checking exit conditions
    if(overflow) 
		return(-1);            // buffer overflow
        
    if( count != 2 )
		return(-1);        //multiple digits
       
	//checking digit range
    if( text[0] >= (int)lowerLimit && text[0] <= (int)upperLimit )
        return(text[0] - '0');  //returning value
    else
        return(-1); //returning error
	
}

//checks for multidigit input between two numbers
int multiDigitInput(int lowerLimit, int upperLimit){
	int limit = 24; // buffer limit
    char text[limit]; //throwaway buffer
    int count = 0; 
    int overflow = 0;
    char t = '\0';
    int result;
    
    //read an entire string
    while( t != '\n'){
        t = getchar();
        text[count] = t;
        count++;
        if (count >= limit){
            overflow = 1;
            count = 0;  // reset buffer
        }
    }

    //checking for overflow
    if(overflow){
		return(-1);            
	}
	
	//checking all values are ints
	int intCounter = 0;
	for (int i = 0; i < count; i++){
		if (text[i] >= '0' && text[i] <= '9')
			intCounter++;
	}
	
	//number of ints = number of chars counted
	if (intCounter == count-1){
		result = (atoi(text)); //sets result to atoi of text
	} else{
		return -1;
	}
	
	if (result >= lowerLimit && result <= upperLimit){
		return result; //returns result if within limits
	} else{
		return -1;
	}
}

//prints art+score etc
void printHeader(const char *playerName, int handNumber, long money, int initialBet, int totalBet, long score){
	printf("______            _      ______ _            _    _            _      _____           _             \n| ___ \\          ( )     | ___ \\ |          | |  (_)          | |    /  __ \\         (_)            \n| |_/ / ___ _ __ |/ ___  | |_/ / | __ _  ___| | ___  __ _  ___| | __ | /  \\/ __ _ ___ _ _ __   ___  \n| ___ \\/ _ \\ '_ \\  / __| | ___ \\ |/ _` |/ __| |/ / |/ _` |/ __| |/ / | |    / _` / __| | '_ \\ / _ \\ \n| |_/ /  __/ | | | \\__ \\ | |_/ / | (_| | (__|   <| | (_| | (__|   <  | \\__/\\ (_| \\__ \\ | | | | (_) |\n\\____/ \\___|_| |_| |___/ \\____/|_|\\__,_|\\___|_|\\_\\ |\\__,_|\\___|_|\\_\\  \\____/\\__,_|___/_|_| |_|\\___/ \n                                                _/ |\n                                               |__/\n\n");
	printf("PLAYER: %s     ROUND: %d     MONEY: %d     INITIAL BET: $%d     TOTAL BET: $%d     SCORE: %d\n\n", playerName, handNumber, money, initialBet, totalBet, score);
}

void printHighScore(){ //courtesty of https://patorjk.com/software/taag
	printf(" /$$   /$$ /$$$$$$  /$$$$$$  /$$   /$$  /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$  /$$$$$$$$  /$$$$$$\n");
	printf("| $$  | $$|_  $$_/ /$$__  $$| $$  | $$ /$$__  $$ /$$__  $$ /$$__  $$| $$__  $$| $$_____/ /$$__  $$\n");
	printf("| $$  | $$  | $$  | $$  \\__/| $$  | $$| $$  \\__/| $$  \\__/| $$  \\ $$| $$  \\ $$| $$      | $$  \\__/\n");
	printf("| $$$$$$$$  | $$  | $$ /$$$$| $$$$$$$$|  $$$$$$ | $$      | $$  | $$| $$$$$$$/| $$$$$   |  $$$$$$\n");
	printf("| $$__  $$  | $$  | $$|_  $$| $$__  $$ \\____  $$| $$      | $$  | $$| $$__  $$| $$__/    \\____  $$\n");
	printf("| $$  | $$  | $$  | $$  \\ $$| $$  | $$ /$$  \\ $$| $$    $$| $$  | $$| $$  \\ $$| $$       /$$  \\ $$\n");
	printf("| $$  | $$ /$$$$$$|  $$$$$$/| $$  | $$|  $$$$$$/|  $$$$$$/|  $$$$$$/| $$  | $$| $$$$$$$$|  $$$$$$/\n");
	printf("|__/  |__/|______/ \\______/ |__/  |__/ \\______/  \\______/  \\______/ |__/  |__/|________/ \\______/\n\n");
}

void printGameOver(){ //courtesy of https://patorjk.com/software/taag
	printf("  /$$$$$$   /$$$$$$  /$$      /$$ /$$$$$$$$        /$$$$$$  /$$    /$$ /$$$$$$$$ /$$$$$$$\n");
	printf(" /$$__  $$ /$$__  $$| $$$    /$$$| $$_____/       /$$__  $$| $$   | $$| $$_____/| $$__  $$\n");
	printf("| $$  \\__/| $$  \\ $$| $$$$  /$$$$| $$            | $$  \\ $$| $$   | $$| $$      | $$  \\ $$\n");
	printf("| $$ /$$$$| $$$$$$$$| $$ $$/$$ $$| $$$$$         | $$  | $$|  $$ / $$/| $$$$$   | $$$$$$$/\n");
	printf("| $$|_  $$| $$__  $$| $$  $$$| $$| $$__/         | $$  | $$ \\  $$ $$/ | $$__/   | $$__  $$\n");
	printf("| $$  \\ $$| $$  | $$| $$\\  $ | $$| $$            | $$  | $$  \\  $$$/  | $$      | $$  \\ $$\n");
	printf("|  $$$$$$/| $$  | $$| $$ \\/  | $$| $$$$$$$$      |  $$$$$$/   \\  $/   | $$$$$$$$| $$  | $$\n");
	printf(" \\______/ |__/  |__/|__/     |__/|________/       \\______/     \\_/    |________/|__/  |__/\n\n\n");
}

void printArt(){ //courtesty of https://patorjk.com/software/taag
	printf("______            _      ______ _            _    _            _      _____           _             \n| ___ \\          ( )     | ___ \\ |          | |  (_)          | |    /  __ \\         (_)            \n| |_/ / ___ _ __ |/ ___  | |_/ / | __ _  ___| | ___  __ _  ___| | __ | /  \\/ __ _ ___ _ _ __   ___  \n| ___ \\/ _ \\ '_ \\  / __| | ___ \\ |/ _` |/ __| |/ / |/ _` |/ __| |/ / | |    / _` / __| | '_ \\ / _ \\ \n| |_/ /  __/ | | | \\__ \\ | |_/ / | (_| | (__|   <| | (_| | (__|   <  | \\__/\\ (_| \\__ \\ | | | | (_) |\n\\____/ \\___|_| |_| |___/ \\____/|_|\\__,_|\\___|_|\\_\\ |\\__,_|\\___|_|\\_\\  \\____/\\__,_|___/_|_| |_|\\___/ \n                                                _/ |\n                                               |__/\n\n");
}

void displayInfo(){
	system("cls");
	printf("Program: 		blackjackUnwound!\nAuthor:			Benjamin Francis Stanton\nCreated on:		17/05/2021\nLast modified:		20/05/2021\nDescription:		A fully fleshed out game of blackjack vs a computer dealer, with betting,\n.			highscore, and save/load functionality! Player must attempt to build a\n.			winning hand from the cards dealt. Picture cards all have a value of 10,\n.			with the exception of ACE, which can be ONE or ELEVEN.\n.\n.			Hands in order as follows:\n.			BLACKJACK (ace and 10 value card),\n.			FIVE CARD TRICK (hand of five with total value under 21)\n.			TWENTY ONE (exactly 21 from a non ACE-TEN combination)\n.			HIGHCARD (less than 21)\n.			BUST (more than 21)\n.\n.			BLACKJACK and FIVE CARD TRICK are unique in that the winner receives\n.			double the staked bet.\n.\n.			Both player and dealer are dealt one card face up, at which point the\n.			player chooses his initial bet, between 1 and 10.\n.\n.			Player and dealer are then dealt a second card face down. If the dealer\n.			has BLACKJACK this is immediately made clear, and unless player has\n.			BLACKJACK, the dealer wins twice the bet.\n.			\n.			");
	enterToContinue();
	system("cls");
	printf(".			Player then choses from the following:\n.			BUY: draw new card, increasing bet between initial bet and 2x initial bet\n.			TWIST: receive a new card without increasing bet\n.			STICK: receive no further cards, wait for dealer to play\n.\n.			If player goes BUST from drawing new cards, he loses.\n.\n.			Once the player is happy with his hand he sticks, and it is the dealer's\n.			turn. The dealer must continue to draw cards until he either goes BUST or\n.			his hand is equal or greater than 17.\n.\n.			If neither player goes BUST, the higher valued hand wins. If the player\n.			and the dealer both have the same valued hand, the dealer wins the round.\n.\n.			The rules have been adapted from https://www.pagat.com/banking/pontoon.html\n.			\n.			");
	enterToContinue();
}

