//========================================================================================================================================================
// Name        : CSC215_Expectiminimax.cpp
// Description : Expectiminimax for a two player game with two dices.
//				 1. Each player is given equal number of chances
//				 2. Game:
//						1. One board (array) of 80 tiles [hardcoded in TILES].
//						2. The board has a set of constraints mentioned in "boardConstraint()" routine.
//						   For example, if player lands on tile "6", the person is taken forward to tile "55".
//						   Similarly, the player is taken backward if he lands on tile "35".
//						3. Two dices are thrown for each round and this is simulated in this program through random number generator with a seed value.
//						4. The players have 6 possible choices - Move Forward/Backward Dice Roll 1 value/Dice Roll 2 value/Dice Roll 1 + Dice Roll 2 value.
//						5. Person who reaches >= 65 tile wins. If a person reaches 65 in last round, computer still has one move and reached 66 for example,
//						   then computer wins.
//						6. If both players are in same tile >= 65, then game is draw
//						7. Heuristic is defined in "evaluate" routine. It is defined as the distance to goal tile from computer's current tile position.
//========================================================================================================================================================

#include <iostream>
using namespace std;
#include <stdlib.h>


#define TILES 80 			//	Max TILES in board
#define WINTILE 65			//	65 or above is a WIN TILE position
#define MIN 1				//	Dice face minimum
#define MAX 6				//	Dice face maximum


int b[TILES] = {0};			// Global Board
int maxdepth = 5;			// Search is 5 Plies maximum
int compsMoveLast = 0;		// Computer Player's last move
int humanMoveLast = 0;		// Human Player's last move

int boardConstraint(int pos);											// Board Constraints

double min(int depth, int diceRoll1, int diceRoll2);					// MIN player - parameters: depth in tree, diceRoll 1 value, diceRoll 2 value
double max(int depth, int diceRoll1, int diceRoll2);					// MAX player - parameters: depth in tree, diceRoll 1 value, diceRoll 2 value
double minChance(int depth);											// Chance node called from MIN node
double maxChance(int depth);											// Chance node called from MAX node

double evaluate();														// Heuristic
double probabilityForMoves(int diceRoll1, int diceRoll2);				// Probability for the Dice Rolls

void printBoard();														// For Output - Display Board State
void printBoardTiles(int i, bool b);									// For Output - Display Board Tiles
void finalTilePositions();												// For Output - Display Final Positions of Players
void printDiceRollValues(string player, int diceRoll1, int diceRoll2);	// For Output - Display Dice Rolls Generated Randomly

void getamove(int diceRoll1, int diceRoll2);							// Get a move from Human Player
void makeamove(int diceRoll1, int diceRoll2);							// Computer Player makes move
void checkGameOver();													// Check if game is over - WIN, LOSE, DRAW?
double check4winner();													// Check if there is a winner

int diceRoll();															// Random Number Generator for Dice Rolls
int diceRollValueChosen(int choice,int diceRoll1,int diceRoll2);		// Dice Roll Value Chosen - Dice 1 or Dice 2 or Dice 1+2

int minLastMove();														// To get the last moves played by human
int maxLastMove();														// To get the last moves played by computer

/*
 * For each turn of game,
 * 1. Get dice roll values generated randomly for player
 * 2. Get a move from Human
 * 3. Make a move for computer from Expectiminimax suggested move
 * 4. Check if game is over
 */
int main() {
	int humanDiceRoll1 = 0, humanDiceRoll2 = 0, compDiceRoll1 = 0, compDiceRoll2 = 0;
	int turn = 1;

	cout << "**************************************************************************************" << endl;
	cout << "\t\t\tRace: Computer Vs Human\t\t" << endl;
	cout << "**************************************************************************************" << endl;
	srand( time( NULL ) );	// Seed value for random number generator

	for(;;){
		cout << endl << "\t\t\t\tTURN " << turn++ << endl << endl;

		humanDiceRoll1 = diceRoll();humanDiceRoll2 = diceRoll();
		printDiceRollValues("Human", humanDiceRoll1, humanDiceRoll2);
		getamove(humanDiceRoll1,humanDiceRoll2);

		compDiceRoll1 = diceRoll();compDiceRoll2 = diceRoll();
		printDiceRollValues("Computer", compDiceRoll1, compDiceRoll2);
		makeamove(compDiceRoll1,compDiceRoll2);

		checkGameOver();
	}
	return 0;
}

