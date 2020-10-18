# FLAGS

DEBUG = -Wall -Wextra

FLAGS = -lpthread -pthread

CC = g++


all: NP

NP: ./src/*
	$(CC) -o NP $(FLAGS) ./src/main.c #$(DEBUG)





