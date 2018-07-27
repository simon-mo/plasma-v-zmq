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
  context_t context_1(1);
  socket_t requester(context_1, ZMQ_REQ);
  requester.connect("tcp://localhost:5560");

  context_t context_2(1);
  socket_t dumper(context_2, ZMQ_REQ);
  dumper.connect("tcp://localhost:5561");

  for (int i = 0; i < 2000; i++)
  {
    printf("back to top of the loop, ");

    if (i % 100 == 0)
    {
      std::cerr << i << std::endl;
    }

    ObjectID object_id = ObjectID::from_random();
    printf("Genereated new oid, ");

    // std::cout << object_id.binary() << std::endl;

    // Generate Float Arr
    int64_t input_length = 224 * 224 * 3;
    float *input = generate_input(input_length);
    printf("Generated new input, ");

    // message_t id(20);
    // memcpy(id.data(), object_id.binary().data(), 20);
    message_t id(object_id.binary().data(), 20);
    dumper.send(id, ZMQ_SNDMORE);

    message_t size(&input_length, sizeof(int64_t));
    dumper.send(size, ZMQ_SNDMORE);
    // dumper.send(id);

    zmq_send(&dumper, object_id.data(), 20 * sizeof(char), ZMQ_SNDMORE);

    message_t data(input, sizeof(float) * input_length);
    dumper.send(data);
    printf("Sent request to dumper, ");

    message_t reply;
    dumper.recv(&reply);
    printf("Recv ack from dumper, ");

    message_t id2(20);
    memcpy(id2.data(), object_id.binary().data(), 20);
    requester.send(id2);
    printf("Sent request to python, ");

    message_t reply2;
    requester.recv(&reply2);
    printf("Recv ack from python\n");
  }
}
