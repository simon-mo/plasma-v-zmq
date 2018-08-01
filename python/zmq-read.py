import zmq
import numpy as np
import time
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind('tcp://*:5560')
import gc
gc.disable()

def get_np(msg):
    return np.frombuffer(msg, dtype=float)

py_time = []
for i in range(2000):    
    msg = socket.recv()
    get_np(msg)
    socket.send_string("")
