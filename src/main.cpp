
#define WIDTH 16
#define HEIGHT 16

#include <deque>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode () {

	std::cout << "\x1b[?25h";

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

}

void enableRawMode () {

	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}

struct pos {

	uint8_t x, y;

	pos () : x(0), y(0) {}

	pos (uint8_t x, uint8_t y) : x(x), y(y) {}

};

enum DIRECTION {
	RIGHT,
	UP,
	DOWN,
	LEFT
};

pos food;

std::deque<pos> snake;

void spawnFood () {

	start:
	food.x = rand() % WIDTH;
	food.y = rand() % HEIGHT;

	for (pos block : snake) if (block.x == food.x && block.y == food.y) goto start;

}

void keyListener () {

	uint8_t arrow_stage = 0;

	while (1) {
		char c = '\0';
		read(STDIN_FILENO, &c, 1);
		switch (arrow_stage) {
			case 0:
				if (c == 'q') goto quit;
				if (c == 's') {
					std::cout << "\x1b[" << (food.y+2) << ';' << (food.x+2) << "f " << std::flush;
					spawnFood();
					std::cout << "\x1b[31m\x1b[" << (food.y+2) << ';' << (food.x+2) << "fo\x1b[0m" << std::flush;
				}
				// TODO arrow key code
		}
	}

quit: // TODO quit code
	return;

}

int main () {

	srand(time(NULL));

	enableRawMode();

	snake.emplace_back(1,1);
	snake.emplace_back(2,1);
	snake.emplace_back(3,1);

	spawnFood();

	// hide cursor
	std::cout << "\x1b[?25l";

	// draw walls
	std::cout << "\x1b[2J\x1b[H\x1b[38;5;214m";
	for (int i = 0; i < WIDTH + 2; i++) std::cout << 'x';
	std::cout << '\n';
	for (int i = 0; i < HEIGHT; i++) {
		std::cout << 'x';
		for (int j = 0; j < WIDTH; j++) std::cout << ' ';
		std::cout << "x\n";
	}
	for (int i = 0; i < WIDTH + 2; i++) std::cout << 'x';
	
	// draw snake
	std::cout << "\x1b[32m";
	for (pos block : snake) {

		std::cout << "\x1b[" << (block.y+2) << ';' << (block.x+2) << "f#";
		
	}

	// draw food
	std::cout << "\x1b[31m";
	std::cout << "\x1b[" << (food.y+2) << ';' << (food.x+2) << "fo";
	std::cout << "\x1b[0m";

	std::cout << "\x1b[" << (HEIGHT+3) << ";0f";

	std::cout << std::flush;

	// start thread
	std::thread key_thread (keyListener);

	key_thread.join();

	return 0;

}

