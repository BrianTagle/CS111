#Name: Brian Tagle
#Email: taglebrian@gmail.com
#ID: 604907076

.SILENT:
#PNGs = lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png


default:
	gcc -Wall -Wextra -pthread -g -lprofiler SortedList.c lab2_list.c -o lab2_list

tests: default
	chmod +x tests.sh
	./tests.sh

profile: default
	rm -f profile.out
	LD_PRELOAD=tools/lib/libprofiler.so CPUPROFILE=./raw.gperf ./lab2_list --iterations=1000 --threads=12 --sync=s
	/bin/pprof --text ./lab2_list ./raw.gperf >> profile.out
	/bin/pprof --list=thread_function ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf


graphs: tests

	chmod +x graphingScript.gp
	./graphingScript.gp


dist: default graphs 
	tar -czf lab2b-604907076.tar.gz README Makefile SortedList.h SortedList.c lab2_list.c lab2b_list.csv tests.sh graphingScript.gp profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
clean:
	rm -f lab2b-604907076.tar.gz lab2_add lab2_list 



