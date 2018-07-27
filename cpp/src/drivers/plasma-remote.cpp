#include <plasma/client.h>
#include <arrow/tensor.h>
#include <arrow/type.h>
#include <arrow/builder.h>
#include <arrow/array.h>
#include <arrow/buffer.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/writer.h>
#include <random>

#include <zmq.hpp>

using namespace plasma;
using namespace arrow;
using namespace std::chrono;
using namespace zmq;

float *generate_input(int input_length)
{
  std::random_device rd;        // Will be used to obtain a seed for the random number engine
  std::mt19937 generator(rd()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> distribution(0.1, 1.0);

  // int input_length = 10;
  float *input_buffer = reinterpret_cast<float *>(malloc(input_length * sizeof(float)));
  for (int j = 0; j < input_length; ++j)
  {
    input_buffer[j] = distribution(generator);
  }

  return input_buffer;
}

int main(int argc, char **argv)
{
  if (argc != 1) {
    std::cout << "Usage: ./p-remote.o <Model and Plasma Server IP>\n" << std::endl;
  }
  std::string ip = std::string(argv[0]);
  std::string model_addr = "tcp://" + ip + ":5560";
  std::string plasma_addr = "tcp://" + ip + ":5561";
  
  context_t context_1(1);
  socket_t requester(context_1, ZMQ_REQ);
  requester.connect(model_addr);

  context_t context_2(1);
  socket_t dumper(context_2, ZMQ_REQ);
  dumper.connect(plasma_addr);

  for (int i = 0; i < 2000; i++)
  {
    if (i % 100 == 0)
    {
      std::cerr << i << std::endl;
    }

    ObjectID object_id = ObjectID::from_random();

    // Generate Float Arr
    int64_t input_length = 224 * 224 * 3;
    float *input = generate_input(input_length);

    // Send id to plamsa server
    message_t id(object_id.binary().data(), 20);
    dumper.send(id, ZMQ_SNDMORE);

    // Send size to plasma server
    message_t size(&input_length, sizeof(int64_t));
    dumper.send(size, ZMQ_SNDMORE);

    // Send data to plasma server
    message_t data(input, sizeof(float) * input_length);
    dumper.send(data);

    // Recv ack from plasma server
    message_t reply;
    dumper.recv(&reply);

    // Send id to python "model" server
    message_t id2(20);
    memcpy(id2.data(), object_id.binary().data(), 20);
    requester.send(id2);

    // Recv ack from python "model" server
    message_t reply2;
    requester.recv(&reply2);
  }
}
