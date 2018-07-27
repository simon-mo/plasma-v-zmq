all: bin/p-local.o bin/p-remote.o bin/zmq-local.o bin/p-server.o bin/p-fine.o

bin/p-local.o:
	g++ -Wall cpp/src/drivers/plasma-local.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-local.o

bin/p-remote.o:
	g++ -Wall cpp/src/drivers/plasma-remote.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-remote.o

bin/p-fine.o:
	g++ -Wall --std=c++11 cpp/src/drivers/plasma-local-fine-grained.cpp `pkg-config --cflags --libs plasma arrow` -lzmq -obin/p-fine.o

bin/zmq-local.o:
	g++ -Wall cpp/src/drivers/zmq-local.cpp --std=c++11 -lzmq -obin/zmq-local.o

bin/p-server.o:
	g++ -Wall cpp/src/server/plasma-server.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-server.o

clean:
	rm bin/*.o

