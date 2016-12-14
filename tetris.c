#include <stdio.h>
#include <stdlib.h>
#include <tetris.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PLAYER1 0
#define PLAYER2 1
#define PLAYER3 2
#define PLAYER4 3
#define PLAYER5 4
#define MAXPLAYERS 5

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct tetris_level {
    int score;
    int nsec;
};
struct tetris_block {
    char data[5][5];
    int w;
    int h;
};
/* Holds game state info */
struct tetris {
    char **game;
    int w;
    int h;
    int level;
    int gameover;
    int score;
    struct tetris_block current[MAXPLAYERS];
    int x[MAXPLAYERS];
    int y[MAXPLAYERS];
    int activePlayers;
    bool active[MAXPLAYERS];
};

struct tetris_block blocks0[] =
{
    {{"00", 
      "00"}, //O piece
      2, 2
  },
    {{".0.",
      "000"}, //T piece
      3, 2
  },
    {{"0000"},  //line piece
        4, 1},
    {{"00",
      "0.",  //J piece
      "0."},
      2, 3},
    {{"00",
      ".0",   //L piece
      ".0"},
      2, 3},
    {{"00.",
      ".00"},  //Z piece
      3, 2},
    {{".00",
      "00."},
      3, 2}
  };
  struct tetris_block blocks1[] =
{
    {{"11", 
      "11"}, //O piece
      2, 2
  },
    {{".1.",
      "111"}, //T piece
      3, 2
  },
    {{"1111"},  //line piece
        4, 1},
    {{"11",
      "1.",  //J piece
      "1."},
      2, 3},
    {{"11",
      ".1",   //L piece
      ".1"},
      2, 3},
    {{"11.",
      ".11"},  //Z piece
      3, 2},
    {{".11",
      "11."},
      3, 2}
  };
  struct tetris_block blocks2[] =
{
    {{"22", 
      "22"}, //O piece
      2, 2
  },
    {{".2.",
      "222"}, //T piece
      3, 2
  },
    {{"2222"},  //line piece
        4, 1},
    {{"22",
      "2.",  //J piece
      "2."},
      2, 3},
    {{"22",
      ".2",   //L piece
      ".2"},
      2, 3},
    {{"22.",
      ".22"},  //Z piece
      3, 2},
    {{".22",
      "22."},
      3, 2}
  };
  struct tetris_block blocks3[] =
{
    {{"33", 
      "33"}, //O piece
      2, 2
  },
    {{".3.",
      "333"}, //T piece
      3, 2
  },
    {{"3333"},  //line piece
        4, 1},
    {{"33",
      "3.",  //J piece
      "3."},
      2, 3},
    {{"33",
      ".3",   //L piece
      ".3"},
      2, 3},
    {{"33.",
      ".33"},  //Z piece
      3, 2},
    {{".33",
      "33."},
      3, 2}
  };
  struct tetris_block blocks4[] =
{
    {{"44", 
      "44"}, //O piece
      2, 2
  },
    {{".4.",
      "444"}, //T piece
      3, 2
  },
    {{"4444"},  //line piece
        4, 1},
    {{"44",
      "4.",  //J piece
      "4."},
      2, 3},
    {{"44",
      ".4",   //L piece
      ".4"},
      2, 3},
    {{"44.",
      ".44"},  //Z piece
      3, 2},
    {{".44",
      "44."},
      3, 2}
  };

  struct tetris_level levels[]=
  {
    {0, 1200000},
    {1500, 900000},
    {8000, 700000},
    {20000, 500000},
    {40000, 400000},
    {75000, 300000},
    {100000, 200000}
  };

#define TETRIS_PIECES (sizeof(blocks0)/sizeof(struct tetris_block))
#define TETRIS_LEVELS (sizeof(levels)/sizeof(struct tetris_level))

struct termios save;

// Sets termios attributes immediately
void
tetris_cleanup_io() {
    tcsetattr(fileno(stdin),TCSANOW,&save);
}

void
tetris_signal_quit(int s) {
    tetris_cleanup_io();
}

