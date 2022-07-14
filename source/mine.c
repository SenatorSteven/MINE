/* mine.c

MIT License

Copyright (C) 2022 Stefanos "Steven" Tsakiris

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define true /*----------------------------*/ (1)
#define false /*---------------------------*/ (0)

#define SYSTEM_FAILURE /*------------------*/ (-1)

#define UNEXPLORED_SQUARE /*---------------*/ (' ')
#define MINE_SQUARE /*---------------------*/ ('*')
#define FLAGGED_SQUARE /*------------------*/ ('X')
#define EMPTY_SQUARE /*--------------------*/ ('.')

#define NoneInputError /*------------------*/ (0)
#define XInputError /*---------------------*/ (1)
#define YInputError /*---------------------*/ (2)
#define ActionInputError /*----------------*/ (3)
#define XOutOfBoundsInputError /*----------*/ (4)
#define YOutOfBoundsInputError /*----------*/ (5)
#define ActionOutOfBoundsInputError /*-----*/ (6)
#define SquareAlreadyExploredInputError /*-*/ (7)

#define setInputError(e) /*----------------*/ (inputError = e##InputError)

#define setupArguments(void) /*------------*/ { \
	if(!_setupArguments(argumentAmount, argument)){ \
		fprintf(stderr, "%s: usage: %s [width] [height] [mines]\n", programName, programName); \
		return 1; \
	} \
}

#define setupMemory(void) /*---------------*/ { \
	if(!_setupMemory()){ \
		return 1; \
	} \
}

typedef unsigned int uint;
typedef uint8_t      bool;
typedef uint8_t      InputError;

typedef struct{
	uint x;
	uint y;
} Point;

const char *programName;
uint gameWidth;
uint gameHeight;
uint gameSize;
uint gameMines;
uint widthLength;
uint heightLength;
uint minesLength;
uint frameSize;
char *board;
uint heightMinusOne;
uint widthMinusOne;
char *frame;
uint inputX;
uint inputY;
char *frameReal;
uint frameOffsetTrueWidth;
Point *stack;
Point *stackCurrent;
uint squaresExplored;

static bool _setupArguments(const int argumentAmount, const char *const *argument);
static uint  charToUint(const char *string);
static bool _setupMemory(void);
static void  createBoard(void);
static void  updateBoardSquare(const int offset);
static void  createFrame(void);
static void  gameLoop(void);
static void  printScreen(void);
static void  printBoard(void);
static void  updateFrameSquare(const int xOffset, const int yOffset);
static void  cleanup(void);

