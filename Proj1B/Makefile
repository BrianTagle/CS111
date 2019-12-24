#NAME: Brian Tagle
#EMAIL: taglebrian@gmail.com
#ID:604907076

.SILENT:

both: client server

client:
	gcc -Wall -Wextra -lmcrypt lab1b-client.c -o lab1b-client
server:
	gcc -Wall -Wextra -lmcrypt lab1b-server.c -o lab1b-server

clean:
	rm -rf lab1b-client lab1b-server lab1b-604907076.tar.gz
dist:
	tar -czf lab1b-604907076.tar.gz lab1b-client.c lab1b-server.c README Makefile my.key
