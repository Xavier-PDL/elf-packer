all: packer stub hw
packer: packer.c
	gcc -o $@ $^ 

stub: stub.c
	gcc -o $@ $^

hw: hw.c
	gcc -o $@ $^

test:
	./packer 
	./output

clean:
	rm -f packer stub hw payload output *~ .*~
