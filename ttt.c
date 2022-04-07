#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

int randomSeed  = 0;

// stuff for semaphores and shared memory
int bsUseSemUndo = 0;
int bsRetryOnEintr = 1;
int initSemAvailable(int semId, int semNum);
int initSemInUse(int semId, int semNum);
int reserveSem(int semId, int semNum);
int releaseSem(int semId, int semNum);
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *_buf;
	};
typedef struct __sharedmem {
	int turn;
	int arr1[3][3];
	}sharedmem;
typedef struct __rec_t {
    unsigned int playernum;
    int winner;
    int player;
    int move;
} rec_t;

// Functions given to us
int initSemAvailable(int semId, int semNum)
{
union semun arg;
arg.val = 1;
return semctl(semId, semNum, SETVAL, arg);
}
int initSemInUse(int semId, int semNum)
{
union semun arg;
arg.val = 0;
return semctl(semId, semNum, SETVAL, arg);
}
int reserveSem(int semId, int semNum)
{
struct sembuf sops;
sops.sem_num = semNum;
sops.sem_op = -1;
sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;
while (semop(semId, &sops, 1) == -1)
{
if (errno != EINTR || !bsRetryOnEintr)
return -1;
}
return 0;
}
int releaseSem(int semId, int semNum)
{
struct sembuf sops;
sops.sem_num = semNum;
sops.sem_op = 1;
sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;
return semop(semId, &sops, 1);
}

// pointers to structures
struct __sharedmem *s;
rec_t r;

void usage() // function for mistakes in program calls
{
    fprintf(stderr, "usage: ./ttt -1 for player 1 or ./ttt -2 for player 2\n");
    exit(1);
}


// place either an x or an o based on what player it currently is
void placeHere(int x, int y) 
{
if(r.player == 1) // if it's player 1, place an X
{
	s->arr1[x][y] = 1;
}
if(r.player == -1) // if it's player 2, place an O
{
	s->arr1[x][y] = -1;
}
}

// if there are no 2 in a rows, place in an open spot
void move()
{
if(r.player == 1)
{
	if(r.move > 1)	
	{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			if(s->arr1[i][j] == 0)
			{
				placeHere(i, j);
				r.move++;
			}
		}
	}
	}
	if(r.move == 0)
	{
		placeHere(1,1);
		r.move++;
	}
}
if(r.player == 2)
{
	if(r.move > 1)	
	{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			if(s->arr1[i][j] == 0)
			{
				placeHere(i, j);
				r.move++;
			}
		}
	}
	}
	if(r.move == 1)
	{
		s->arr1[0][1] == 1;
		r.move++;
		
	}

}
}

