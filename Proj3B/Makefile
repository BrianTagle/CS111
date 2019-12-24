#NAME: Brian Tagle
#EMAIL: taglebrian@gmail.com
#ID: 604907076
.SILENT:

default: lab3b.py

	rm -f lab3b
	ln lab3b.py lab3b
	chmod +x lab3b
	echo "compilation successful"


dist:
	tar -czf lab3b-604907076.tar.gz Makefile README lab3b.py
clean:
	rm -f lab3b-604907076.tar.gz lab3b