/*
 * Get a move from Human and fill the board with 2 for that move.
 */
void getamove(int diceRoll1, int diceRoll2){
	int lastmove = minLastMove(), dice, movetomake = 0, diceValue;
	char moveDir;

	cout << "Enter move backward or forward: b or f - ";
	cin >> moveDir;
	cout << "Enter\n 1 for Dice Roll-1\n 2 for Dice Roll-2\n 3 for sum of Dice Roll's 1 and 2\n";
	cin >> dice;

	for(int i = 0; i < TILES-1; i++){ //undo all previous moves by human
		if(b[i+1] >= 2)
			b[i+1] -= 2;
	}

	switch(moveDir){
		case 'f':
			movetomake = lastmove + diceRollValueChosen(dice,diceRoll1, diceRoll2);
			break;
		case 'b':
			diceValue = diceRollValueChosen(dice,diceRoll1, diceRoll2);
			if(lastmove-diceValue > 0)
				movetomake = lastmove - diceValue;
			break;
	}

	humanMoveLast = boardConstraint(movetomake);
	b[humanMoveLast] += 2; //If human, fill board with 2.
}

/*
 * Computer makes the best move given by Expectiminimax Algorithm
 */
void makeamove(int diceRoll1, int diceRoll2)
{
	double best=-20000.0, score;
	int diceChosen = 0, bestDiceMove = 0, depth=0, mi, bestMove, maxLastmove=0 , compLastMove = maxLastMove();

	b[compLastMove] -= 1; 								//undo last move
	for (int i= 1; i<=2; i++)							//2 possible move directions - forward , backward
	{
		for(int j = 1; j<=3; j++){ 						// 3 possible dice rolls (3*2 moves)
			diceChosen = diceRollValueChosen(j,diceRoll1,diceRoll2);
			switch(i){
				case 1:
					maxLastmove = compLastMove + diceChosen;
					break;
				case 2:
					if(compLastMove - diceChosen > 0)
						maxLastmove = compLastMove - diceChosen;
					break;
			}
			b[boardConstraint(maxLastmove)] += 1; 		// make move on board
			score = maxChance(depth+1); 				//call maxChance to get weighted sum of the score
			if (score == best) {						//Best score equal? Then pick the move that gets the person closest to the goal
				if(boardConstraint(bestMove) < boardConstraint(maxLastmove)){
					mi=i; best=score; bestMove = maxLastmove; bestDiceMove = diceChosen;
				}
			}
			else if(score > best){
				mi=i; best=score; bestMove = maxLastmove; bestDiceMove = diceChosen;
			}
			b[boardConstraint(maxLastmove)] -= 1; 		// undo move
		}
	}
	if(mi == 1)
		cout << "Computer's move is "<< bestMove << " "<< ": Forward Move, "<< "Dice roll/sum chosen was: "<< bestDiceMove <<", Expected score is "<< best << endl;
	else if(mi == 2)
		cout << "Computer's move is " << bestMove << " "<< ": Backward Move, "<< "Dice roll/sum chosen was"<< bestDiceMove <<", Expected score is "<< best << endl;
	b[boardConstraint(bestMove)] += 1;				  // make best move
}

/*
 * Min player : makes move, calculate score by calling minChance , pick minimizing score for computer player.
 */
double min(int depth, int diceRoll1, int diceRoll2)
{
  double best=20000.0, score;
  int diceChosen = 0, minLastmove;

  if (check4winner() != -1.0){												// Winner? Terminate search
	  return (check4winner());
  }
  if (depth == maxdepth) { double eval = evaluate();  return eval;}			// Max Depth - return heuristic value
  for (int i= 1; i<=2; i++)													// 2 possible move directions - forward , backward
  {
	for(int j = 1; j<=3; j++){												// 3 possible dice rolls (3*2 moves)
		diceChosen = diceRollValueChosen(j,diceRoll1,diceRoll2);
		switch(i){
			case 1:
				minLastmove = minLastMove() + diceChosen;
				break;
			case 2:
				if(minLastMove() - diceChosen > 0)
					minLastmove = minLastMove() - diceChosen;
				break;
		}
    	b[boardConstraint(minLastmove)] += 2; 								// make move on board
	    score = minChance(depth+1); 										// call minChance to get weighted sum of the score
	    if (score < best) best=score;										// Best score
        b[boardConstraint(minLastmove)] -= 2; 								// undo move
    }
  }
  return(best);
}