void
tetris_set_ioconfig() {
    struct termios custom;
    int fd = fileno(stdin);
    tcgetattr(fd, &save);
    custom = save;
    custom.c_lflag &= ~(ICANON|ECHO); // Turn off canonical mode and echo immmediately
    tcsetattr(fd,TCSANOW,&custom);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0)|O_NONBLOCK); //Turn on nonblocking
}
//returns a new tetris struct, takes the player to be removed. player cannot be inactive or >5 <0
struct tetris * tetris_removeplayer (struct tetris * t, int player) {
    if (t->active[player] == false) {
        fprintf(stderr, "Bug. removeplayer used improperly\n");
        exit(1);
    }
    struct tetris * n = malloc(sizeof(struct tetris));
    n->level = t->level;
    n->score = t->score;
    n->gameover = 0;
    n->w = t->w - 5;
    n->h = t->h;
    n->activePlayers = t->activePlayers - 1;
    t->active[player] = false;
    
    
    n->game = malloc(sizeof(char *) * n->w);
    for (int i = 0; i < t->w; i++) {
        if (i < n->w) {
            n->game[i] = t->game[i];
        } else {
            free(t->game[i]);
        }
    }
    free(t->game);
    for (int i = 0; i < MAXPLAYERS; i++) {
        n->active[i] = t->active[i];
        if (t->active[i] == true) {
            if (t->x[i] + t->current[i].w >= n->w) {
                tetris_new_block(n, i);
            } else {
                n->x[i] = t->x[i];
                n->y[i] = t->y[i];
                n->current[i] = t->current[i];
            }
        }
    }
    free(t);
    return n;
}

//returns a new tetris struct, takes the player to be added. player cannot be active or >5 <0
struct tetris * tetris_addplayer (struct tetris *t, int player) {
    if (t->active[player] == true) {
        fprintf(stderr, "Bug. Addplayer used improperly\n");
        exit(1);
    }
    struct tetris * n = malloc(sizeof(struct tetris));
    n->level = t->level;
    n->score = t->score;
    n->gameover = 0;
    n->w = t->w + 5;
    n->h = t->h;
    for (int i = 0; i < MAXPLAYERS; i++) {
        n->active[i] = t->active[i];
        if (t->active[i] == true) {
            n->x[i] = t->x[i];
            n->y[i] = t->y[i];
            n->current[i] = t->current[i];
        }
    }
    n->active[player] = true;

    n->activePlayers = t->activePlayers + 1;
    n->game = malloc(sizeof(char *) * n->w);
    for (int i = 0; i < n->w; i++) {
        if (i < t->w) {
            n->game[i] = t->game[i];
        } else {
            n->game[i] = malloc(sizeof(char) * n->h);
            for (int j = 0; j < n->h; j++) {
                n->game[i][j] = '.';
            }
        }
    }
    free(t->game);
    free(t);
    return n;
}
//Resets the game
void tetris_reset (struct tetris *t) {
    int x, y;
    t->level = 1;
    t->score = 0;
    t->gameover = 0;
    for (x = 0; x < t->w; x++) {
        for (y = 0; y < t->h; y++) {
            t->game[x][y] = '.';
        }
    }
    for (int i = 0; i < MAXPLAYERS; i++) 
    {
        if (t->active[i] == true) {
            tetris_new_block(t, i);
        }
    }

}
void
tetris_init(struct tetris *t, int w, int h) {
    int x, y;
    t->level = 1;
    t->score = 0;
    t->gameover = 0;
    t->w = w;
    t->h = h;
    t->activePlayers = 1;
    for (int i = 0; i < MAXPLAYERS; i++) {
        t->active[i] = false;
    }
    t->active[PLAYER1] = true;
    /* allocates space for 2d array of chars that represent the gameboard*/
    t->game = malloc(sizeof(char *) * w);
    for (x = 0; x < w; x++) {
        t->game[x] = malloc(sizeof(char) * h);
        for (y = 0; y < h; y++)
            t->game[x][y] = '.';
    }
}

/*frees memory associated with tetris struct */
void
tetris_clean(struct tetris *t) {
    int x;
    for (x = 0; x < t->w; x++) {
        free(t->game[x]);
    }
    free(t->game);
    free(t);
}

