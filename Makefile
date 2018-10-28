all: 
	make clean_all
	make Receiver.out
	make Sender251.out
	make Sender257.out
	make Sender997.out
	make clean

all_stub:
	make clean_all 
	make Receiver.out
	make Sender251_Stub.out
	make Sender257.out
	make Sender997.out
	make clean

Sender997.out: Sender997.o
	g++ Sender997.o -o Sender997.out

Sender997.o:
	g++ -c Sender997.cpp

Sender257.out: Sender257.o
	g++ Sender257.o -o Sender257.out

Sender257.o:
	g++ -c Sender257.cpp

Sender251.out: 251_sender.o
	g++ 251_sender.o patch64.o -o Sender251.out
	
Sender251.o:
	g++ -c 251_sender.cpp

Receiver.out: Receiver.o
	g++ Receiver.o -o Receiver.out

Receiver.o:
	g++ -c Receiver.cpp

clean_all:
	mkdir temp
	mv patch64.o ./temp/patch64.o
	rm -f *.out *.gch *.o
	mv ./temp/patch64.o patch64.o
	rmdir temp
	
clean:
	mkdir temp
	mv patch64.o ./temp/patch64.o
	rm -f *.gch *.o
	mv ./temp/patch64.o patch64.o	
	rmdir temp
