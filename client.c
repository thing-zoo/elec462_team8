#define _CRT_SECURE_NO_WARNINGS
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
int sock;

#include "posts.c"
#include "cal.c"

//init
#define TICKS_TO_COOLDOWN 0
int inputCoolCount = 0;

int key = 0;
void update();
//

//처음 옵셥
#define QUIT 0
#define MAIN_MENU 1
#define SIGNIN 2
#define SIGNUP 3
#define DIARY 4
//


//화면 만들기..
#define MAX_CHOICE_CHARS 30 // max number of chars a menu option can have
#define TOP_BANNER_PAD 2
#define BANNER_OPTIONS_PAD 4
//

//Server&Client
#define MAXBUF 1024
#define MAXCLIENT 100//max client num
//

//첫 화면에 옵션 선택부분
#define MAX_CHOICE_CHARS 30 // max number of chars a menu option can have
#define TOP_BANNER_PAD 2
#define BANNER_OPTIONS_PAD 4

typedef struct
{
	int stateID;
	char text[MAX_CHOICE_CHARS];
} menuChoice;

menuChoice *mc;

int numChoices, currentNumChoices = 0;
int titleColumns[3];//banner 0 signin 1 signup2
int titleRows[3]; // number of colums and rows in the title banner
int cursorPos; // current row cursor is on (vertical cursor position)
int topOptPos, botOptPos; // position of the first menu choice, last menu choice

char *titleText_banner; // char pointer to hold text of the tile
char *titleText_signin;
char *titleText_signup;
//

typedef struct LoginData{
	int Clientfd;
	char ID[10];
	char PASSWORD[MAXBUF];
}LoginData;//클라이언트의 ID와 PASSWORD를 저장하는 구조체
LoginData logindata;

//Screen
#define MAX_FPS 50
#define MSEC_IN_SEC 1000
#define DELAY (MSEC_IN_SEC / MAX_FPS)
#define TICS_PER_SEC MAX_FPS
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
int colors[] = {0,1,2,3,4,5,6,7};

//state
int state = 1;
int prevState =0;
int substate = 0;
//

char titleBanner[] = {"\
0000  00000    0    0000  0    0\
0   0   0     0 0   0   0  0  0 \
0   0   0    0   0  0 00    00  \
0   0   0    00000  0   0   00  \
0000  00000 0     0 0   0   00  "};

char titleSignup[] = {"\
00000 00000  000   00   0 0    0 0000 \
0       0   0      0 0  0 0    0 0   0\
00000   0   0  0   0  0 0 0    0 0000 \
    0   0   0   0  0   00 0    0 0    \
00000 00000  0000  0   00  0000  0    "};


char titleSignin[] = {"\
00000 00000  000   00   0 00000 00   0\
0       0   0      0 0  0   0   0 0  0\
00000   0   0  0   0  0 0   0   0  0 0\
    0   0   0   0  0   00   0   0   00\
00000 00000  0000  0   00 00000 0   00"};

WINDOW *w;


//main.c
void initialize();
void render();
void gamePause();
void cleanup();
int getKey();
bool notReadyToQuit();
void update();
int tick = 0;



//오류 다루기
void error_handling(char *message);

//처음 옵셥 선택할 때 사용되는 함수들
int getState();
int getPrevState();
void setState(int newState);

//첫 화면에 옵션 선택부분 구현
void initMenu(int numberOfChoices);


bool initTitle_banner(int columns, int rows, char *text);
bool initTitle_signin(int columns, int rows, char *text);
bool initTitle_signup(int columns, int rows, char *text);
int initMenuChoice(int id, char choiceText[]);

// called by update(), passes key pressed from keyboard (if any)
void updateMainMenu(int key);
void updateSignin(int key);
void updateSignup(int key);
// called by render()
void renderMainMenu();
void renderSignin();
void renderSignup();

// call before quitting, frees up allocated memory
void cleanupMenu();

