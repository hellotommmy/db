dbms:main.o comm.o queue.o create.o drop.o insert.o select.o group.o create_table.o drop_table.o insert_table.o select_basic.o

	gcc -o dbms main.o comm.o queue.o create.o drop.o insert.o select.o group.o  create_table.o drop_table.o insert_table.o select_basic.o

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

create_table:create_table.c

	gcc -c create_table.c

drop_table:drop_table.c

	gcc -c drop_table.c

insert_table:insert_table.c

	gcc -c insert_table.c

select_basic:select_basic.c

	gcc -c select_basic.c

clean:

	rm dbms main.o comm.o queue.o create.o drop.o insert.o select.o group.o  create_table.o drop_table.o insert_table.o select_basic.o
