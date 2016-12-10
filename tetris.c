#include <stdio.h>
#include <stdlib.h>
#include <tetris.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

#define PLAYER1 0
#define PLAYER2 1
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
    struct tetris_block current;
    struct tetris_block current2;
    int x;
    int y;
    int x2;
    int y2;
};

struct tetris_block blocks[] =
{
    {{"OO", 
      "OO"}, //O piece
    2, 2
    },
    {{".T.",
      "TTT"}, //T piece
    3, 2
    },
    {{"IIII"},  //line piece
        4, 1},
    {{"JJ",
      "J.",  //J piece
      "J."},
    2, 3},
    {{"LL",
      ".L",   //L piece
      ".L"},
    2, 3},
    {{"ZZ.",
      ".ZZ"},  //Z piece
    3, 2},
    {{".SS",
      "SS."},
    3, 2}
};

struct tetris_level levels[]=
{
    {0,
        1200000},
    {1500,
        900000},
    {8000,
        700000},
    {20000,
        500000},
    {40000,
        400000},
    {75000,
        300000},
    {100000,
        200000}
};

#define TETRIS_PIECES (sizeof(blocks)/sizeof(struct tetris_block))
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

void
tetris_init(struct tetris *t,int w,int h) {
    int x, y;
    t->level = 1;
    t->score = 0;
    t->gameover = 0;
    t->w = w;
    t->h = h;
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
    for (x = 0; x<t->w; x++) {
        free(t->game[x]);
    }
    free(t->game);
}

