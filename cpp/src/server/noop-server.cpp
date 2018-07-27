#include <zmq.hpp>

using namespace zmq;

int main(int argc, char **argv) {

  context_t context(1);
  socket_t server(context, ZMQ_REP);
  server.bind("tcp://*:5560");

  while (true) {
    message_t id;
    server.recv(&id);
    message_t ack(&"", 1);
    server.send(ack);
  }
}
