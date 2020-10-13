# FLAGS

DEBUG = -Wall -Wextra

FLAGS = -lpthread

CC = gcc


all: NP

NP: ./src/*
	$(CC) -o NP $(FLAGS) ./src/main.c #$(DEBUG)





