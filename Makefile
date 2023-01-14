
output: producer.o worker.o
	gcc producer.o worker.o -o output -pthread -lrt

producer.o: producer.c
	gcc -c producer.c -pthread -lrt

worker.o: worker.c 
	gcc -c worker.c -pthread -lrt

clean:
	rm *.o output 