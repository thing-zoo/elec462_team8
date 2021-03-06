#include "posts.h"

//write a title
int write_title(struct post *p){ 
	char buf[TITLEBUF];
	int pos = 0;
	int c;	

	while( ( c = getch() ) != EOF ){
		
		if( c == 27 ) return 1;

		if( c == '\n' ) break;

		if( c == KEY_DOWN || c == KEY_RIGHT || c == KEY_UP || c == KEY_LEFT ) continue; //Ignore Arrow Key...

		if(pos == 0 && c == 263){ // backspace(263) handling
			move(0,7);
			continue;
		}
		else if(c == 263){
			printw(" \b");
			pos--;
			continue;
		}
		
		if(pos+1 >= TITLEBUF){	
			printw("\b");
			continue;
		}
		buf[pos++] = c;
	}


	buf[pos] = '\0';
	
	strcpy(p->title,buf);// store title

	move(5,0); // to contents

	return 0;
}

// what time is it now
void get_time(struct post *p){
	time_t t;
	char *str1;

	t = time(NULL);
	
	str1 = ctime(&t);
 	str1[strlen(str1)-1] = '\0';
	strcpy(p->time,str1);
	
	return;
}

//write contents
void write_contents(struct post *p){
	char buf[CONTBUF];
	int pos = 0;
	int c;

	while( ( c = getch() ) != EOF ){
		if( c == 27 ) { // ESC(27) handling to finish contents
			printw(" \b");
			break; 
		}

		if( c == KEY_DOWN || c == KEY_RIGHT || c == KEY_UP || c == KEY_LEFT ) continue; //Ignore Arrow Key...
		
		if( c == '\n' ) { //Enter('\n') handling
			move(5,0);		
			printw("%s\n",buf);
		}

		if((pos == 0 && c == 263) || ( buf[pos-1] == '\n' && c == 263 )){ // backspace(263) handling
			continue;
		}
		else if(c == 263){
			printw(" \b");
			pos--;
			continue;
		}	
		
		if(pos+1 >= CONTBUF){	
			printw("\b");
			continue;
		}

		buf[pos++] = c;
		buf[pos] = '\0';
	}

	if( c == EOF && pos == 0 ) exit(1); //handling...

	

	get_time(p); //what time is it??
	strcpy(p->contents,buf); //store contents
	
	
}

//store post to file
void store_post(struct post p){ 
	int inst = 3;
	if( write(sock,(int *)&inst,sizeof(int)) == -1 ){
		fprintf(stderr, "write to sock error\n");
		exit(1);
	}

	if( write(sock, &p, sizeof(p)) == -1 ){
		fprintf(stderr,"write error\n");
		exit(1);
	}

}

//before posting, print notice
void print_notice(void){

	crmode();
	noecho();
	clear();

	printw("<<<    NOTICE    >>>\n");
	printw("\nTitle must be under 32bytes.\n");
	printw("\nContent must be under 1024bytes.\n");
	printw("\nand you couldn't delete or edit upperline's contents.\n");
	printw("\nwhich means once you press <Enter>, you couldn't go before like old typewriter.\n");
	printw("\nbut you could use backspace to edit contents in the same line.\n");
	
	printw("\n\nPress <ESC> key, and you could save the post.\n");
	printw("\nThanks for posting!!\n");
	printw("\nPress any key to continue..\n");
	
	
	getch();


}

//write a post
void write_post(int year, int month, int day){
	struct post p;	
	int v;

	print_notice();	
	//

	strcpy(p.ID,clnt_ID);
	p.year = year;
	p.month = month;
	p.day = day;
	//
		
	crmode();
	echo();	
	clear();
	//
		
	printw("Title :                                           |\n");	
	printw("--------------------------------------------------|");
	printw("\nID : %-10s                                   |\n",p.ID);
	printw("%4d - %2d - %2d                                    |\n",p.year,p.month,p.day);
	printw("--------------------------------------------------|");
	
	move(0,7);
	refresh();

	v = write_title(&p);
	if( v == 1 ) return;
	write_contents(&p);


	store_post(p);

}


//delete a post
int delete_post(struct post p){

	int inst = 4;
	int result;

	
	write(sock, (int *)&inst, sizeof(int));

	write(sock, &p, sizeof(p));
	write(sock, clnt_ID, sizeof(clnt_ID));
	
	read(sock,(int *)&result,sizeof(int));


	return result;
	
}


//literally refresh postslist
struct post * post_refresh(int *sum, int year, int month, int day){
/*	int fd;
	struct post *plist;
	struct post temp;
	*sum = 0;
*/
	struct post *plist;
	*sum = 0;
	int tsum;
	int inst = 5;
	int i;	

	write(sock,(int*)&inst,sizeof(int));
	
	write(sock,(int *)&year,sizeof(int));
	write(sock,(int *)&month,sizeof(int));
	write(sock,(int *)&day,sizeof(int));
	

	read(sock, (int *)&tsum,sizeof(int));
	
	
	*sum = tsum;

