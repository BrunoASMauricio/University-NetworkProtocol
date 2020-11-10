# FLAGS

DEBUG = -Wall -Wextra

FLAGS = -lpthread -pthread

CC = g++


all: NP

NP: ./src/*
	$(CC) -g -o NP $(FLAGS) ./src/main.c #$(DEBUG)

DEBUG: ./src/*
		$(CC) -g -o NP $(FLAGS) ./src/main.c $(DEBUG)

