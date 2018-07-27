import pyarrow.plasma as plasma
import zmq
import pyarrow as pa
client = plasma.connect('/tmp/plasma','',0)

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind('tcp://*:5560')

def get_np(oid):
    buff = client.get_buffers([oid])[0]
    reader = pa.BufferReader(buff)
    t = pa.read_tensor(reader)
    return t.to_numpy()

while True:
    oid = socket.recv()
    get_np(plasma.ObjectID(oid))
    socket.send_string("")