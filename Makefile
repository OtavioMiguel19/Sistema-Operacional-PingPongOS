all:
	gcc -o ppos-test ppos-core-aux.c ppos_disk.c disk.c pingpong-disco2.c -lrt libppos_static.a

clean:
	rm -rf ppos-test