// First see if current player has 2 in a row and win if so. If current player doesn't, then check if other player does and block. if neither player does, then place in open spot.
void pointCnt()
{
int sum, test = 0;
for(int i = 0; i < 2; i++)
{
if(r.player == 1)
{
	test = 1;
}
if(r.player == -1)
{
	test = -1;
}
// Check by column
for(int i = 0; i < 3; i++)
{
	if(s->arr1[i][0] == test) // if there is an x/o here, increment sum
	{
		sum++;
	}
	if(sum == 3) // if the sum is 3, x/o has won.
	{
		r.winner = r.player;
	}
}
if(sum == 2) // if sum = 2, then find empty spot in this row and place there
{
	for(int i = 0; i < 3; i++)
	{
		if(s->arr1[i][0] != test)
		{
			placeHere(i, 0);
			test = 2;
			r.player = 2;
			sum = 0;
		}
	}
}
sum = 0;
for(int i = 0; i < 3; i++)
{
	if(s->arr1[i][1] == test)
	{
		sum++;
	}
	if(sum == 3)
	{
		r.winner = test;
	}
}
if(sum == 2)
{
	for(int i = 0; i < 3; i++)
	{
		if(s->arr1[i][1] != test)
		{
			placeHere(i, 1);
			test = 2;
			r.player = 2;
			sum = 0;
		}
	}
}
sum = 0;
for(int i = 0; i < 3; i++)
{
	if(s->arr1[i][2] == test)
	{
		sum++;
	}
	if(sum == 3)
	{
		r.winner = test;
	}
}
if(sum == 2)
{
	for(int i = 0; i < 3; i++)
	{
		if(s->arr1[i][2] != test)
		{
			placeHere(i, 2);
			test = 2;
			r.player = 2;
			sum = 0;
		}
	}
}
sum = 0;

// Check by row
for(int i = 0; i < 3; i++)
{
	if(s->arr1[0][i] == test)
	{
		sum++;
	}
	if(sum == 3)
	{
		r.winner = test;
	}
}
if(sum == 2)
{
	for(int i = 0; i < 3; i++)
	{
		if(s->arr1[0][i] != test)
		{
			placeHere(0, i);
			test = 2;
			r.player = 2;
			sum = 0;
		}
	}
}
sum = 0;

for(int i = 0; i < 3; i++)
{
	if(s->arr1[1][i] == test)
	{
		sum++;
	}
	if(sum == 3)
	{
		r.winner = test;
	}
}
sum = 0;
if(sum == 2)
{
	for(int i = 0; i < 3; i++)
	{
		if(s->arr1[1][i] == 0)
		{
			placeHere(1, i);
			test = 2;
			r.player = 2;
			sum = 0;
		}
	}
}
for(int i = 0; i < 3; i++)
{
	if(s->arr1[2][i] == test)
	{
		sum++;
	}
	if(sum == 3)
	{
		r.winner = test;
	}
}
if(sum == 2)
{
	for(int i = 0; i < 3; i++)
	{
		if(s->arr1[2][i] == 0)
		{
			placeHere(2, i);
			test = 2;
			r.player = 2;
			sum = 0;
		}
	}
}
sum = 0;

	
// Check for diagonal
if(s->arr1[0][2] == test)
{
sum++;
}
if(s->arr1[1][1] != test && s->arr1[1][1] != 0 && sum == 1)
{
	sum = 0;
}
if(s->arr1[1][1] == test)
{
sum++;
}
if(s->arr1[2][0] == test)
{
sum++;				
}
if(s->arr1[0][2] != test && s->arr1[0][2] != 0 && sum == 1)
{
	sum = 0;
}
if(sum == 3)
{
		r.winner = test;
}
if(sum == 2)
{
	if(s->arr1[0][2] == 0)
	{
		placeHere(0,2);
		test = 2;
		r.player = 2;
		sum = 0;
	}
	if(s->arr1[1][1] == 0)
	{
		placeHere(1,1);
		test = 2;
		r.player = 2;
		sum = 0;
	}
	if(s->arr1[2][0] == 0)
	{
		placeHere(2,0);
		test = 2;
		r.player = 2;
		sum = 0;
	}
}
sum = 0;

// other diagonal
if(s->arr1[0][0] == test)
{
sum++;
}
if(s->arr1[1][1] != test && s->arr1[1][1] != 0 && sum == 1)
{
	sum = 0;
}
if(s->arr1[1][1] == test)
{
sum++;
}
if(s->arr1[2][2] == test)
{
sum++;				
}
if(sum == 3)
{
		r.winner = test;
}
if(sum == 2)
{
	if(s->arr1[0][0] == 0)
	{
		placeHere(0,0);
		test = 2;
		r.player = 2;
		sum = 0;
	}
	if(s->arr1[1][1] == 0)
	{
		placeHere(1,1);
		test = 2;
		r.player = 2;
		sum = 0;
	}
	if(s->arr1[2][2] == 0)
	{
		placeHere(2,2);
		test = 2;
		r.player = 2;
		sum = 0;
	}
}
r.player = r.player * -1;
}
if(test != 2)
{
	move();
}
}


char setMark(int x, int y)
{
//if 1, set to X
//if 0, set to NULL
//if -1, set to O
if(s->arr1[x][y] == 0)
{
	printf("       ");
}
		
if(s->arr1[x][y] == 1)
{
	printf("   X   ");
}		
		
if(s->arr1[x][y] == -1)
{
	printf("   O   ");
}
}


