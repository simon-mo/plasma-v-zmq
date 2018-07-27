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

#include <sstream>
#include <iostream>
#include <string>

using namespace plasma;
using namespace arrow;
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

long long add_timestamp(std::chrono::time_point<std::chrono::high_resolution_clock> start) {
  auto end = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(end - start);
  return duration.count();
}

int main(int argc, char** argv) {

  PlasmaClient client_;
  ARROW_CHECK_OK(client_.Connect("/tmp/plasma", "", 0));

  context_t context(1);
  socket_t requester(context, ZMQ_REQ);
  requester.connect("tcp://localhost:5560");

  std::vector<std::vector<long long>> durations_collection = {};

  for (int i=0; i < 2000; i++) {
    // ObjectID object_id = ObjectID::from_binary("00000000000000000000");
    if (i % 100 == 0) {
      std::cerr << i << std::endl;
    }
    ObjectID object_id = ObjectID::from_random();

    // std::cout << object_id.binary() << std::endl;

    // Generate Float Arr
    int64_t input_length = 224*224*3;
    float* input = generate_input(input_length);

    std::vector<long long> durations = {};
    auto start = high_resolution_clock::now();

    // Create Tensor
    const uint8_t* casted = reinterpret_cast<const uint8_t*>(input);
    auto value_buffer = std::make_shared<Buffer>(casted, 4*input_length);
    Tensor t(float32(), value_buffer, {input_length});
    durations.push_back(add_timestamp(start));

    // Get Meta Infomation
    int64_t datasize;
    ARROW_CHECK_OK(ipc::GetTensorSize(t, &datasize));
    int32_t meta_len = 0;
    durations.push_back(add_timestamp(start));
    
    // Create Plasma Object and Write Tensor to the buffer
    std::shared_ptr<Buffer> buffer;
    ARROW_CHECK_OK(
        client_.Create(object_id, datasize, NULL, 0, &buffer));
    durations.push_back(add_timestamp(start));

    io::FixedSizeBufferWriter stream(buffer);
    ARROW_CHECK_OK(arrow::ipc::WriteTensor(t, &stream, &meta_len, &datasize));
    durations.push_back(add_timestamp(start));
    
    // Seal Plasma Object
    ARROW_CHECK_OK(client_.Seal(object_id));
    durations.push_back(add_timestamp(start));

    
    message_t id(20);
    memcpy(id.data(), object_id.binary().data(), 20);

    
    requester.send(id);
    durations.push_back(add_timestamp(start));

    message_t reply;
    requester.recv(&reply);


    durations.push_back(add_timestamp(start));
    durations_collection.push_back(durations);
  }

  for(std::vector<long long> durations: durations_collection) {
    std::stringstream ss;
    for(auto dur: durations){
      ss << dur << ",";
    }
    std::cout << ss.str() << std::endl;
  }


  // Disconnect the client.
  ARROW_CHECK_OK(client_.Disconnect());
}