int main(int argc, char* argv[])
{
	struct sockaddr_in serv_addr;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));


	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
		error_handling("connect() error!");


	initialize();

	while(notReadyToQuit())
	{
		update();
		render();
		gamePause();

		tick++;
		if(tick == TICS_PER_SEC)
		{
			tick = 0;
		}
	}

	cleanup();
    close(sock);

	return 0;

}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void initialize()
{
	w = initscr(); // init a curses window

	start_color(); // call so can use colors
	int i, j, k = 0; // counter variables
	for(i = 0; i < 8; i++) // initialize all 64 color pairs
	{
		for(j = 0; j < 8; j++)
		{
			if(k) // skip cp #1; already defined as black and white
			{
				init_pair(k, colors[i], colors[j]);
			}
			k++;
		}
	}

	
	cbreak(); // put terminal in c break mode
	nodelay(w, TRUE); // don't wait for enter to read char

	noecho(); // turn off key echoing
	keypad(w, TRUE); // allow getch() to detect non-character key presses

	curs_set(0); // hide the cursor

	initMenu(3);
	
	initTitle_banner(32, 5, titleBanner);
	initTitle_signin(38, 5, titleSignin);
    initTitle_signup(38, 5, titleSignup);
	initMenuChoice(SIGNIN, "Sign in");
	initMenuChoice(SIGNUP, "Sign up");
	initMenuChoice(QUIT, "Quit");

	//initGame();

	setState(MAIN_MENU);
}


void render() 
{
	clear();
	switch(getState())
	{
		case MAIN_MENU:
			renderMainMenu();
			break;
		case SIGNIN:
		    renderSignin();
		 	break;
		case SIGNUP:
		 	renderSignup();
		 	break;

		default:
			break;
	}
	refresh(); // call curses's refresh funct to update screen
}

bool notReadyToQuit()
{
	if(state)
	{
		return TRUE;
	} else 
	{
		return FALSE;
	}
}



//update.c

void update() {

	key = getKey();
	if(key != ERR){
		if(inputCoolCount == 0)
		{
			inputCoolCount = TICKS_TO_COOLDOWN;
		} 
		else {
			key = ERR;
		}

		switch(getState())
		{
			case MAIN_MENU:
				updateMainMenu(key);
				break;
			case SIGNIN:
			 	updateSignin(key);
			 	break;
            case SIGNUP:
			  	updateSignup(key);
			 	break;	     	
			default:
			 	break;
		}
	} 

	if(inputCoolCount > 0)
	{
			inputCoolCount--;
	}
}

int getKey(){
	int key = getch();
	return key;
}


void gamePause() 
{
	napms(DELAY); // from curses.h, sleeps for n milliseconds
}

void cleanup()
{
	curs_set(1); // set cursor back to being visible
	cleanupMenu();
	//cleanupGame();
	clear();
	refresh();
	delwin(w);
    	endwin();
}

int getState()
{
	return state;
}

int getPrevState()
{
	return prevState;
}

void setState(int newState)
{
	prevState = state;
	state = newState;
}

void initMenu(int numberOfChoices)
{
	// allocate mem for menu choices
	mc = (menuChoice*) malloc(numberOfChoices * sizeof(menuChoice));
	numChoices = numberOfChoices;
}

bool initTitle_banner(int columns, int rows, char *text)
{
	// make sure entire menu will actually fit
	if((columns > SCREEN_WIDTH)||(rows > (SCREEN_HEIGHT - 1 - numChoices)))
	{
		return FALSE;
	}
	titleColumns[0] = columns;
	titleRows[0] = rows;

	// allocate memory for title banner's text
	titleText_banner = malloc(rows * columns + 1);


	// copy text into title text
	int i;
	for(i = 0; i < rows * columns; i++)
	{
		titleText_banner[i] = text[i];
	}

	cursorPos = TOP_BANNER_PAD + titleRows[0] + BANNER_OPTIONS_PAD + 1;
	topOptPos = cursorPos;
	botOptPos = cursorPos + numChoices - 1;
	return TRUE;
}

