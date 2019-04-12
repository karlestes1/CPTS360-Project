myfs: main.o util.o init.o commands.o makefile
	gcc -w -o myfs main.o util.o init.o commands.o

main.o: main.c init.h util.h type.h commands.h
	gcc -c -w main.c

util.o: util.c util.h type.h
	gcc -c -w util.c

init.o: init.c init.h util.h type.h
	gcc -c -w init.c

commands.o: commands.c commands.h util.h type.h
	gcc -c -w commands.c

clean:
	rm myfs main.o util.o init.o commands.o

newdisk:
	rm disk
	cp mydisk disk