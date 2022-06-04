naturalsum: naturalsum.c
	gcc -g -Wall $^ -o $@


.PHONY: clean

clean:
	rm naturalsum
