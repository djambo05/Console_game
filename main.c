#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define SNAKE_INCREASE 3

volatile sig_atomic_t signal_status = 0;

void sighandler(int s) {
  signal_status = s;
}

void init() {
  initscr();
  savetty();
  nonl(); // no new line (after pressing ENTER)
  cbreak(); // buffer no sending (after pressing ENTER)
  noecho(); // no displaying your writing down (by pressing key of keyboard)
  timeout(0); // no finishing execution
  leaveok(stdscr, TRUE); // carriage movement is TRUE
  curs_set(0); //hide cursor
  signal(SIGINT, sighandler);
  signal(SIGQUIT, sighandler);
  signal(SIGWINCH, sighandler);
  signal(SIGTSTP, sighandler);
}

void finish() {
  curs_set(1); //cursor recovery
  clear();
  refresh(); //screen cleaning
  resetty(); //settings recovery
  endwin(); //finish console  working
  exit(0);
}

char **board;

int lines, cols;

void clear_board() {
  for (int i = 0; i < LINES; i++) {
    for (int j = 0; j < COLS; j++) {
      board[i][j] = ' ';
    }
  }
}

void init_board() {
  lines = LINES;
  cols = COLS;
  board = (char **) malloc(LINES * sizeof(char*)); //create board dinamically
  if (!board) {
    finish();
  }
  for (int i = 0; i < LINES; i++) {
    board[i] = (char*) malloc(COLS * sizeof(char));
    if (!board[i]) {
      finish();
    }
  }
  clear_board();
}

#define MAX_SNAKE_LENGTH 500

typedef struct Coord {
  int row;
  int col;
} Coord;

Coord snake[MAX_SNAKE_LENGTH];

#define POINT '*'
#define INIT_SNAKE_LENGTH 10

int snakeSize = 1; //current snake size
int snakeLength = INIT_SNAKE_LENGTH; //required snake size

void put(int row, int col, char ch) { //growning snake
  board[row][col] = ch; // put start symbol on a board
  move(row, col); //shake location
  addch(ch); //add symbol
}

Coord get_random_location() {
  Coord res;
  while (1) {
    res.row = rand() % lines; // lines limit
    res.col = rand() % cols; // cols limit
    if (board[res.row][res.col] == ' ') {
      break;
    }
  }
  return res; //random snake location
}

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define STOP 4
#define APPLE '@'

int direction = STOP; //starting direction
int game_over = 0;

void put_apple();

void change_direction() { //changing direction
  int keypress;
  keypress = wgetch(stdscr);
  if (keypress == ERR) {
    return;
  }
  if (keypress == 'q') {
    finish();
  }
  if (game_over) {
    if (keypress == 'r') {
      clear_board();
      snakeLength = INIT_SNAKE_LENGTH;
      snakeSize = 1;
      snake[0] = get_random_location();
      direction = STOP;
      game_over = 0;
      clear();
      refresh();
      put(snake[0].row, snake[0].col, POINT);
      put_apple();
    }
    return;
  }
  if (keypress == 'a') {
    direction = LEFT;
  } else if (keypress == 'd') {
    direction = RIGHT;
  } else if (keypress == 'w') {
    direction = UP;
  } else if (keypress == 's') {
    direction = DOWN;
  }
}

//----------
//**
// *#
//----------
//snake = [1:2, 1:1, 0:1, 0:0]
//(after change_direction(RIGHT))
//----------
// *
// **#
//----------
//snake = [1:3, 1:2, 1:1, 0:1, <0:0>]

void change_head() {
  if (direction == RIGHT) {
    snake[0].col++;
  } else if (direction == LEFT) {
    snake[0].col--;
  } else if (direction == DOWN) {
    snake[0].row++;
  } else if (direction == UP) {
    snake[0].row--;
  }
}

int verify_head() {
  if (snake[0].row < 0 || snake[0].col < 0
      || snake[0].row >= lines || snake[0].col >= cols) {
    return -1;
  }
  if (board[snake[0].row][snake[0].col] != ' '
    && board[snake[0].row][snake[0].col] != APPLE) {
    return -1;
  }
  return 0;
}

void logic() {
  memmove(&snake[1], &snake[0], sizeof(Coord) * snakeSize); //copy info from snake[0] position to snake[1] position and multiply size of one element with current snake size
  change_head();
  if (verify_head()) {
    direction = STOP;
    game_over = 1;
    return;
  }
  if (board[snake[0].row][snake[0].col] == APPLE) {
    snakeLength += SNAKE_INCREASE;
    put_apple();
  }
  put(snake[0].row, snake[0].col, POINT);
  if (snakeSize < snakeLength) {
    snakeSize++;
  } else {
    put(snake[snakeSize].row, snake[snakeSize].col, ' '); //write ' ' to delete *
  }
}

void put_apple() {
  Coord pos = get_random_location();
  put(pos.row, pos.col, APPLE);
}

int main() {
  srand((unsigned) time(NULL));
  init();
  init_board();
  snake[0] = get_random_location();
  put(snake[0].row, snake[0].col, POINT);
  put_apple();
  while (1) {
    if (signal_status) {
      finish();
    }
    change_direction();
    if (direction != STOP && !game_over) {
      logic();
    }
    wgetch(stdscr); //check on click
    napms(50);
  }

  finish();
  return 0;
}
