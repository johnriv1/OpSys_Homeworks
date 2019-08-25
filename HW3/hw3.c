#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <ctype.h> /* isalnum, isdigit */ 

#include <pthread.h>

typedef struct
{
  char** board;
  int squares_covered;
  /*hold current position on board, where r is row and c is column; initially (0,0)*/
  int current_r;
  int current_c;
  int move_num;
} board_struct;

/*maintains max number of squares covered by sunny; updated everytime a dead end is reached*/
int max_squares = 0;

/*shared array used to maintain a list of "dead end" board configurations*/
board_struct** dead_end_boards;
int dead_end_boards_size = 0;

int m;
int n;
int x = 0;

pthread_t home_thread_id;

/* the mutex variable will be used to guarantee that x is always 45 */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void print_Board(board_struct Board, int m, int n, long int pthread_self)
{
	#ifdef DEBUG_MODE
	printf("Current position is (%d,%d)\n", Board.current_r, Board.current_c);
	printf("%d squares are covered\n", Board.squares_covered);
	#endif
	for (int i = 0; i < m; i++)
	{
		if ((i == 0))
		{
			printf("THREAD %ld: > ", pthread_self);
		}
		else
		{
			printf("THREAD %ld:   ", pthread_self);
		}
		for (int j = 0; j < n; j++)
		{
			printf("%c", Board.board[i][j]);
		}
		printf("\n");
	}
}

void copy_Board(board_struct* old_Board, board_struct* new_Board, int m, int n)
{
	new_Board->board = calloc(m, sizeof(char*));
	for (int i = 0; i < m ; i++)
	{
		new_Board->board[i] = calloc(n, sizeof(char));
	}
	for (int i = 0; i < m ; i++)
	{
		for (int j = 0; j < n; j++)
		{
			new_Board->board[i][j] = old_Board->board[i][j];
		}
	}
	new_Board->squares_covered = old_Board->squares_covered;
	new_Board->current_r = old_Board->current_r;
	new_Board->current_c = old_Board->current_c;
	new_Board->move_num = old_Board->move_num;
}

/*https://stackoverflow.com/questions/29248585/c-checking-command-line-argument-is-integer-or-not*/
int isNumber(char number[])
{
   int i = 0;
	/*
   //checking for negative numbers
   if (number[0] == '-')
       i = 1;
   */    
   for (; number[i] != 0; i++)
   {
   	//if (number[i] > '9' || number[i] < '0')
   	if (!isdigit(number[i]))
   		return 0;
   }
   return 1;
}

/*returns 1 if inputted position is free to move into*/
int move_check(board_struct Board, int move_r, int move_c, int m, int n)
{
	if (move_r >= m || move_r < 0)
		return 0;
	if (move_c >= n || move_c < 0)
		return 0;
	if (Board.board[move_r][move_c] != '.')
		return 0;
	return 1;
}
/*function that returns array of next possible positions*/
/*order of position checking to be determined*/
int possible_move_generator(board_struct Board, int m, int n, int** possible_locations)
{
	(*possible_locations) = calloc(8, sizeof(int));
	int num_moves = 0;
	if (move_check(Board, Board.current_r-1, Board.current_c-2, m, n))
	{	
		(*possible_locations)[num_moves*2] = Board.current_r-1;
		(*possible_locations)[num_moves*2+1] = Board.current_c-2;
		num_moves++;
	}
	if (move_check(Board, Board.current_r-2, Board.current_c-1, m, n))
	{
		(*possible_locations)[num_moves*2] = Board.current_r-2;
		(*possible_locations)[num_moves*2+1] = Board.current_c-1;
		num_moves++;
	}
	if (move_check(Board, Board.current_r-2, Board.current_c+1, m, n))
	{
		(*possible_locations)[num_moves*2] = Board.current_r-2;
		(*possible_locations)[num_moves*2+1] = Board.current_c+1;
		num_moves++;
	}
	if (move_check(Board, Board.current_r-1, Board.current_c+2, m, n))
	{
		(*possible_locations)[num_moves*2] = Board.current_r-1;
		(*possible_locations)[num_moves*2+1] = Board.current_c+2;
		num_moves++;
	}
	if (move_check(Board, Board.current_r+1, Board.current_c+2, m, n))
	{
		(*possible_locations)[num_moves*2] = Board.current_r+1;
		(*possible_locations)[num_moves*2+1] = Board.current_c+2;
		num_moves++;
	}
	
	if (move_check(Board, Board.current_r+2, Board.current_c+1, m, n))
	{
		(*possible_locations)[num_moves*2] = Board.current_r+2;
		(*possible_locations)[num_moves*2+1] = Board.current_c+1;
		num_moves++;
	}
	if (move_check(Board, Board.current_r+2, Board.current_c-1, m, n))
	{
		(*possible_locations)[num_moves*2] = Board.current_r+2;
		(*possible_locations)[num_moves*2+1] = Board.current_c-1;
		num_moves++;
	}
	
	if (move_check(Board, Board.current_r+1, Board.current_c-2, m, n))
	{
		(*possible_locations)[num_moves*2] = Board.current_r+1;
		(*possible_locations)[num_moves*2+1] = Board.current_c-2;
		num_moves++;
	}
	
	#ifdef DEBUG_MODE
	printf("this board has %d possible next moves\n", num_moves);
	printf("possibilities for next position are: ");
	for (int i = 0; i < num_moves; i++)
	{
		printf("(r=%d, c=%d) ", (*possible_locations)[2*i], (*possible_locations)[2*i+1]);
	}
	printf("\n");
	#endif
	
	return num_moves;		
}


