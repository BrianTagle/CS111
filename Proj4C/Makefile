#NAME: Brian Tagle
#EMAIL: taglebrian@gmail.com
#ID: 604907076

.SILENT:

default:
	gcc -Wall -Wextra -lmraa -lm lab4c_tcp.c -o lab4c_tcp
	gcc -Wall -Wextra -lmraa -lm -lcrypto  -lssl lab4c_tls.c -o lab4c_tls

clean:
	rm -f lab4c-604907076.tar.gz lab4c_tcp lab4c_tls

dist: 
	tar -czf lab4c-604907076.tar.gz lab4c_tls.c lab4c_tcp.c README Makefile