/*
 * Max player : makes move, calculate score by calling maxChance , pick maximizing score for computer player.
 */
double max(int depth, int diceRoll1, int diceRoll2)
{
  double best = -20000.0, score;
  int maxLastmove, diceChosen = 0;

  if (check4winner() != -1.0){												// Winner? Terminate search
	  return (check4winner());
  }
  if (depth == maxdepth)  {double eval = evaluate(); return eval;}			// Max Depth? Return Heuristic value
  for (int i= 1; i<=2; i++)													// 2 possible move directions - forward , backward
  {
	for(int j = 1; j<=3; j++){												// 3 possible dice rolls (3*2 moves)
		diceChosen = diceRollValueChosen(j,diceRoll1,diceRoll2);
		switch(i){
			case 1:
				maxLastmove = maxLastMove() + diceChosen;
				break;
			case 2:
				if(maxLastMove() - diceChosen > 0)
					maxLastmove = maxLastMove() - diceChosen;
				break;
		}
        b[boardConstraint(maxLastmove)] += 1; 								// make move on board
        score = maxChance(depth+1); 										// call maxChance to get weighted sum of the score
        if (score > best) best=score;										// Best score
        b[boardConstraint(maxLastmove)] -= 1;								// undo move
    }
  }
  return(best);
}

/*
 * MinChance : Chance Node, Calculate weighted sum by multiplying probability of each max move and its score.
 */
double minChance(int depth)
{
	double score = 0.0;

	if (check4winner() != -1.0){ 										// if winner? return the score
	  return (check4winner());
	}
	if (depth == maxdepth)  {double eval = evaluate(); return eval;}	// Utility Function
	for (int i= 1; i<= MAX; i++)										// 21 Dice Rolls
	{
		for(int j = i; j <= MAX; j++){
			score += probabilityForMoves(i,j)*max(depth+1,i,j);			// Weighted sum
		}
	}
	return(score);
}

/*
 * MaxChance : Chance Node, Calculate weighted sum by multiplying probability of each min move and its score.
 */
double maxChance(int depth)
{
	double score = 0.0;
	if (check4winner() != -1.0){ 										// if winner? return the score
	  return (check4winner());
	}
	if (depth == maxdepth) {double eval = evaluate(); return eval;} 	// Utility Function
	for (int i= 1; i<= MAX; i++)										// 21 Dice Rolls
	{
		for(int j = i; j <= MAX; j++){
			score += probabilityForMoves(i,j)*min(depth+1,i,j); 		// Weighted Sum of score*probability!
		}
	}
	return(score);
}

/*
 * Check4winner : Return 0 if draw, 81 if win, -66 if lost, -1 if game not over
 */
double check4winner(){
	for(int i=TILES-1; i >=WINTILE; i--){
		if(b[i] == 1){
			return double(TILES+1);
		}
		else if(b[i] == 2){
			return double((-WINTILE)-1);;
		}
		else if(b[i] > 1 && (b[i] %2 != 0)){
			return 0.0;
		}
	}
	return -1.0;
}

/*
 * Evaluate : Heuristic: distance to the goal. closer to the goal, less -ve value. Away from goal, more -ve value.
 */
double evaluate(){
	for(int i=TILES-2; i >=0; i--){
		if( b[i+1] >= 1 && (b[i+1] % 2 != 0) ){
			return double(i-WINTILE+1);
		}
	}
	return double(-WINTILE);
}

/*
 * probabilityForMoves : Dice Rolls (1,1) , (2,2 ).. (6,6) : 1/36, other combinations are 1/18.
 */
double probabilityForMoves(int diceRoll1, int diceRoll2){
	//probability for moves - 11,22,.. 66 : 1/36, others 1/18. Totally 21 distinct moves
	if(diceRoll1 == diceRoll2){
		return (1.0/36.0);
	}
	else
		return (1.0/18.0);
}

/*
 * printBoard : Outputs the Board State
 */