/*thread function*/
/*Will take as input a board_struct and the new Sonny position*/
/*Will modify (copy of) board_struct with updated position and coverage*/
/*then will check to see if new movement is possible/ and if there are multiple possible moves*/
/*if one new movement possible, upate board with new movement option and recheck*/
/*if more than one is possible, call thread function with all movement options that are possible*/
/*if none are possible, add board to global variable dead_end_boards;*/
void * Board_Advancement(void * board)
{
	board_struct* initial_board = (board_struct*) board;
	int num_possibilities = 1;
	int *max_squares_covered = malloc(sizeof(int));
	int rc;

	*max_squares_covered = (*initial_board).squares_covered;		
	#ifdef DEBUG_MODE
	printf("THREAD %ld started\n", pthread_self());
	#endif
	/*if only one move possible, keep advancing without splitting up into multiple threads*/
	while(num_possibilities == 1)
	{
		#ifdef DEBUG_MODE
		printf("\nTHREAD %ld started LOOP\n\n", pthread_self());
		#endif
		int* possible_locations = NULL;
		num_possibilities = possible_move_generator(*initial_board, m, n, &possible_locations);
		if (num_possibilities == 0)
		{
			/*board is covered*/
			if (initial_board->squares_covered == n*m)
			{
				printf("THREAD %ld: Sonny found a full knight's tour!\n", pthread_self());
				for (int i = 0; i < m; i++)
				{
					free(initial_board->board[i]);
				}
				free(initial_board->board);
				free(initial_board);
			}
			else
			{
				printf("THREAD %ld: Dead end after move #%d\n", pthread_self(), initial_board->move_num);	
				pthread_mutex_lock( &mutex );
				if (dead_end_boards_size == 0)
				{
					dead_end_boards_size++;
					dead_end_boards = calloc(dead_end_boards_size, sizeof(struct board_struct*));
				}
				else
				{
					dead_end_boards_size++;
					dead_end_boards = realloc(dead_end_boards, dead_end_boards_size*sizeof(struct board_struct*));
					if (dead_end_boards == NULL)
					{
						perror("ERROR: realloc() failed; no memory available\n" );
  						exit(EXIT_FAILURE);
					}
				}
				dead_end_boards[dead_end_boards_size-1] = initial_board;
				//printf("HEREEEE\n");
				//print_Board(*(dead_end_boards[dead_end_boards_size-1]), m, n, pthread_self());
				pthread_mutex_unlock( &mutex );
			}

			if (*max_squares_covered > max_squares)
			{
				pthread_mutex_lock( &mutex );
				max_squares = *max_squares_covered;
				pthread_mutex_unlock( &mutex );
			}			
			//printf("max_squares %d\n", max_squares);				
			pthread_exit( max_squares_covered );
		}
		else if (num_possibilities == 1)
		{
			initial_board->board[possible_locations[0]][possible_locations[1]] = 'S';
			initial_board->current_r = possible_locations[0];
			initial_board->current_c = possible_locations[1];
			initial_board->move_num += 1;
			initial_board->squares_covered += 1;
			*max_squares_covered += 1;
			//printf("4max_squares_covered is now %d\n", *max_squares_covered); 
			#ifdef DEBUG_MODE
			printf("Board on movenum %d is: (with max_squares %d)\n", initial_board->move_num, max_squares);
			print_Board(*initial_board, m, n, pthread_self());
			#endif
		}
		else if (num_possibilities > 1)
		{				
			pthread_t tid[num_possibilities];
			printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", pthread_self(), num_possibilities, initial_board->move_num); 
			for (int i = 0; i < num_possibilities; i++)
			{
				board_struct * new_Board = calloc(1, sizeof(board_struct));
				copy_Board(initial_board, new_Board, m, n);
				
				new_Board->board[possible_locations[2*i]][possible_locations[2*i+1]] = 'S';
				new_Board->current_r = possible_locations[2*i];
				new_Board->current_c = possible_locations[2*i + 1];
				new_Board->squares_covered += 1;
				new_Board->move_num += 1;
				//*max_squares_covered += 1;
				//printf("THREAD %ld: 2max_squares_covered is now %d\n", pthread_self(), *max_squares_covered); 
				/*pthread create*/
				//print_Board(*new_Board, m, n, pthread_self());
				rc = pthread_create( &tid[i], NULL, Board_Advancement, (void*) new_Board);
				if ( rc != 0 )
				{
					fprintf( stderr, "MAIN: Could not create thread (%d)\n", rc );
  					exit(EXIT_FAILURE);
				}
				#ifdef NO_PARALLEL
				int * SquaresCovered_return;
				pthread_join( tid[i], (void **)&SquaresCovered_return);
				printf("THREAD %ld: THREAD [%ld] joined (returned %d)\n", pthread_self(), tid[i], *SquaresCovered_return); 
				//printf("THREAD %ld: 1max_squares_covered is now %d\n", pthread_self(),*max_squares_covered); 
				if (*SquaresCovered_return > *max_squares_covered)
				{
					*max_squares_covered = *SquaresCovered_return;
				}
				free(SquaresCovered_return);
				//printf("THREAD %ld: max is now %d\n", pthread_self() ,*max_squares_covered); 
				#endif
			}
			#ifndef NO_PARALLEL
			for (int i = 0; i < num_possibilities; i ++)
			{
				int * SquaresCovered_return;
				pthread_join( tid[i], (void **)&SquaresCovered_return);
				
				printf("THREAD %ld: THREAD [%ld] joined (returned %d)\n", pthread_self(), tid[i], *SquaresCovered_return); 
				
				if (*SquaresCovered_return > *max_squares_covered)
				{
					*max_squares_covered = *SquaresCovered_return;
					//printf("3max_squares_covered is now %d\n", *max_squares_covered); 
				}
			}
			#endif
			for (int i = 0; i < m; i++)
			{
				free(initial_board->board[i]);
			}
			free(initial_board->board);
			free(initial_board);
			/*exit from function if home thread joined all threads*/
			if (pthread_self() == home_thread_id)
			{
				free(possible_locations);
				free(max_squares_covered);
				return NULL;
			}
			pthread_exit( max_squares_covered);
		}
		free(possible_locations);
	}
	/*It shouldnt reach here*/
	pthread_exit(max_squares_covered);
}