void
tetris_print(struct tetris *t) {
    int x, y;
    for (x = 0; x < 30; x++)
        printf("\n");
    printf("[LEVEL: %d | SCORE: %d]\n", t->level, t->score);
    for (x = 0; x < 2 * t->w + 2; x++)
        printf("~");
    printf("\n");
    for (y = 0; y < t->h; y++) {
        printf ("!");
        for (x = 0; x < t->w; x++) {  //Checks and prints chars for pieces
            if (x >= t->x && y >= t->y 
                    && x < (t->x + t->current.w) && y < (t->y + t->current.h) 
                    && t->current.data[y - t->y][x - t->x] != '.') {
                printf("%c ", t->current.data[y - t->y][x - t->x]);
            }
            else if (x >= t->x2 && y >= t->y2 
                     && x < (t->x2 + t->current2.w) && y < (t->y2 + t->current2.h) 
                    && t->current2.data[y - t->y2][x - t->x2] != '.') {
                printf("%c ", t->current2.data[y - t->y2][x - t->x2]);
            }
            else {
                printf("%c ", t->game[x][y]);
            }
        }
        printf ("!\n");
    }
    for (x = 0; x < 2 * t->w + 2; x++)
        printf("~");
    printf("\n");
}
int tetris_collidetest(struct tetris *t, int player) {
    int x, y, X, Y;
    if (player == PLAYER1) {
        struct tetris_block b = t->current;
        struct tetris_block c = t->current2;
        for (x = 0; x < b.w; x++) {
            for (y = 0; y < b.h; y++) {
                X = t->x + x;
                Y = t->y + y;
                if (b.data[y][x] != '.') {
                    if (X >= t->x2 && Y >= t->y2 && X < (t->x2 + t->current2.w) && 
                        Y < (t->y2 + t->current2.h) && t->current2.data[Y - t->y2][X - t->x2] != '.') {
                        // fprintf(stderr, "X = %d, Y = %d \n", X, Y);
                        // fprintf(stderr, "current2.w = %d, current2.h = %d \n", t->current2.w, t->current2.h);
                        // fprintf(stderr, "x = %d, y = %d \n", x, y);
                        // fprintf(stderr, "current2.data[y - t->y2][x - t->x2] = '%c' \n", t->current2.data[y - t->y2][x - t->x2]);


                        // fprintf(stderr, "x2 = %d, y2 = %d\n", t->x2, t->y2);
                        return 1;
                    }
                }
            }
        }
    } else {
        struct tetris_block b = t->current2;       
        for (x = 0; x < b.w; x++) {
            for (y = 0; y < b.h; y++) {
                X = t->x2 + x;
                Y = t->y2 + y;
                if (b.data[y][x] != '.') {
                    if (X >= t->x && Y >= t->y && X < (t->x + t->current.w) && 
                        Y < (t->y + t->current.h) && t->current.data[Y - t->y][X - t->x] != '.') {
                        return 1;
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
    if (player == PLAYER1) {
        struct tetris_block b = t->current;
        struct tetris_block c = t->current2;
        for (x = 0; x < b.w; x++) {
            for (y = 0; y < b.h; y++) {
                X = t->x + x;
                Y = t->y + y;
                if (X < 0 || X >= t->w) {
                    return 1;
                }
                if (b.data[y][x] != '.') {
                    if ((Y >= t->h) || 
                            (X >= 0 && X < t->w && Y >= 0 && t->game[X][Y] != '.')) {
                        return 1;
                    }
                    // if (X >= t->x2 && Y >= t->y2 && X < (t->x2 + t->current2.w) && 
                    //     Y < (t->y2 + t->current2.h) && t->current2.data[Y - t->y2][X - t->x2] != '.') {
                        
                    //     return 1;
                    // }
                }      
            }
        }
    } else {
        struct tetris_block b = t->current2;       
        for (x = 0; x < b.w; x++) {
            for (y = 0; y < b.h; y++) {
                X = t->x2 + x;
                Y = t->y2 + y;
                if (X < 0 || X >= t->w) {
                    return 1;
                }
                if (b.data[y][x] != '.') {
                    if ((Y >= t->h) || 
                            (X >= 0 && X < t->w && Y >= 0 && t->game[X][Y] != '.')) {
                        return 1;
                    }
                    // if (X >= t->x && Y >= t->y && X < (t->x + t->current.w) && 
                    //     Y < (t->y + t->current.h) && t->current.data[Y - t->y][X - t->x] != '.') {
                    //     return 1;
                    // }
                }
                
            }
        }
    }
    return 0;
}
void
tetris_new_block(struct tetris *t, int player) {
    if (player == PLAYER1) {

        t->current = blocks[random()%TETRIS_PIECES];
        t->x = (t->w / 2) - (t->current.w / 2) - 3;
        t->y = 0;
        if (tetris_hittest(t, PLAYER1)) {
            t->gameover = 1;
        }
    } else if (player == PLAYER2) {
        t->current2 = blocks[random()%TETRIS_PIECES];

        t->x2 = (t->w / 2) + (t->current.w / 2) + 3;
        t->y2 = 0;
        if (tetris_hittest(t, PLAYER2)) {
            t->gameover = 1;
        }
    }
}


//Adds the fallen block to the game array 
void
tetris_print_block(struct tetris *t, int player) {
    int x,y,X,Y;
    if (player == PLAYER1) {
        struct tetris_block b = t->current;
        for (x = 0; x < b.w; x++)
            for (y = 0; y < b.h; y++) {
                if (b.data[y][x] != '.')
                    t->game[t->x + x][t->y + y] = b.data[y][x];
            }
    } else {
        struct tetris_block b = t->current2;
        for (x = 0; x < b.w; x++)
            for (y = 0; y < b.h; y++) {
                if (b.data[y][x] != '.')
                    t->game[t->x2 + x][t->y2 + y] = b.data[y][x];
            }
    }
}

void
tetris_rotate(struct tetris *t, int player) {
    if (player == PLAYER1) {
        struct tetris_block b = t->current;
        struct tetris_block s = b;
        int x,y;
        b.w = s.h;
        b.h = s.w;
        for (x = 0; x < s.w; x++)
            for (y = 0; y < s.h; y++) {
                b.data[x][y] = s.data[s.h - y - 1][x];
            }
        x = t->x;
        y = t->y;
        t->x -= (b.w - s.w) / 2;
        t->y -= (b.h - s.h) / 2;
        t->current = b;
        if (tetris_hittest(t, PLAYER1)) {
            t->current = s;
            t->x = x;
            t->y = y;
        }
    } else {
        struct tetris_block b = t->current2;
        struct tetris_block s = b;
        int x,y;
        b.w = s.h;
        b.h = s.w;
        for (x = 0; x < s.w; x++)
            for (y = 0; y < s.h; y++) {
                b.data[x][y] = s.data[s.h - y - 1][x];
            }
        x = t->x2;
        y = t->y2;
        t->x2 -= (b.w - s.w) / 2;
        t->y2 -= (b.h - s.h) / 2;
        t->current2 = b;
        if (tetris_hittest(t, PLAYER2)) {
            t->current = s;
            t->x2 = x;
            t->y2 = y;
        }
    }
    
}

void
tetris_gravity(struct tetris *t) {
    int x, y;
    t->y++;
    if (tetris_hittest(t, PLAYER1)) {
        t->y--;
        tetris_print_block(t, PLAYER1);
        tetris_new_block(t, PLAYER1);
    }
    t->y2++;
    if (tetris_hittest(t, PLAYER2)) {
        t->y2--;
        tetris_print_block(t, PLAYER2);
        tetris_new_block(t, PLAYER2);
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

void
tetris_run(int w, int h) {
    struct timespec tm;
    struct tetris t;
    char cmd;
    int count = 0;
    tetris_set_ioconfig();
    tetris_init(&t, w, h);
    srand(time(NULL));

    tm.tv_sec = 0;
    tm.tv_nsec = 1000000;

    tetris_new_block(&t, PLAYER1);
    tetris_new_block(&t, PLAYER2);

    while (!t.gameover) {
        nanosleep(&tm, NULL);
        count++;
        if (count % 50 == 0) {
            tetris_print(&t);
            
        }
        if (count % 350 == 0) {
            tetris_gravity(&t);
            tetris_check_lines(&t);
        }
        while ((cmd = getchar()) > 0) {
            switch (cmd) {
                case 'a':
                    t.x--;
                    if (tetris_hittest(&t, PLAYER1) || tetris_collidetest(&t, PLAYER1))
                        t.x++;
                    break;
                case 'd':
                    t.x++;
                    if (tetris_hittest(&t, PLAYER1) || tetris_collidetest(&t, PLAYER1))
                        t.x--;
                    break;
                case 's':
                    t.y++;
                    if (tetris_collidetest(&t, PLAYER1)) {
                        t.y--;
                    } else {
                        if (tetris_hittest(&t, PLAYER1)) {
                            t.y--;
                            tetris_print_block(&t, PLAYER1);
                            tetris_new_block(&t, PLAYER1);
                        }
                    }
                    break;
                case 'w':
                    tetris_rotate(&t, PLAYER1);
                    break;
                case 'j':
                    t.x2--;
                    if (tetris_hittest(&t, PLAYER1) || tetris_collidetest(&t, PLAYER2))
                        t.x2++;
                    break;
                case 'l':
                    t.x2++;
                    if (tetris_hittest(&t, PLAYER1) || tetris_collidetest(&t, PLAYER2))
                        t.x2--;
                    break;
                case 'k':
                    t.y2++;
                    if (tetris_collidetest(&t, PLAYER2)) {
                        t.y2--;
                    } else {
                        if (tetris_hittest(&t, PLAYER2)) {
                            t.y2--;
                            tetris_print_block(&t, PLAYER2);
                            tetris_new_block(&t, PLAYER2);
                        }
                    }
                    break;
                case 'i':
                    tetris_rotate(&t, PLAYER2);
                    break;
            }
        }
        tm.tv_nsec = tetris_level(&t);
    }

    tetris_print(&t);
    printf("*** GAME OVER ***\n");

    tetris_clean(&t);
    tetris_cleanup_io();
}
