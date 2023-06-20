preemp: ppos-core-aux.o libppos_static.a pingpong-preempcao.o
			gcc ppos-core-aux.c libppos_static.a pingpong-preempcao.c -o preemp

sche: ppos-core-aux.o libppos_static.a pingpong-scheduler.o
			gcc ppos-core-aux.c libppos_static.a pingpong-scheduler.c -o sche

disco1: ppos-core-aux.o libppos_static.a ppos_disk.o pingpong-disco1.o disk.o
			gcc ppos-core-aux.c libppos_static.a ppos_disk.c pingpong-disco1.c disk.c -lrt

disco2: ppos-core-aux.o libppos_static.a ppos_disk.o pingpong-disco2.o disk.o
			gcc ppos-core-aux.c libppos_static.a ppos_disk.c pingpong-disco2.c disk.c -lrt

ppos-core-aux.o: ppos-core-aux.c
			gcc -c ppos-core-aux.c

pingpong-preempcao.o: pingpong-preempcao.c
			gcc -c pingpong-preempcao.c

pingpong-scheduler.o: pingpong-scheduler.c
			gcc -c pingpong-scheduler.c

disk.o: disk.c
			gcc -c disk.c

ppos_disk.o: ppos_disk.c
			gcc -c ppos_disk.c

pingpong-disco1.o: pingpong-disco1.c
			gcc -c pingpong-disco1.c

pingpong-disco2.o: pingpong-disco2.c
			gcc -c pingpong-disco2.c

clean: *.o preemp sche disco1 disco2
			rm *.o	preemp sche disco1 disco2
		
target: dependencies
	action