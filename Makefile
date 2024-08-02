build:
	gcc -Wall -std=c99 -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lm ./src/*.c -o renderer
run:
	./renderer
clean:
	rm renderer
