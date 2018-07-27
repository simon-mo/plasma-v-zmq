import zmq
import numpy as np
import time
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind('tcp://*:5560')
import gc
gc.disable()

while True:
    msg = socket.recv()
    socket.send_string("")

