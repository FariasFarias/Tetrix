all: tetrix

tetrix:
	gcc tetrix.c include\functions_game.c -o tetrix.exe -static -lncurses -lpthread -g