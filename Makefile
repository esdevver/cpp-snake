SRC_FILES = $(shell find src -name '*.cpp')

all: build run

build:
	# Compiling
	@mkdir -p bin
	g++ $(SRC_FILES) -o ./bin/main -pthread -Werror -O3

run:
	# Running
	./bin/main

clean:
	rm bin -r