bool initTitle_signin(int columns, int rows, char *text)
{
	// make sure entire menu will actually fit
	if((columns > SCREEN_WIDTH)||(rows > (SCREEN_HEIGHT - 1 - numChoices)))
	{
		return FALSE;
	}
	titleColumns[1] = columns;
	titleRows[1] = rows;

	// allocate memory for title banner's text
	titleText_signin = malloc(rows * columns + 1);


	// copy text into title text
	int i;
	for(i = 0; i < rows * columns; i++)
	{
		titleText_signin[i] = text[i];
	}

	return TRUE;
}

bool initTitle_signup(int columns, int rows, char *text)
{
	// make sure entire menu will actually fit
	if((columns > SCREEN_WIDTH)||(rows > (SCREEN_HEIGHT - 1 - numChoices)))
	{
		return FALSE;
	}
	titleColumns[2] = columns;
	titleRows[2] = rows;

	// allocate memory for title banner's text
	titleText_signup = malloc(rows * columns + 1);


	// copy text into title text
	int i;
	for(i = 0; i < rows * columns; i++)
	{
		titleText_signup[i] = text[i];
	}

	
	return TRUE;
}

int initMenuChoice(int id, char choiceText[])
{
	// make sure it's not going to initialize more options than memory 
	// was allocated for choices
	if(currentNumChoices >= numChoices)
	{
		return -1;
	}

	mc[currentNumChoices].stateID = id;
	strcpy(mc[currentNumChoices].text, choiceText);

	return ++currentNumChoices;
}

void updateMainMenu(int key)
{
	// process keyboard input
	switch(key)
	{
		case KEY_UP:
			if(cursorPos > topOptPos)
			{
				cursorPos--; // move cursor up one
			}
			break;
		case KEY_DOWN:
			if(cursorPos < botOptPos)
			{
				cursorPos++; // move cursor down one
			}
			break;
		case '\n':
		case 'z':
			// if enter pressed, set the state of the choice that
			// was selected
			setState(mc[cursorPos - topOptPos].stateID);
			break;
		case 27:
			setState(QUIT);
			break;
		default:
			break;
	}
}

void renderMainMenu()
{
	// perform setup for printing title banner
	int leftEdgePos = (int) (80 - titleColumns[0])/2;
	int x = leftEdgePos;
	int y = TOP_BANNER_PAD;
	int i;

	// print out title banner
	for(i = 0; i < titleColumns[0] * titleRows[0]; i++)
	{
		mvaddch(y, x, titleText_banner[i]);
		if((x - leftEdgePos) >= (titleColumns[0]-1))
		{
			x = leftEdgePos;
			y++;
		} else x++;
	}

	// move cursor to correct row to print out choices
	y = TOP_BANNER_PAD + titleRows[0] + BANNER_OPTIONS_PAD;

	// print out choices
	for(i = 0; i < currentNumChoices; i++)
	{
		y++;
		leftEdgePos = (int) (80 - strlen(mc[i].text))/2;
		// add cursors to indicate slected choice
		if(y == cursorPos)
		{
			mvaddstr(y, leftEdgePos - 3, "-> ");
			mvaddstr(y, leftEdgePos + strlen(mc[i].text), " <-");
		}
		mvaddstr(y, leftEdgePos, mc[i].text);
	}
	mvprintw(22,0, "Use arrow keys to select an option"); 
	mvprintw(23,0, "Select an option with (Enter)"); 
	mvprintw(24,0, "Select \"Quit\" or press (ESC) to quit"); 
}

void cleanupMenu()
{
	free(mc);
	free(titleText_banner);
}

void updateSignin(int key)
{
// process keyboard input
	switch(key)
	{
		case KEY_UP:
			if(cursorPos > topOptPos)
			{
				cursorPos--; // move cursor up one
			}
			break;
		case KEY_DOWN:
			if(cursorPos < botOptPos)
			{
				cursorPos++; // move cursor down one
			}
			break;
		case '\n':
		case 'z':
			// if enter pressed, set the state of the choice that
			// was selected
			setState(mc[cursorPos - topOptPos].stateID);
			break;
		case 27:
			setState(QUIT);
			break;
		default:
			break;
	}
}

