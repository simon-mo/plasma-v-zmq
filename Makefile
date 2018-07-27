all: bin/p-local.o bin/p-remote.o bin/zmq-local.o bin/p-server.o

bin/p-local.o:
	g++ cpp/src/drivers/plasma-local.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-local.o

bin/p-remote.o:
	g++ cpp/src/drivers/plasma-remote.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-remote.o

bin/zmq-local.o:
	g++ cpp/src/drivers/zmq-local.cpp -lzmq --std=c++11 -obin/zmq-local.o

bin/p-server.o:
	g++ cpp/src/server/plasma-server.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-server.o

clean:
	rm bin/*.o