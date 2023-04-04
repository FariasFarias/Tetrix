all: tetrix

tetrix:
	gcc tetrix.c -o tetrix.exe -static -lncurses -L"C:\MinGW\lib" -lpthread