	MALLOC(plist,sizeof(struct post)*tsum);
	for(i = 0; i < tsum; i++){
		read(sock, &plist[i], sizeof(struct post));
	}

	//read(sock, (struct post *)plist, sizeof(struct post)*tsum);


	return plist;

}

//print a  post
void print_post(struct post p){
	

	clear();
	printw("Title : %-32s          |\n",p.title);	
	printw("--------------------------------------------------|");
	printw("\nID : %-10s                                   |\n",p.ID);
	printw("Posted at : %4d - %2d - %2d                        |\n",p.year, p.month, p.day);
	printw("Posted time : %-30s      |\n",p.time);
	printw("--------------------------------------------------|\n");
	printw("%s\n",p.contents);
	printw("--------------------------------------------------\n");
	printw("\nPress any key to continue..\n");
	
	getch();

}

//if there is no posts in selected date..
void empty_day(int year,int month, int day){ 	
	struct post *plist;
	int c;
	int len;

	crmode();
	noecho();
	clear();

	printw("< %d - %d - %d >\n\n",year, month, day );
	printw("There is no posts... Please posting TT\n");
	printw("Press <p> to post\n");
	printw("or Press <ESC> to return\n");
	while(1){

		c = getch();
	
		if( c == 27 ) return;
		if( c == 'p' ){
			write_post(year,month,day);
			plist =	post_refresh(&len,year,month,day);
			print_posts(plist,len,year,month,day);
			return;
		}
		else continue;
	}


	return;
}

//print posts 5-way set
void print_posts_set(struct post *plist, int st, int end,int no){
	int i;
	
	clear();
	printw("< %d - %d - %d >\n\n",plist[0].year, plist[0].month, plist[0].day );
	for(i = st;i < end; i++){
		printw("%d : \"%s\" - %s, written in %s\n\n",i-st,plist[i].title,plist[i].ID,plist[i].time);
	}	
	printw("\n                  <---------   %d   --------->\n",no);
	printw("Press the number of a post you want to see\n");
	printw("Press <p> to post\n");
	printw("Press <d> to delete a post ( You can only delete your own post )\n");
	printw("Press <ESC> to Quit\n");
	
}

//print posts & select what to do
void print_posts(struct post *plist, int len,int year, int month, int day){
	int st = 0;
	int c;
	int end = 5;
	int no = 1;
	int del;

	if( len < end ) end = len;	

	while(1){
		crmode();
		noecho();
		keypad(stdscr, TRUE);
	
		print_posts_set(plist,st,end,no);
		
		c = getch();
		
		if( c == 27 ){// ESC
			if( len > 0 ) free(plist);
			break;
		}
		
		if( c == KEY_RIGHT && end < len ) {  //go next( -> )
			st = end;
			end += 5;
			no++;
		}

		if( c == KEY_LEFT && st > 0 ) { // go previous( <- )
			end = st;
			st -= 5;
			no--;	
		}

		if( c == 'p' ) { // post
			write_post(year, month,day);
			end++;
		}
		
		if( c == 'd' ){ // delete
			del = select_delete(st,end) - 48;
			if( del == -1 ) continue;
			if( delete_post(plist[st+del]) == 1 ){
				printw("You couldn't delete other user's post !!\n");
				printw("Press any key to continue..\n");
				getch();
				continue;
			}
			end++;
		}

		free(plist);
		plist = post_refresh(&len,year,month,day);	

		//select a post
		if( c == '0' ) print_post(plist[st]);
		if( c == '1' && end-st-1 >= 1) print_post(plist[st+1]);
		if( c == '2' && end-st-1 >= 2) print_post(plist[st+2]);
		if( c == '3' && end-st-1 >= 3) print_post(plist[st+3]);
		if( c == '4' && end-st-1 >= 4) print_post(plist[st+4]);	

		//check
		if( end > len ) end = len;
		if( st < 0 ) st = 0;
		if( end - st > 5 ) end--;
		if( end == st && c == 'd' ){
			end = st;
			st -= 5;
			no--;
		}

		if( end == 0 ) {
			if( len > 0 ) free(plist);
			empty_day(year,month,day);
			break;
		}

	}
		

}

//what is the number of a post you wanna delete?
int select_delete(int st, int end){
	int c;
	int i = end-st-1;

	printw("\nWhich number of a post you want to delete?(0 - %d), press <d> again to cancel\n", i);		
	while(1){
	
		c = getch();
		if( c == 'd' ) return 47;
		if( c == '0' ) break;
		if( c == '1' && i >= 1 ) break;
		if( c == '2' && i >= 2 ) break;
		if( c == '3' && i >= 3 ) break;
		if( c == '4' && i >= 4 ) break;
	}
	return c;

}

//show posts in selected date
void posts_list(int year, int month,int day){
	struct post *plist;
	int sum = 0;

	plist = post_refresh(&sum,year,month,day);	
	
	if( sum == 0 ){ // no posts for that month & that day
		empty_day(year,month,day);
		return;
	}
	
	else{
		print_posts(plist, sum,year, month, day);
		return; //go to the calender
	}
}