int main( int argc, char** argv )
{
	/*board is size m (rows) x n (columns)*/
	/*main thread will display all "dead end" boards wit at least x squares covered*/
	if ((argc >= 3) && isNumber(argv[1]) && isNumber(argv[2]))
	{
		m = atoi(argv[1]);
		n = atoi(argv[2]);
		if (m <= 2 || n <= 2)
		{
			fprintf(stderr, "ERROR: Invalid argument(s)\n");
			fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		fprintf(stderr, "ERROR: Invalid argument(s)\n");
		fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}
	if (argc == 4)
	{
		if (isNumber(argv[3]))
		{
			x = atoi(argv[3]);
			if (x > (m * n))
			{
				fprintf(stderr, "ERROR: Invalid argument(s)\n");
				fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
				return EXIT_FAILURE;
			}
		}
		else
		{
			fprintf(stderr, "ERROR: Invalid argument(s)\n");
			fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		x = 0;
	}
	if (argc > 4)
	{
		fprintf(stderr, "ERROR: Invalid argument(s)\n");
		fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}
	#ifdef DEBUG_MODE
	printf("Input is m = %d, n = %d, x = %d\n", m, n, x);
	#endif
	//pthread_t tid[ MAXTHREADS ];
	
	printf("THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n", pthread_self(), m, n);
	
	home_thread_id = pthread_self();
	
	board_struct * initial_board = calloc(1, sizeof(board_struct));
	initial_board->squares_covered = 1;
	initial_board->current_r = 0;
	initial_board->current_c = 0;
	initial_board->move_num = 1;
	initial_board->board = calloc(m, sizeof(char*));
	for (int i = 0; i < m ; i++)
	{
		initial_board->board[i] = calloc(n, sizeof(char));
	}
	for (int i = 0; i < m ; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if ((i==0) && (j==0))
			{
				initial_board->board[i][j] = 'S';
			}
			else
			{
				initial_board->board[i][j] = '.';
			}
		}
	}
	
	Board_Advancement(initial_board);
	printf("THREAD %ld: Best solution(s) found visit %d squares (out of %d)\n",pthread_self(), max_squares, m*n);

	printf("THREAD %ld: Dead end boards:\n", pthread_self());
	for (int i = 0; i < dead_end_boards_size; i++)
	{
		if (dead_end_boards[i]->squares_covered >= x)
		{
			print_Board(*(dead_end_boards[i]), m, n, pthread_self());
		}
	}
	for (int i = 0; i < dead_end_boards_size; i++)
	{
		for (int j = 0; j < m ; j++)
		{
			free(dead_end_boards[i]->board[j]);
		}
		free(dead_end_boards[i]->board);
		free(dead_end_boards[i]);
	}
	free(dead_end_boards);
}











