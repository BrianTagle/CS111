#Name: Brian Tagle
#Email: taglebrian@gmail.com
#ID: 604907076

.SILENT:
CC=gcc
CFLAGS =-Wall -Wextra
FILES = lab0.c Makefile README backtrace.png breakpoint.png

lab0: lab0.c 
	$(CC) $(CFLAGS) lab0.c -o lab0	

check: lab0 inputoutputTest inputfileErrorTest outputfileErrorTest unrecognizedargumentTest signalTest
clean: 
	rm -f lab0 lab0-604907076.tar.gz
dist: 
	tar -czf lab0-604907076.tar.gz $(FILES)

inputoutputTest:
	echo "test text" > input.txt; \
	./lab0 --input=input.txt --output=output.txt &> /dev/null; \
	if [ $$? -ne 0 ]; then \
		echo "inputoutputTest failed (could not open files)"; \
	else \
		cmp input.txt output.txt; \
		if [ $$? -ne 0 ]; then \
			echo "inputoutputTest failed (input and output mismatch)"; \
		else \
			echo "inputoutputTest successful"; \
		fi \
	fi 
	rm input.txt output.txt

unrecognizedargumentTest:
	./lab0 --unrecognizedArgument &> /dev/null; \
	if [ $$? -ne 1 ]; then \
	echo "unrecognizedargumentTest failed"; \
	else \
	echo "unrecognizedargumentTest successful"; \
	fi

inputfileErrorTest:
	echo "h" > input.txt; 
	rm input.txt; 
	./lab0 --input=input.txt &> /dev/null; \
	if [ $$? -ne 2 ]; then \
	echo "inputfileErrorTest failed"; \
	else \
	echo "inputfileErrorTest successful"; \
	fi

outputfileErrorTest:
	echo "test text" > input.txt;
	echo "" > output.txt; 
	chmod -w output.txt
	./lab0 --input=input.txt --output=output.txt &> /dev/null; \
	if [ $$? -ne 3 ]; then \
	echo "outputfileErrorTest failed"; \
	else \
	echo "outputfileErrorTest successful"; \
	fi
	rm -f input.txt output.txt

signalTest:
	./lab0 --catch --segfault &> /dev/null; \
	if [ $$? -ne 4 ]; then \
	echo "signalTest failed"; \
	else \
	echo "signalTest successful"; \
	fi	

