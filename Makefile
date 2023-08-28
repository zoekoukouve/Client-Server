ALL: parent

parent: parent.o child.o aid_functions.o
	gcc parent.o child.o aid_functions.o -o parent -lpthread

parent.o: parent.c
	gcc -g -Wall -c parent.c

child.o: child.c
	gcc -g -Wall -c child.c

aid_functions.o: aid_functions.c
	gcc -g -Wall -c aid_functions.c

clean:
	rm -f parent parent.o child.o aid_functions.o