
#define WIDTH 28
#define HEIGHT 16
#define FPS 8

#include <chrono>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode() {

  std::cout << "\x1b[?25h\x1b[2J\x1b[H";

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {

  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void setStatus() { std::cout << "\x1b[" << (HEIGHT + 4) << ";0f\x1b[K"; }

struct pos {

  union {
    struct {
      uint16_t as_number;
    };
    struct {
      uint8_t x, y;
    };
  };

  pos() : x(0), y(0) {}

  pos(uint8_t x, uint8_t y) : x(x), y(y) {}

  bool operator==(pos &other) { return x == other.x && y == other.y; }
};

enum DIRECTION : uint8_t { RIGHT, UP, LEFT, DOWN };

bool game_running;

pos food;

void spawnFood();

struct Snake {

private:
  DIRECTION dir = RIGHT, pending_dir = RIGHT;

public:
  std::deque<pos> blocks;

  Snake() {

    blocks.emplace_back(1, 1);
    blocks.emplace_back(2, 1);
    blocks.emplace_back(3, 1);
  }

  void direction(DIRECTION new_dir) {

    if ((dir ^ new_dir) & 1)
      pending_dir = new_dir;
  }

  void step() {

    dir = pending_dir;

    pos new_head = blocks.back();

    switch (dir) {
    case RIGHT:
      new_head.x++;
      break;
    case UP:
      new_head.y--;
      break;
    case LEFT:
      new_head.x--;
      break;
    case DOWN:
      new_head.y++;
      break;
    }

    bool game_over = false;

    if (new_head.x < 0 || new_head.y < 0 || new_head.x >= WIDTH ||
        new_head.y >= HEIGHT)
      game_over = true;
    else
      for (pos block : blocks)
        if (block == new_head) {
          game_over = true;
          break;
        }

    if (game_over) {
      setStatus();
      std::cout << "\x1b[1;31mGame over\x1b[0m; final score "
                << (blocks.size() - 3) << std::flush;
      game_running = false;
      return;
    }

    bool eat = false;

    if (new_head == food) {
      eat = true;
    }

    blocks.push_back(new_head);
    std::cout << "\x1b[32m\x1b[" << (new_head.y + 3) << ';' << (new_head.x + 2)
              << "f#\x1b[0m";

    if (!eat) {
      pos tail = blocks.front();
      std::cout << "\x1b[" << (tail.y + 3) << ';' << (tail.x + 2)
                << "f\x1b[38;5;234m\u253c\x1b[0m";
      blocks.pop_front();
    } else {
      if (blocks.size() == WIDTH * HEIGHT) {
        setStatus();
        std::cout << "\x1b[1;35mYou win?\x1b[0m You filled the board."
                  << std::flush;
        game_running = false;
        return;
      }
      spawnFood();
      std::cout << "\x1b[31m\x1b[" << (food.y + 3) << ';' << (food.x + 2)
                << "fo\x1b[0m";
      int score = blocks.size() - 3;
      setStatus();
      std::cout << "\x1b[1;35mScore: " << score << "\x1b[0m";
    }

    std::cout << std::flush;
  }

} snake;

void spawnFood() {

start:
  food.x = rand() % WIDTH;
  food.y = rand() % HEIGHT;

  for (pos block : snake.blocks)
    if (block == food)
      goto start;
}

void keyListener() {

  uint8_t arrow_stage = 0;

  while (1) {
    char c = '\0';
    read(STDIN_FILENO, &c, 1);
    switch (arrow_stage) {
    default:
    case 0:
      if (c == 'q')
        goto quit;
      if (c == '\e')
        arrow_stage = 1;
      else {
        DIRECTION new_dir;
        switch (c) {
        case 'w':
          new_dir = UP;
          break;
        case 'a':
          new_dir = LEFT;
          break;
        case 's':
          new_dir = DOWN;
          break;
        case 'd':
          new_dir = RIGHT;
          break;
        }
        snake.direction(new_dir);
      }
      break;
    case 1:
      if (c == '[')
        arrow_stage = 2;
      break;
    case 2:
      DIRECTION new_dir;
      switch (c) {
      case 'A':
        new_dir = UP;
        break;
      case 'B':
        new_dir = DOWN;
        break;
      case 'C':
        new_dir = RIGHT;
        break;
      case 'D':
        new_dir = LEFT;
        break;
      }
      snake.direction(new_dir);
      arrow_stage = 0;
      break;
    }
  }

quit:
  game_running = false;
}

int main() {

  srand(time(NULL));

  enableRawMode();

  spawnFood();

  // hide cursor
  std::cout << "\x1b[?25l";

  // draw walls
  std::cout << "\x1b[2J\x1b[H\x1b[1;38;5;44mArrow keys\x1b[0m to move. "
               "\x1b[1;38;5;44mq\x1b[0m to quit.\n\x1b[38;5;214m";
  for (int i = 0; i < WIDTH + 2; i++)
    std::cout << 'x';
  std::cout << '\n';
  for (int i = 0; i < HEIGHT; i++) {
    std::cout << 'x';
    std::cout << "\x1b[38;5;234m";
    for (int j = 0; j < WIDTH; j++)
      std::cout << "\u253c";
    std::cout << "\x1b[38;5;214m";
    std::cout << "x\n";
  }
  for (int i = 0; i < WIDTH + 2; i++)
    std::cout << 'x';
  std::cout << "\n\x1b[1;35mScore: 0\x1b[0m";

  // draw snake
  std::cout << "\x1b[32m";
  for (pos block : snake.blocks) {

    std::cout << "\x1b[" << (block.y + 3) << ';' << (block.x + 2) << "f#";
  }

  // draw food
  std::cout << "\x1b[31m";
  std::cout << "\x1b[" << (food.y + 3) << ';' << (food.x + 2) << "fo";
  std::cout << "\x1b[0m";

  std::cout << "\x1b[" << (HEIGHT + 3) << ";0f";

  std::cout << std::flush;

  // start thread
  std::thread key_thread(keyListener);

  game_running = true;

  while (game_running) {
    snake.step();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
  }

  key_thread.join();

  return 0;
}
