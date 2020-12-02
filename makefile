all: dserver.c dclient.c
	gcc dserver.c -o dserver -lm
	gcc dclient.c -o dclient

clean: 
	rm dserver dclient