// Board taken from here: http://www.cprogrammingnotes.com/question/tic-tac-toe-game.html
void display()
{

	printf("\n");
  	printf("       |       |     \n");		
    	setMark(0,0);
    	printf("|");
    	setMark(0,1);
    	printf("|");
    	setMark(0,2);
    	printf("\n");

    	printf("  _____|_______|_____\n");
    	printf("       |       |     \n");

    	setMark(1,0);
    	printf("|");
    	setMark(1,1);
    	printf("|");
    	setMark(1,2);
    	printf("\n");

    	printf("  _____|_______|_____\n");
    	printf("       |       |     \n");

    	setMark(2,0);
    	printf("|");
    	setMark(2,1);
    	printf("|");
    	setMark(2,2);
    	printf("\n");

    	printf("       |       |     \n\n"); 
}


// player 1
void player1()
{
r.player = 1;
int fd, rand1, rand2;
// seed random number generator
srand(randomSeed);
  char * myfifo = "/tmp/xoSync";
  if (mkfifo(myfifo, S_IRWXU) == -1) 		// STEP 1(PLAYER 1): Attempt to create the FIFO xoSync. If the fifo already exists or is created, continue. 
  {
   if (errno != EEXIST)
   {
    perror("mkfifo");
    exit(EXIT_FAILURE);
    }
  } 
  
  mkfifo(myfifo, 0666);
  rand1 = rand() % (unsigned int) 0xFFFFFFFF;		// STEP 2(PLAYER 1): Generate two random numbers
  rand2 = rand() % (unsigned int) 0xFFFFFFFF;		// STEP 2(PLAYER 1): Generate two random numbers
  key_t key1 = ftok(myfifo, rand1);	 		// STEP 3(PLAYER 1): create 1st key using fifo and 1st random key value. use the first key generated as the projection value for shared memory
  key_t key2 = ftok(myfifo, rand2); 			// STEP 3(PLAYER 1): create 2nd key using fifo and 2nd random key value. use the first key generated as the projection value for shared memory
  int shmid = shmget(key1,sizeof(struct __sharedmem),0666|IPC_CREAT);	// STEP 4(PLAYER 1): create shared memory and make sure to give it proper access permissions.
  int semid = semget(key2,2,0666|IPC_CREAT); 		// STEP 5(PLAYER 1): Create a semaphore with a set size of 2
  initSemAvailable(semid, 0); 			// STEP 6(PLAYER 1): player 1. Use setval to initialize player 1 as available
  initSemInUse(semid, 1);  				// STEP 6(PLAYER 1): player 2. Use setval to initialize player 2 as in use
  s = shmat(shmid,NULL, 0); 				// STEP 7(PLAYER 1): Attach the segment of shmem to process and initialize it. The initial state of each cell is empty (0) & turn is (0)
  s->arr1[0][0] = 0;
  s->arr1[0][1] = 0;
  s->arr1[0][2] = 0;
  s->arr1[1][0] = 0;
  s->arr1[1][1] = 0;
  s->arr1[1][2] = 0;
  s->arr1[2][0] = 0;
  s->arr1[2][1] = 0;
  s->arr1[2][2] = 0;
  fd = open(myfifo, O_WRONLY); 			// STEP 8(PLAYER 1): open the fifo for write
  write(fd,&rand1,sizeof(rand1)); 			// STEP 9(PLAYER 1): write the random numbers you generated to the fifo. write them in order of generation.
  write(fd,&rand2,sizeof(rand2));
  close(fd);   					// STEP 10(PLAYER 1): close the FIFO
  while(s->turn > -1){					// STEP 11(PLAYER 1): Enter the gameplay loop
  	reserveSem(semid, 0);				// STEP 1(PLAYER 1): Reserve player 1's semaphore
	display();					// STEP 2(PLAYER 1): Display the state of the game board
   	pointCnt();					// STEP 3(PLAYER 1): Make player 1's move
	display();					// STEP 4(PLAYER 1): Display the state of the game board
	if(r.winner == 1 || r.move == 9)		// STEP 5(PLAYER 1): If player 1 has won or no more plays exist, set the turn counter to -1
	{	
		printf("game over. player 1 wins.");	// STEP 5(PLAYER 1): If player 1 has won or no more plays exist, set the turn counter to -1
		s->turn = -1;				// STEP 5(PLAYER 1): If player 1 has won or no more plays exist, set the turn counter to -1
	}						// STEP 5(PLAYER 1): If player 1 has won or no more plays exist, set the turn counter to -1
	releaseSem(semid, 1);	  	   		// STEP 6(PLAYER 1): Release player 2's semaphore
  }							// END OF GAMEPLAY LOOP
  fd = open(myfifo, O_WRONLY);			// STEP 12(PLAYER 1): Open the FIFO xoSync for write
  close(fd);						// STEP 13(PLAYER 1): Close the FIFO
  shmdt(s);						// STEP 14(PLAYER 1): Detach the segment of shared memory
  shmctl(shmid, 0, IPC_RMID);				// STEP 15(PLAYER 1): Delete the shared memory and semaphores
  semctl(semid, 0, IPC_RMID);				// STEP 15(PLAYER 1): Delete the shared memory and semaphores
 
}