int main(const int argumentAmount, const char *const *argument){
	setupArguments();
	setupMemory();
	createBoard();
	createFrame();
	gameLoop();
	cleanup();
	return 0;
}
static bool _setupArguments(const int argumentAmount, const char *const *argument){
	programName = *argument;
	if(argumentAmount != 4)                                    return false;
	if((gameWidth  = charToUint(*(argument + 1))) == (uint)-1) return false;
	if((gameHeight = charToUint(*(argument + 2))) == (uint)-1) return false;
	if((gameMines  = charToUint(*(argument + 3))) == (uint)-1) return false;
	if(gameWidth < 3){
		gameWidth = 3;
	}else if(gameWidth > 99){
		gameWidth = 99;
	}
	if(gameHeight < 3){
		gameHeight = 3;
	}else if(gameHeight > 99){
		gameHeight = 99;
	}
	gameSize = gameWidth * gameHeight;
	if(gameMines < 1){
		gameMines = 1;
	}else if(gameMines >= gameSize){
		gameMines = gameSize - 1;
	}
	if(gameWidth < 10){
		widthLength = 1;
	}else{
		widthLength = 2;
	}
	if(gameHeight < 10){
		heightLength = 1;
	}else{
		heightLength = 2;
	}
	if(gameMines < 10){
		minesLength = 1;
	}else if(gameMines < 100){
		minesLength = 2;
	}else if(gameMines < 1000){
		minesLength = 3;
	}else{
		minesLength = 4;
	}
	frameSize = (2 * gameWidth + 6) * (gameHeight + 2) + 52 + widthLength + heightLength + minesLength;
	return true;
}
static uint charToUint(const char *string){
	uint n = 0;
	while(*string && *string != ' ' && *string != '\t' && *string != '\n'){
		if(*string < '0' || *string > '9'){
			return -1;
		}
		n = n * 10 + *string - 48;
		++string;
	}
	return n;
}
static bool _setupMemory(void){
	if(!(board = malloc(gameSize * sizeof(char)))){
		fprintf(stderr, "%s: could not allocate board\n", programName);
		return false;
	}
	if(!(frame = malloc(frameSize * sizeof(char)))){
		fprintf(stderr, "%s: could not allocate frame\n", programName);
		return false;
	}
	if(!(stack = malloc(gameSize * sizeof(Point)))){
		fprintf(stderr, "%s: could not allocate stack\n", programName);
		return false;
	}
	return true;
}
static void createBoard(void){
	srand(time(NULL));
	widthMinusOne = gameWidth - 1;
	heightMinusOne = gameHeight - 1;
	{
		const char *const wall = board + gameSize;
		do{
			*board = '0';
		}while(++board < wall);
		board -= gameSize;
	}
	{
		uint offset;
		uint currentMine = 0;
		do{
			offset = rand() % gameSize;
			setMineSquare:{
				if(*(board + offset) == MINE_SQUARE){
					if(!offset){
						offset = gameSize;
					}
					--offset;
					goto setMineSquare;
				}
				*(board + offset) = MINE_SQUARE;
			}
		}while(++currentMine < gameMines);
	}
	{
		const int leftSquare       = -1;
		const int rightSquare      = 1;
		const int aboveSquare      = -gameWidth;
		const int belowSquare      = gameWidth;
		const int aboveLeftSquare  = aboveSquare + leftSquare;
		const int aboveRightSquare = aboveSquare + rightSquare;
		const int belowLeftSquare  = belowSquare + leftSquare;
		const int belowRightSquare = belowSquare + rightSquare;
		uint y = 0;
		do{
			uint x = 0;
			do{
				if(*board == MINE_SQUARE){
					if(x > 0){
						updateBoardSquare(leftSquare);
						if(y > 0){
							updateBoardSquare(aboveLeftSquare);
						}
						if(y < heightMinusOne){
							updateBoardSquare(belowLeftSquare);
						}
					}
					if(x < widthMinusOne){
						updateBoardSquare(rightSquare);
						if(y > 0){
							updateBoardSquare(aboveRightSquare);
						}
						if(y < heightMinusOne){
							updateBoardSquare(belowRightSquare);
						}
					}
					if(y > 0){
						updateBoardSquare(aboveSquare);
					}
					if(y < heightMinusOne){
						updateBoardSquare(belowSquare);
					}
				}
				++board;
			}while(++x < gameWidth);
		}while(++y < gameHeight);
		board -= gameSize;
	}
	{
		const char *const wall = board + gameSize;
		do{
			if(*board == '0'){
				*board = EMPTY_SQUARE;
			}
		}while(++board < wall);
		board -= gameSize;
	}
	return;
}
static void updateBoardSquare(const int offset){
	if(*(board + offset) != MINE_SQUARE){
		++*(board + offset);
	}
	return;
}
static void createFrame(void){
	char *f = frame;
	uint x;
	uint y;
	frameOffsetTrueWidth = 2 * gameWidth + 6;
	sprintf(f, "\nMINE - (M)INE (I)s (N)ot (E)ntertaining! (%ux%u - %u)\n\n  o ", gameWidth, gameHeight, gameMines);
	f += 53 + widthLength + heightLength + minesLength;
	x = 0;
	do{
		*(++f) = '-';
		*(++f) = ' ';
	}while(++x < gameWidth);
	*(++f) = 'o';
	*(++f) = '\n';
	frameReal = f + 5;
	y = 0;
	do{
		*(++f) = ' ';
		*(++f) = ' ';
		*(++f) = '|';
		*(++f) = ' ';
		x = 0;
		do{
			*(++f) = UNEXPLORED_SQUARE;
			*(++f) = ' ';
		}while(++x < gameWidth);
		*(++f) = '|';
		*(++f) = '\n';
	}while(++y < gameHeight);
	*(++f) = ' ';
	*(++f) = ' ';
	*(++f) = 'o';
	*(++f) = ' ';
	x = 0;
	do{
		*(++f) = '-';
		*(++f) = ' ';
	}while(++x < gameWidth);
	*(++f) = 'o';
	*(++f) = '\n';
	*(++f) = '\n';
	*(++f) = '\0';
	return;
}
static void gameLoop(void){
	const uint nonMineSquares = gameSize - gameMines;
	InputError inputError;
	char input[5];
	uint action;
	setInputError(None);
	squaresExplored = 0;
	gameLoop:{
		printScreen();
		switch(inputError){
			case XInputError: /*---------------------*/ fprintf(stdout, "X is not a number in [1 - %u]\n", gameWidth); /*--*/ break;
			case YInputError: /*---------------------*/ fprintf(stdout, "Y is not a number in [1 - %u]\n", gameHeight); /*-*/ break;
			case ActionInputError: /*----------------*/ fprintf(stdout, "Action is not a number in [1, 2]\n"); /*----------*/ break;
			case XOutOfBoundsInputError: /*----------*/ fprintf(stdout, "X out of bounds\n\n"); /*-------------------------*/ break;
			case YOutOfBoundsInputError: /*----------*/ fprintf(stdout, "Y out of bounds\n\n"); /*-------------------------*/ break;
			case ActionOutOfBoundsInputError: /*-----*/ fprintf(stdout, "Action out of bounds\n\n"); /*--------------------*/ break;
			case SquareAlreadyExploredInputError: /*-*/ fprintf(stdout, "Square already explored\n\n"); /*-----------------*/ break;
			default: /*------------------------------*/ /*-----------------------------------------------------------------*/ break;
		}
		setInputError(None);
		fprintf(stdout, "x [1 - %u]: ", gameWidth);
		fflush(stdout);
		if(scanf("%4s", input) != 1 || (inputX = charToUint(input)) == (uint)-1){
			setInputError(X);
			goto gameLoop;
		}
		if(--inputX > gameWidth){
			setInputError(XOutOfBounds);
			goto gameLoop;
		}
		fprintf(stdout, "y [1 - %u]: ", gameHeight);
		fflush(stdout);
		if(scanf("%4s", input) != 1 || (inputY = charToUint(input)) == (uint)-1){
			setInputError(Y);
			goto gameLoop;
		}
		if(--inputY > gameHeight){
			setInputError(YOutOfBounds);
			goto gameLoop;
		}
		fprintf(stdout, "action [1, 2]: ");
		fflush(stdout);
		if(scanf("%4s", input) != 1 || (action = charToUint(input)) == (uint)-1){
			setInputError(Action);
			goto gameLoop;
		}
		if(action != 1 && action != 2){
			setInputError(ActionOutOfBounds);
			goto gameLoop;
		}
		{
			char *const f = frameReal + 2 * inputX + inputY * frameOffsetTrueWidth;
			if(*f != UNEXPLORED_SQUARE && *f != FLAGGED_SQUARE){
				setInputError(SquareAlreadyExplored);
				goto gameLoop;
			}
			if(action == 2){
				if(*f == FLAGGED_SQUARE){
					*f = UNEXPLORED_SQUARE;
					goto gameLoop;
				}
				*f = FLAGGED_SQUARE;
				goto gameLoop;
			}
			if(action == 1 && *f == FLAGGED_SQUARE){
				goto gameLoop;
			}
			*f = *(board + inputX + inputY * gameWidth);
			if(*f == MINE_SQUARE){
				printBoard();
				fprintf(stdout, "\nYou lost!\n\n");
				return;
			}
			if(*f != EMPTY_SQUARE){
				goto winRequirementsCheck;
			}
		}
		stackCurrent = stack;
		updateAdjacentSquares:{
			if(inputX > 0){
				updateFrameSquare(-1, 0);
				if(inputY > 0){
					updateFrameSquare(-1, -1);
				}
				if(inputY < heightMinusOne){
					updateFrameSquare(-1, 1);
				}
			}
			if(inputX < widthMinusOne){
				updateFrameSquare(1, 0);
				if(inputY > 0){
					updateFrameSquare(1, -1);
				}
				if(inputY < heightMinusOne){
					updateFrameSquare(1, 1);
				}
			}
			if(inputY > 0){
				updateFrameSquare(0, -1);
			}
			if(inputY < heightMinusOne){
				updateFrameSquare(0, 1);
			}
			if(stackCurrent > stack){
				--stackCurrent;
				inputX = (*stackCurrent).x;
				inputY = (*stackCurrent).y;
				goto updateAdjacentSquares;
			}
		}
		winRequirementsCheck:{
			if(++squaresExplored == nonMineSquares){
				printBoard();
				fprintf(stdout, "\nYou win!!!\n\n");
				return;
			}
			goto gameLoop;
		}
	}
}
static void printScreen(void){
	if(system("clear") == SYSTEM_FAILURE){
		fprintf(stderr, "%s: could not clear screen\n", programName);
	}
	fprintf(stdout, "%s", frame);
	return;
}
static void printBoard(void){
	const char *const fWall = frameReal + gameHeight * (2 * gameWidth + 6);
	const char *bWall = board;
	char *f = frameReal;
	do{
		bWall += gameWidth;
		do{
			*f = *board;
			f += 2;
		}while(++board < bWall);
	}while((f += 6) < fWall);
	board -= gameSize;
	printScreen();
	return;
}
static void updateFrameSquare(const int xOffset, const int yOffset){
	const uint newX = inputX + xOffset;
	const uint newY = inputY + yOffset;
	char *const f = frameReal + 2 * newX + newY * frameOffsetTrueWidth;
	if(*f == UNEXPLORED_SQUARE){
		*f = *(board + newX + newY * gameWidth);
		if(*f == EMPTY_SQUARE){
			(*stackCurrent).x = newX;
			(*stackCurrent).y = newY;
			++stackCurrent;
		}
		++squaresExplored;
	}
	return;
}
static void cleanup(void){
	free(stack);
	free(frame);
	free(board);
	return;
}
