build: wish

clean:
	rm -rf wish wish.o

wish: wish.o
	gcc $< -o $@ -lreadline -Wall

wish.o: main.c
	gcc -std=gnu99 -c $< -o $@ -lreadline -Wall
