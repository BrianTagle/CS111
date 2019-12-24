#Name: Brian Tagle
#Email: taglebrian@gmail.com
#ID: 604907076

.SILENT:
#PNGs = lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png


default:
	gcc -Wall -Wextra -pthread lab2_add.c -o lab2_add
	gcc -Wall -Wextra -pthread SortedList.c lab2_list.c -o lab2_list

tests: default
	chmod +x addtests.sh
	./addtests.sh
	chmod +x listtests.sh
	-./listtests.sh


graphs: tests
	chmod +x lab2_add.gp
	./lab2_add.gp
	chmod +x lab2_list.gp
	./lab2_list.gp


dist: default graphs
	tar -czf lab2a-604907076.tar.gz README Makefile addtests.sh listtests.sh lab2_add.c lab2_add.csv SortedList.h SortedList.c lab2_list.c lab2_list.csv lab2_add.gp lab2_list.gp lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
clean:
	rm -f lab2a-604907076.tar.gz lab2_add lab2_list 



