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

  PlasmaClient client_;
  ARROW_CHECK_OK(client_.Connect("/tmp/plasma", "", 0));

  context_t context(1);
  socket_t requester(context, ZMQ_REQ);
  requester.connect("tcp://localhost:5560");

  std::vector<long long> durations = {};

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

    auto start = high_resolution_clock::now();

    // Create Tensor
    const uint8_t* casted = reinterpret_cast<const uint8_t*>(input);
    auto value_buffer = std::make_shared<Buffer>(casted, 4*input_length);
    Tensor t(float32(), value_buffer, {input_length});

    // Get Meta Infomation
    int64_t datasize;
    ARROW_CHECK_OK(ipc::GetTensorSize(t, &datasize));
    int32_t meta_len = 0;
    
    // Create Plasma Object and Write Tensor to the buffer
    std::shared_ptr<Buffer> buffer;
    ARROW_CHECK_OK(
        client_.Create(object_id, datasize, NULL, 0, &buffer));

    io::FixedSizeBufferWriter stream(buffer);
    ARROW_CHECK_OK(arrow::ipc::WriteTensor(t, &stream, &meta_len, &datasize));
    
    // Seal Plasma Object
    ARROW_CHECK_OK(client_.Seal(object_id));

    
    message_t id(20);
    memcpy(id.data(), object_id.binary().data(), 20);

    
    requester.send(id);

    message_t reply;
    requester.recv(&reply);

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    durations.push_back(duration.count());
  }

  for(long long dur: durations) {
    std::cout << dur << std::endl;
  }


  // Disconnect the client.
  ARROW_CHECK_OK(client_.Disconnect());
}
