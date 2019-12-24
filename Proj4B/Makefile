#NAME: Brian Tagle
#EMAIL: taglebrian@gmail.com
#ID: 604907076
.SILENT:

default:
	gcc -Wall -Wextra -lmraa -lm lab4b.c -o lab4b

check: default
	chmod +x test.sh
	./test.sh
clean:
	rm -f lab4b-604907076.tar.gz lab4b
dist:
	tar -czf lab4b-604907076.tar.gz Makefile README lab4b.c test.sh



