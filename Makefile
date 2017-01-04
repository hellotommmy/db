dbms:main.o comm.o queue.o create.o drop.o insert.o select.o group.o

	gcc -o dbms main.o comm.o queue.o create.o drop.o insert.o select.o group.o

main.o:main.c

	gcc -c main.c

comm.o:comm.c comm.h

	gcc -c comm.c

queue.o:queue.c queue.h

	gcc -c queue.c

create.o:create.c

	gcc -c create.c

drop.o:drop.c

	gcc -c drop.c

insert.o:insert.c

	gcc -c insert.c

select.o:select.c

	gcc -c select.c

group.o:group.c

	gcc -c group.c
clean:

	rm dbms main.o comm.o queue.o create.o drop.o insert.o select.o group.o