void player2()
{
r.player = 2;
int rand1, rand2, fd;
  char * myfifo = "/tmp/xoSync";
  if (mkfifo(myfifo, S_IRWXU) == -1) 		// STEP 1(PLAYER 2): Attempt to create the FIFO xoSync. If the fifo already exists or is created, continue. 
  {
   if (errno != EEXIST)
   {
    perror("mkfifo");
    exit(EXIT_FAILURE);
    }
  }
  
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_RDONLY); 			// STEP 2(PLAYER 2): open the fifo for read 
  read(fd,&rand1, sizeof(rand1));			// STEP 3(PLAYER 2): read two integers from the fifo
  read(fd,&rand2, sizeof(rand2));
  close(fd);						// STEP 4(PLAYER 2): close the FIFO
  key_t key1 = ftok(myfifo, rand1);	 		// STEP 5(PLAYER 1): create 1st key using fifo and 1st random key value. This is for shared memory
  key_t key2 = ftok(myfifo, rand2); 			// STEP 5(PLAYER 1): create 2nd key using fifo and 2nd random key value. This is for semaphors	
  int shmid = shmget(key1,1024,0666|IPC_CREAT);	// STEP 6(PLAYER 2): Retrieve the shared memory  and the semaphore set created by player 1.
  int semid = semget(key2,2,0666|IPC_CREAT); 		// STEP 6(PLAYER 2): Retrieve the shared memory  and the semaphore set created by player 1.
  shmat(shmid,NULL, 0); 				// STEP 7(PLAYER 2): Attach the shared memory segment
  while(1)
  {					        	// START OF GAMEPLAY LOOP. While true loop
  	reserveSem(semid, 0);				// STEP 1(PLAYER 2): Reserve player 1's semaphore
	display();					// STEP 2(PLAYER 2): Display the state of the game board
	if(s->turn == -1)				// STEP 2(PLAYER 2): if turn counter is -1 exit the loop
	{
		break;
	}
		pointCnt();				// STEP 4(PLAYER 2): Make player 2's move
		display();				// STEP 5(PLAYER 2): Display the state of the game board
		s->turn++;				// STEP 6(PLAYER 2): Increment the game turn by 1
		releaseSem(semid, 0);			// Step 7(PLAYER 2): Release player 1's semaphore
  }							// END OF GAMEPLAY LOOP
  fd = open(myfifo, O_RDONLY); 			// STEP 9(PLAYER 2): Open the FIFO xoSync for read
   close(fd);						// STEP 10(PLAYER 2): Close the FIFO
   shmdt(s);						// STEP 11(PLAYER 2): Detach the segment of shared memory
}


int
main(int argc, char *argv[])
{
// input params
int playernum = 0;
int c;
opterr = 0;
while ((c = getopt(argc, argv, "if:12")) != -1) {
	switch (c) {
	case '1': // if player one is chosen
        	r.playernum = 1;
    	break;
	case '2': // if player two is chosen
        	printf("two player chosen");
        	r.playernum = 2;
    	break;
	default:
    	usage();
	}
	}
    

if(r.playernum == 1){
player1();
}

if(r.playernum == 2){
player2();
}


return 0;
}
