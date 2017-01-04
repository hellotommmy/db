dbms:main.o comm.o create.o drop.o insert.o select.o

     gcc –o dbms main.o comm.o create.o drop.o insert.o select.o

main.o:main.c

     gcc –c main.c

comm.o:comm.c comm.h

     gcc -c comm.c

create.o:create.c

     gcc –c create.c

drop.o:drop.c

     gcc –c drop.c

insert.o:insert.c

     gcc –c insert.c

select.o:select.c

     gcc -c select.c

clean:

     rm test main.o comm.o create.o drop.o insert.o select.o