void printBoard(){
	bool comp = false, human = false;
	cout << endl;
	cout << "----------------------------Board State----------------------------\n\n";
	for(int i = TILES-2; i >=0; i--){
		if(comp && human)
			break;
		if(!comp && b[i+1] == 1){					// Computer Player's Current Board Position
			printBoardTiles(i, true);
			cout << "Computer at TILE " << i+1 << endl << endl;
			comp = true;
			compsMoveLast = i+1;
		}
		if(!human && b[i+1] == 2){					// Human Player's Current Board Position
			printBoardTiles(i, false);
			cout << "Human at TILE " << i+1 << endl << endl;
			human = true;
			humanMoveLast = i+1;
		}
		if((!comp || !human) && b[i+1] == 3){		// Computer and Human at same tile
			printBoardTiles(i, true);
			cout << endl;
			printBoardTiles(i, false);
			cout << "Computer and Human at TILE " << i+1 << endl;
			comp = human = true;
			compsMoveLast = humanMoveLast = i+1;
		}
	}
}

/*
 * printBoardTiles : Outputs two tile positions before and after the actual position of computer/human player
 */
void printBoardTiles(int i, bool comp){
	if(i-2 >= 0 ){
		cout << "|\t" << i-2 << "\t|\t" << i-1 << "\t|";
	}
	else if(i-1 >= 0){
		cout << "|\t" << i-1 << "\t|";
	}
	cout << "\t" << i << "\t|";
	comp ? cout << "\tC\t" : cout << "\tH\t";
	cout << "|\t" << i+2 << "\t|\t" << i+3 << "\t|\t";
}

/*
 * checkGameOver : Game Won, Lost, Draw?
 */
void checkGameOver(){
	printBoard();
	if (check4winner() == double(-WINTILE-1)) {
		  cout << endl << "**************" << endl;
		  cout << "Human wins" << endl;
		  finalTilePositions();
		  cout << "**************" << endl;
		  exit(0);
	  }
	  if (check4winner() ==  double(TILES+1)) {
		  cout << endl << "**************" << endl;
		  cout << "Computer Wins"   << endl;
		  finalTilePositions();
		  cout << "**************" << endl;
		  exit(0);
	  }
	  if (check4winner() ==  0.0)    {
		  cout << "**************" << endl;
		  cout << "Game Draw"    << endl;
		  finalTilePositions();
		  cout << "**************" << endl;
		  exit(0);
	  }
}

/*
 * printDiceRollValues : Prints the dice rolls generated for each player
 */
void printDiceRollValues(string player, int diceRoll1, int diceRoll2){
	cout << "************************" << endl;
	cout <<  player << "'s Turn" << endl;
	cout <<  "Dice1 is " << to_string(diceRoll1)<< " and Dice 2 is " << to_string(diceRoll2)  << endl;
	cout << "************************" << endl;
}

/*
 * finalTilePositions : Computer and Human Player positions at end of game
 */
void finalTilePositions(){
	cout << "Final Tile Position: Computer at tile " << maxLastMove() << " , Human at tile " << minLastMove() << endl;
}

/*
 * minLastMove : Last move made by human
 */
int minLastMove(){
	for(int i=TILES-1; i >=0; i--){
		if(b[i] >= 2){
			return i;
		}
	}
	return 0;
}

/*
 * minLastMove : Last move made by computer
 */
int maxLastMove(){
	for(int i=TILES-1; i >=0; i--){
		if(b[i] == 1 || ((b[i] > 1) && b[i]%2 != 0) ){
			return i;
		}
	}
	return 0;
}

/*
 * diceRoll : Simulate random dice roll
 */
int diceRoll(){
	return rand()%(MAX+1-MIN) + MIN;
}

/*
 * boardConstraint : Constraints Defined on Board
 */
int boardConstraint(int pos){
	switch(pos){
		case 6:		return 55;
		case 7:		return 43;
		case 8:		return 35;
		case 9:		return 34;
		case 10:	return 33;
		case 12:	return 10;
		case 17:	return 5;
		case 14:	return 18;
		case 24:	return 34;
		case 35:	return 25;
		case 36:	return 50;
		case 44:	return 43;
		case 58:	return 22;
		case 59:	return 70;
		case 63:	return 40;
		default:	return pos;
	}
}

/*
 * diceRollValueChosen : Returns value of diceRoll 1 or diceRoll 2 or diceRoll 1+2
 */
int diceRollValueChosen(int choice, int diceRoll1, int diceRoll2){
	switch(choice){
		case 1:
			return diceRoll1;
		case 2:
			return diceRoll2;
		case 3:
			return diceRoll1 + diceRoll2;
		default:
			return -1;
		}
}