void updateSignup(int key)
{
// process keyboard input
	switch(key)
	{
		case KEY_UP:
			if(cursorPos > topOptPos)
			{
				cursorPos--; // move cursor up one
			}
			break;
		case KEY_DOWN:
			if(cursorPos < botOptPos)
			{
				cursorPos++; // move cursor down one
			}
			break;
		case '\n':
		case 'z':
			// if enter pressed, set the state of the choice that
			// was selected
			setState(mc[cursorPos - topOptPos].stateID);
			break;
		case 27:
			setState(QUIT);
			break;
		default:
			break;
	}
}

void renderSignin()
{
	// perform setup for printing title banner
	int leftEdgePos = (int) (80 - titleColumns[1])/2;
	int x = leftEdgePos;
	int y = TOP_BANNER_PAD;
    char check[MAXBUF]={0,};
	int i;

	memset(&logindata,0,sizeof(LoginData));

	// print out title banners
	for(i = 0; i < titleColumns[1] * titleRows[1]; i++)
	{
		mvaddch(y, x, titleText_signin[i]);
		if((x - leftEdgePos) >= (titleColumns[1]-1))
		{
			x = leftEdgePos;
			y++;
		} else x++;
	}

	// move cursor to correct row to print out choices
	y = TOP_BANNER_PAD + titleRows[1] + BANNER_OPTIONS_PAD;

	mvprintw(y,leftEdgePos, "Input your ID/PASSWORD");
    echo();
    crmode();
    nocbreak();
    nodelay(stdscr,FALSE);
    y++;
    char buf[MAXBUF];
	int pos = 0;
	int c;	

    mvprintw(y+1,leftEdgePos, "ID :");
    mvprintw(y+2,leftEdgePos, "PASSWORD :");

	move(y+1,leftEdgePos+5); // to contents
	while( ( c = getch() ) != EOF ){
		
		if( c == 27 ) return;

		if( c == '\n' ) break;

		if( c == KEY_DOWN || c == KEY_RIGHT || c == KEY_UP || c == KEY_LEFT ) continue; //Ignore Arrow Key...

		if(pos == 0 && c == 263){ // backspace(263) handling
			move(y+1,leftEdgePos+5); // to contents
			continue;
		}
		else if(c == 263){
			printw(" \b");
			pos--;
			continue;
		}
		
		if(pos+1 >= MAXBUF){	
			printw("\b");
			continue;
		}
     
		buf[pos++] = c;
	}


	buf[pos] = '\0';
	
	strcpy(logindata.ID,buf);// store title

    move(y+2,leftEdgePos+12); // to contents
    char buf2[MAXBUF];
	pos = 0;	
    noecho();
    crmode();
	while( ( c = getch() ) != EOF ){
		
		if( c == 27 ) return;

		if( c == '\n' ) break;

		if( c == KEY_DOWN || c == KEY_RIGHT || c == KEY_UP || c == KEY_LEFT ) continue; //Ignore Arrow Key...

		if(pos == 0 && c == 263){ // backspace(263) handling
			move(0,17);
			continue;
		}
		else if(c == 263){
			printw(" \b");
			pos--;
			continue;
		}
		
		if(pos+1 >= MAXBUF){	
			printw("\b");
			continue;
		}
        printw("*");
		buf2[pos++] = c;
	}


	buf2[pos] = '\0';
	
	strcpy(logindata.PASSWORD,buf2);// store title

	//printf("ID: %s\n",input.ID);
	//printf("PASSWORD: %s\n",input.PASSWORD);

    char signin[MAXBUF] = "signin";

	if(write(sock,signin,sizeof(signin))<0)
    {
		error_handling("write error : ");
	}

	if(write(sock,logindata.ID,sizeof(logindata.ID))<0)
    {
		error_handling("write error : ");
	}

	if(write(sock,logindata.PASSWORD,sizeof(logindata.PASSWORD))<0)
    {
		error_handling("write error : ");
	}

	read(sock,check,1);
	if(strcmp(check,"O")==0)
	{
		strcpy(clnt_ID, logindata.ID);
		mvprintw(y+4,leftEdgePos, "Welcome :D");
		cal(sock);
	}
	else
	{
		mvprintw(y+4,leftEdgePos, "Who are you?");
	}
	
	

	return;

 
}

