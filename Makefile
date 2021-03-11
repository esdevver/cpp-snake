SRC_FILES = $(shell find src -name '*.cpp')

all: build run

build:
	@mkdir -p bin
	g++ $(SRC_FILES) -o ./bin/main -pthread

run:
	./bin/main

