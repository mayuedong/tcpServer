.PHONY:clean
OBJECTS=common.o memory.o sds.o list.o dict.o anet.o ae.o server.o
bulldozer:$(OBJECTS)
	gcc -g $(OBJECTS) aeepoll.o networking.o -o bulldozer
common.o:common.h
	gcc -g -c common.c
memory.o:memory.h
	gcc -g -c memory.c
sds.o:sds.h
	gcc -g -c sds.c
list.o:list.h
	gcc -g -c list.c
dict.o:dict.h
	gcc -g -c dict.c
anet.o:anet.h
	gcc -g -c anet.c
ae.o:ae.h
	gcc -g -c ae.c aeepoll.c
server.o:server.h
	gcc -g -c server.c networking.c
clean:
	rm -f bulldozer $(OBJECTS) aeepoll.o networking.o
