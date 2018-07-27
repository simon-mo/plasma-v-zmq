all: bin/p-local.o bin/p-remote.o bin/zmq-local.o bin/p-server.o bin/p-fine.o

bin/p-local.o: cpp/src/drivers/plasma-local.cpp
	g++ -Wall cpp/src/drivers/plasma-local.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-local.o

bin/p-remote.o: cpp/src/drivers/plasma-remote.cpp
	g++ -Wall cpp/src/drivers/plasma-remote.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-remote.o

bin/p-fine.o: cpp/src/drivers/plasma-local-fine-grained.cpp
	g++ -Wall --std=c++11 cpp/src/drivers/plasma-local-fine-grained.cpp `pkg-config --cflags --libs plasma arrow` -lzmq -obin/p-fine.o

bin/zmq-local.o: cpp/src/drivers/zmq-local.cpp
	g++ -Wall cpp/src/drivers/zmq-local.cpp --std=c++11 -lzmq -obin/zmq-local.o

bin/zmq-remote.o: cpp/src/drivers/zmq-remote.cpp
	g++ -Wall cpp/src/drivers/zmq-remote.cpp --std=c++11 -lzmq -obin/zmq-remote.o

bin/zmq-remote-fine.o: cpp/src/drivers/zmq-remote-fine-grained.cpp
	g++ -Wall cpp/src/drivers/zmq-remote-fine-grained.cpp --std=c++11 -lzmq -obin/zmq-remote-fine.o

bin/p-server.o: cpp/src/server/plasma-server.cpp
	g++ -Wall cpp/src/server/plasma-server.cpp `pkg-config --cflags --libs plasma arrow` -lzmq --std=c++11 -obin/p-server.o

clean:
	rm bin/*.o