void renderSignup()
{
	// perform setup for printing title banner
	int leftEdgePos = (int) (80 - titleColumns[2])/2;
	int x = leftEdgePos;
	int y = TOP_BANNER_PAD;
    char check[MAXBUF]={0,};
	int i;

	memset(&logindata,0,sizeof(LoginData));

	// print out title banners
	for(i = 0; i < titleColumns[2] * titleRows[2]; i++)
	{
		mvaddch(y, x, titleText_signup[i]);
		if((x - leftEdgePos) >= (titleColumns[2]-1))
		{
			x = leftEdgePos;
			y++;
		} else x++;
	}

	// move cursor to correct row to print out choices
	y = TOP_BANNER_PAD + titleRows[2] + BANNER_OPTIONS_PAD;

	mvprintw(y,leftEdgePos, "Create your ID/PASSWORD");
    echo();
    crmode();
    nocbreak();
    nodelay(stdscr,FALSE);
    y++;
    char buf[MAXBUF];
	int pos = 0;
	int c;
    char signup[MAXBUF] = "signup";	

    mvprintw(y+1,leftEdgePos, "ID :");
    mvprintw(y+2,leftEdgePos, "PASSWORD :");

	move(y+1,leftEdgePos+5); // to contents
	while( ( c = getch() ) != EOF ){
		
		if( c == 27 ) return;

		if( c == '\n' ) break;

		if( c == KEY_DOWN || c == KEY_RIGHT || c == KEY_UP || c == KEY_LEFT ) continue; //Ignore Arrow Key...

		if(pos == 0 && c == 263){ // backspace(263) handling
			move(y+1,leftEdgePos+5); // to contents
			continue;
		}
		else if(c == 263){
			printw(" \b");
			pos--;
			continue;
		}
		
		if(pos+1 >= MAXBUF){	
			printw("\b");
			continue;
		}
     
		buf[pos++] = c;
	}


	buf[pos] = '\0';
	
	strcpy(logindata.ID,buf);// store title

    move(y+2,leftEdgePos+12); // to contents
    char buf2[MAXBUF];
	pos = 0;	
    noecho();
    crmode();
	while( ( c = getch() ) != EOF ){
		
		if( c == 27 ) return;

		if( c == '\n' ) break;

		if( c == KEY_DOWN || c == KEY_RIGHT || c == KEY_UP || c == KEY_LEFT ) continue; //Ignore Arrow Key...

		if(pos == 0 && c == 263){ // backspace(263) handling
			move(0,17);
			continue;
		}
		else if(c == 263){
			printw(" \b");
			pos--;
			continue;
		}
		
		if(pos+1 >= MAXBUF){	
			printw("\b");
			continue;
		}
        printw("*");
		buf2[pos++] = c;
	}


	buf2[pos] = '\0';
	
	strcpy(logindata.PASSWORD,buf2);// store title

	//printf("ID: %s\n",input.ID);
	//printf("PASSWORD: %s\n",input.PASSWORD);
    
	if(write(sock,signup,sizeof(signup))<0)
    {
		error_handling("write error : ");
	}

	if(write(sock,logindata.ID,sizeof(logindata.ID))<0)
    {
		error_handling("write error : ");
	}

	if(write(sock,logindata.PASSWORD,sizeof(logindata.PASSWORD))<0)
    {
		error_handling("write error : ");
	}

	read(sock,check,1);

	if(strcmp(check,"O")==0)
	{
		mvprintw(y+4,leftEdgePos, "success!");
	}
	else
	{
		mvprintw(y+4,leftEdgePos, "fail!");
	}

	return;
}


