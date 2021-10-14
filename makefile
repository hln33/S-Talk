all: main.c 
	gcc -g -Wall -Werror main.c receiver.c input.c print.c transmit.c list.c shutdownManager.c -lpthread -o s-talk

clean:
	rm -f s-talk