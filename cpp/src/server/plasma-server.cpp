// This client connects to 
//   - tcp://*:5561 to receive input
//   - /tmp/plasma to dump object


#include <arrow/array.h>
#include <arrow/buffer.h>
#include <arrow/builder.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/writer.h>
#include <arrow/tensor.h>
#include <arrow/type.h>
#include <plasma/client.h>
#include <random>

#include <zmq.hpp>

using namespace plasma;
using namespace arrow;
using namespace std::chrono;
using namespace zmq;

void save_data_to_plasma(std::shared_ptr<PlasmaClient> client,
                         std::string oid_str, void *data,
                         int64_t input_length) {
  ObjectID object_id = ObjectID::from_binary(oid_str);
  // Create Tensor
  const uint8_t *casted = reinterpret_cast<const uint8_t *>(data);
  auto value_buffer =
      std::make_shared<Buffer>(casted, sizeof(float) * input_length);
  Tensor t(float32(), value_buffer, {input_length});

  // Get Meta Infomation
  int64_t datasize;
  ARROW_CHECK_OK(ipc::GetTensorSize(t, &datasize));
  int32_t meta_len = 0;

  // Create Plasma Object and Write Tensor to the buffer
  std::shared_ptr<Buffer> buffer;
  ARROW_CHECK_OK(client->Create(object_id, datasize, NULL, 0, &buffer));

  io::FixedSizeBufferWriter stream(buffer);
  ARROW_CHECK_OK(arrow::ipc::WriteTensor(t, &stream, &meta_len, &datasize));

  // Seal Plasma Object
  ARROW_CHECK_OK(client->Seal(object_id));
}

int main(int argc, char **argv) {
  auto client = std::make_shared<PlasmaClient>();
  ARROW_CHECK_OK(client->Connect("/tmp/plasma", "", 0));

  context_t context(1);
  socket_t server(context, ZMQ_REP);
  server.bind("tcp://*:5561");

  while (true) {
    message_t id;
    server.recv(&id);
    auto oid_str = std::string(reinterpret_cast<const char *>(id.data()), 20);

    message_t size;
    server.recv(&size);
    int64_t *size_p = static_cast<int64_t *>(size.data());
    int64_t input_length = *size_p;

    message_t data;
    server.recv(&data);

    save_data_to_plasma(client, oid_str, data.data(), input_length);

    message_t ack(&"", 1);
    server.send(ack);
  }
  // Disconnect the client.
  ARROW_CHECK_OK(client->Disconnect());
}