void 
tetris_print(struct tetris *t) {
    int x, y;
    bool piece;
    for (x = 0; x < 30; x++)
        printf("\n");
    printf("[LEVEL: %d | SCORE: %d]\n", t->level, t->score);
    for (x = 0; x < 2 * t->w + 2; x++)
        printf("~");
    printf("\n");
    for (y = 0; y < t->h; y++) {
        printf ("!");
        for (x = 0; x < t->w; x++) {
            piece = false;
            for (int i = 0; i < MAXPLAYERS; i++) {
                if (t->active[i] == true && x >= t->x[i] && y >= t->y[i] 
                    && x < (t->x[i] + t->current[i].w) && y < (t->y[i] + t->current[i].h) 
                    && t->current[i].data[y - t->y[i]][x - t->x[i]] != '.') {
                    printf("%c ", t->current[i].data[y - t->y[i]][x - t->x[i]]);

                    piece = true;
                }
            }
            if (piece == false) {
                printf("%c ", t->game[x][y]);
            }
        }
    printf ("!\n");
    }
    for (x = 0; x < 2 * t->w + 2; x++)
        printf("~");
    printf("\n");
}
//Returns 1 if the player collides with another player while falling. Else returns 0.
//player must be active
int tetris_collidetest(struct tetris *t, int player) {
    int x, y, X, Y;
    struct tetris_block b = t->current[player];
    for (x = 0; x < b.w; x++) {
        for (y = 0; y < b.h; y++) {
            X = t->x[player] + x;
            Y = t->y[player] + y;
            if (b.data[y][x] != '.') {
                for (int i = 0; i < MAXPLAYERS; i++) {
                    if (i != player && t->active[i] == true) {
                        if (X >= t->x[i] && Y >= t->y[i] && X < (t->x[i] + t->current[i].w) && 
                            Y < (t->y[i] + t->current[i].h) && t->current[i].data[Y - t->y[i]][X - t->x[i]] != '.') {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
//returns 1 if the current piece hits something
int
tetris_hittest(struct tetris *t, int player) {
    int x, y, X, Y;
    struct tetris_block b = t->current[player];
    for (x = 0; x < b.w; x++) {
        for (y = 0; y < b.h; y++) {
            X = t->x[player] + x;
            Y = t->y[player] + y;
            if (X < 0 || X >= t->w) {
                return 1;
            }
            if (b.data[y][x] != '.') {
                if ((Y >= t->h) || 
                    (X >= 0 && X < t->w && Y >= 0 && t->game[X][Y] != '.')) {
                    return 1;
                }
            }      
        }
    }
    return 0;
}

void
tetris_new_block(struct tetris *t, int player) {
    int count = 0;
    if (player == 0) {
        t->current[player] = blocks0[random()%TETRIS_PIECES];
    } else if (player == 1) {
        t->current[player] = blocks1[random()%TETRIS_PIECES];
    } else if (player == 2) {
        t->current[player] = blocks2[random()%TETRIS_PIECES];

    } else if (player == 3) {
        t->current[player] = blocks3[random()%TETRIS_PIECES];

    } else {
        t->current[player] = blocks4[random()%TETRIS_PIECES];

    }
    for (int i = 0; i < MAXPLAYERS; i++) {
        if (t->active[i] == true && i < player) {
                count++; 
            }
    }
    t->x[player] = (count + 1) * (t->w / (t->activePlayers + 1)) + 1;
    t->y[player] = 0;
    if (tetris_hittest(t, player)) {
        t->gameover = 1;
    }
}

//Adds the fallen block to the game array 
void
tetris_print_block(struct tetris *t, int player) {
    int x,y,X,Y;
    struct tetris_block b = t->current[player];
    for (x = 0; x < b.w; x++) {
        for (y = 0; y < b.h; y++) {
            if (b.data[y][x] != '.') {
                t->game[t->x[player] + x][t->y[player] + y] = b.data[y][x];
            }
        }
    }
}

void
tetris_rotate(struct tetris *t, int player) {
    struct tetris_block b = t->current[player];
    struct tetris_block s = b;
    int x,y;
    b.w = s.h;
    b.h = s.w;
    for (x = 0; x < s.w; x++) {
        for (y = 0; y < s.h; y++) {
            b.data[x][y] = s.data[s.h - y - 1][x];
        }
    }
    x = t->x[player];
    y = t->y[player];
    t->x[player] -= (b.w - s.w) / 2;
    t->y[player] -= (b.h - s.h) / 2;
    t->current[player] = b;
    if (tetris_hittest(t, player)) {
        t->current[player] = s;
        t->x[player] = x;
        t->y[player] = y;
    }
}

void
tetris_gravity(struct tetris *t) {
    for (int i = 0; i < MAXPLAYERS; i++) {
        if (t->active[i] == true) {
            t->y[i]++;
            if (tetris_hittest(t, i)) {
                t->y[i]--;
                tetris_print_block(t, i);
                tetris_new_block(t, i);
            }
        }
    }
}
//logic for pieces falling when a line is cleared
void
tetris_fall(struct tetris *t, int l) {
    int x, y;
    for (y = l; y > 0; y--) {
        for (x = 0; x < t->w; x++)
            t->game[x][y] = t->game[x][y-1];
    }
    for (x = 0; x < t->w; x++)
        t->game[x][0] = '.';
}

//Check to clear lines
void
tetris_check_lines(struct tetris *t) {
    int x,y,l;
    int points = 100;
    for (y = t->h - 1; y >= 0; y--) {
        l = 1;
        for (x = 0; x < t->w && l; x++) {
            if (t->game[x][y] == '.') {
                l = 0;
            }
        }
        if (l) {
            t->score += points;
            points *= 2;
            tetris_fall(t, y);
            y++;
        }
    }
}

int
tetris_level(struct tetris *t) {
    int i;
    for (i = 0; i <  TETRIS_LEVELS; i++) {
        if (t->score >= levels[i].score) {
            t->level = i+1;
        } else break;
    }
    return levels[t->level - 1].nsec;
}

//Can take a non-null tetris struct to resume a game
void
tetris_run(int w, int h, struct tetris * n, int portno) {
    struct timespec tm;
    struct tetris * t;
    char cmd;
    int count = 0;    
    srand(time(NULL));

    if (n == NULL) {
        t = malloc(sizeof(struct tetris));
        tetris_set_ioconfig();
        tetris_init(t, w, h);
        tetris_new_block(t, PLAYER1);
    } else {
        t = n;
    }
    tm.tv_sec = 0;
    tm.tv_nsec = 1000000;
    
    // initialization and arg checking
    const int BUF_SIZE = 2048;
    int sockfd, newsockfd, curr_fd, max_fd;
    socklen_t clilen;
    char buffer[BUF_SIZE];
    char read_buffer[1];
    struct sockaddr_in serv_addr, cli_addr;
    int i, j;
    
    // main socket set up
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    // bind and listen for clients
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    if (listen(sockfd,5) < 0)
        error("ERROR on listening");
    clilen = sizeof(cli_addr);
    
    // initialze file desc set for select
    fd_set readfds, tempfds;
    FD_ZERO(&readfds);   
    FD_SET(sockfd, &readfds);
    max_fd = sockfd;
    
    // add stdin to fdset for local host player
    FD_SET(fileno(stdin), &readfds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    
    

    while (!t->gameover) {
        //nanosleep(&tm, NULL);
        count++;
        if (count % 50 == 0) {
            tetris_print(t);
            // fprintf(stderr, "t->x[1] = %d, t->y[1] = %d \n", t->x[1], t->y[1]);
            fprintf(stderr, "w = %d, h = %d \n", t->w, t->h);
            fprintf(stderr, "activePlayers = %d\n", t->activePlayers);
            for (int i = 0; i < MAXPLAYERS; i ++) {
                if (t->active[i] == true) {
                    fprintf(stderr, "Player %d is active\n", i);
                }
            }

        }
        if (count % 350 == 0) {
            tetris_gravity(t);
            tetris_check_lines(t);
        }
        
        //printf("Hey!\n");
        //tetris_print(t);
        
        // -- SELECT on fds ---
        tempfds = readfds; // make a copy of read set into temp set
        if (pselect(max_fd+1, &tempfds, NULL, NULL, &tm, NULL) < 0)
            error("ERROR in select");
        
        // 1) check for new connections, accept and assign newsockfd if so 
        if (FD_ISSET(sockfd, &tempfds)) { 
            
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);            
            if (newsockfd < 0) 
                error("ERROR on accept");
            printf("Incoming connection... assigning socket %d.\n", newsockfd);
            
            // add new socket fd to read set, update max_fd if nesc
            FD_SET(newsockfd, &readfds);
            if (newsockfd > max_fd) 
                max_fd = newsockfd;
            
            // remove server socket from temp set
            FD_CLR(sockfd, &tempfds);
        }
        
        //printf("Got connection before reading\n");
        
        // 2) check all possible active sockets
        for (curr_fd = 0; curr_fd <= max_fd; curr_fd++) {
            
            if (FD_ISSET(curr_fd, &tempfds)) {
                
                // read with read buffer
                bzero(read_buffer, 1);
                j = read(curr_fd, read_buffer, 1);
                
                // handle disconnects
                if (j == 0) {
                    close(curr_fd);
                    FD_CLR(curr_fd, &readfds);
                }
                
                char cmd = read_buffer[0];
                //printf("%c \n", cmd);
                
                
                //while ((cmd = getchar()) > 0) {
                switch (cmd) {
                    case 'a':
                        t->x[PLAYER1]--;   
                        if (tetris_hittest(t, PLAYER1) || tetris_collidetest(t, PLAYER1)) {
                            t->x[PLAYER1]++;
                        }
                        break;
                    case 'd':
                        t->x[PLAYER1]++;
                        if (tetris_hittest(t, PLAYER1) || tetris_collidetest(t, PLAYER1)) {
                            t->x[PLAYER1]--;
                        }
                        break;
                    case 's':
                        t->y[PLAYER1]++;
                        if (tetris_collidetest(t, PLAYER1)) {
                            t->y[PLAYER1]--;
                        } else {
                            if (tetris_hittest(t, PLAYER1)) {
                                t->y[PLAYER1]--;
                                tetris_print_block(t, PLAYER1);
                                tetris_new_block(t, PLAYER1);
                            }
                        }
                        break;
                    case 'w':
                        tetris_rotate(t, PLAYER1);
                        break;
                    case 'p':
                        if (t->activePlayers < 5) {
                            for (int i = 0; i < MAXPLAYERS; i++)
                            {
                                if (t->active[i] == false) {
                                    t = tetris_addplayer(t, i);
                                    tetris_new_block(t, i);
                                    break;
                                }
                            }
                        }
                        break;
                    case 'm':
                        if (t->activePlayers > 1) {
                            t = tetris_removeplayer(t, 0);

                            //Debugging networking not working yet.
                        }
                        break;
                    case 'j':
                        t->x[1]--;
                        if (tetris_hittest(t, PLAYER2) || tetris_collidetest(t, PLAYER2))
                        t->x[1]++;
                        break;
                    case 'l':
                        t->x[1]++;
                        if (tetris_hittest(t, PLAYER2) || tetris_collidetest(t, PLAYER2))
                        t->x[1]--;
                        break;
                    case 'k':
                        t->y[3]++;
                        if (tetris_collidetest(t, PLAYER4)) {
                            t->y[3]--;
                        } else {
                            if (tetris_hittest(t, PLAYER4)) {
                                t->y[3]--;
                                tetris_print_block(t, PLAYER4);
                             tetris_new_block(t, PLAYER4);
                            }
                        }
                        break;
                    case 'i':
                        tetris_rotate(t, PLAYER5);
                        break;
                }
            }
        }
        tm.tv_nsec = tetris_level(t);
    }

    tetris_print(t);
    printf("*** GAME OVER ***\n");
    printf("Play again? (y/n) \n");
    cmd = getchar();
    bool terminate = false;
    while(!terminate) {
        if (cmd == 'y' || cmd == 'Y') {
                tetris_reset(t);
                tetris_run(t->w, t->h, t, portno);
                terminate = true;
        } else if (cmd == 'n' || cmd == 'N') {
                tetris_clean(t);
                tetris_cleanup_io();
                terminate = true;
        } else {
                cmd = getchar();
        }
    }
}
