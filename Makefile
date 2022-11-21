OBJECTS = myserver.o

myserver : $(OBJECTS)
	gcc -o myserver myserver.o
myserver.o : myserver.c
	gcc -c myserver.c
clean :
	rm -f $(OBJECTS)
