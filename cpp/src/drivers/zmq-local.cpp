#include <random>
#include <iostream>
#include <zmq.hpp>

using namespace std::chrono;
using namespace zmq;

float* generate_input(int input_length) {
  std::random_device rd;         // Will be used to obtain a seed for the random number engine
  std::mt19937 generator(rd());  // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> distribution(0.1, 1.0);

  // int input_length = 10;
  float* input_buffer = reinterpret_cast<float*>(malloc(input_length * sizeof(float)));
  for (int j = 0; j < input_length; ++j) {
      input_buffer[j] = distribution(generator);
  }

  return input_buffer;
}

int main(int argc, char** argv) {

  context_t context(1);
  socket_t requester(context, ZMQ_REQ);
  requester.connect("tcp://localhost:5560");

  std::vector<long long> durations = {};

  for (int i=0; i < 2000; i++) {
    // ObjectID object_id = ObjectID::from_binary("00000000000000000000");
    if (i % 100 == 0) {
      std::cerr << i << std::endl;
    }

    // std::cout << object_id.binary() << std::endl;

    // Generate Float Arr
    int64_t input_length = 224*224*3;
    float* input = generate_input(input_length);

    auto start = high_resolution_clock::now();

    message_t msg_data(input, input_length*sizeof(float));
    requester.send(msg_data);

    message_t reply;
    requester.recv(&reply);

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    durations.push_back(duration.count());
  }

  for(long long dur: durations) {
    std::cout << dur << std::endl;
  }
}
