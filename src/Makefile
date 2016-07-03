CFLAGS = `guile-config compile`
LIBS = `guile-config link`

build: wish

clean:
	rm -rf wish wish.o

wish: wish.o
	gcc $< -o $@ -lreadline -Wall $(LIBS)


wish.o: main.c
	gcc -std=gnu99 -c $< -o $@ -lreadline -Wall $(CFLAGS)
