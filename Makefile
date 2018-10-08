all: 
	make clean
	make all_251_stub

all_251_stub: Sender_Stub.o Receiver.o Sender257.o Sender997.o
	g++ Sender997.o -o Sender997.out | g++ Receiver.o -o Receiver.out -pthread | g++ Sender257.o -o Sender257.out | g++ Sender_Stub.o -o Sender_Stub.out

all_but_251: Sender257.o Sender997.o Receiver.o
	g++ Sender997.o -o Sender997.out | g++ Receiver.o -o Receiver.out | g++ Sender257.o -o Sender257.out

all_stub: Sender_Stub.o Receiver.o
	g++ Sender_Stub.o -o Sender_Stub.out | g++ Receiver.o -o Receiver.out

all_997: Sender997.o Receiver.o
	g++ Sender997.o -o Sender997.out | g++ Receiver.o -o Receiver.out
	
all_257: Sender257.o Receiver.o
	g++ Sender257.o -o Sender257.out | g++ Receiver.o -o Receiver.out

output_receiver.o: Receiver.o Components.o
	g++ Receiver.o -o Receiver.out

output_sender.o: Sender_Stub.o Components.o
	g++ Sender_Stub.o -o Sender.out

Sender997.o:
	g++ -c Sender997.cpp MsgPigeon.cpp

Sender257.o:
	g++ -c Sender257.cpp MsgPigeon.cpp

Sender_Stub.o:	patch/patch64.o
	g++ -c Sender_Stub.cpp Sender.h PTools.cpp MsgPigeon.cpp patch/patch64.o patch/get_info.h

Receiver.o: 
	g++ -c Receiver.cpp Receiver.h PTools.cpp MsgPigeon.cpp

clean:
	rm -f *.out *.gch *.o
