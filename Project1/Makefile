all: application skeen

skeen: skeen.c 
	gcc skeen.c -o skeen -lsctp -lpthread -Wall

application: phase2.c
	gcc phase2.c -o Application 

clean:
	rm -rf skeen Application 
