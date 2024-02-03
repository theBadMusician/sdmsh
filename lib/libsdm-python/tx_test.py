#!/usr/bin/python

import sys
from sdm import *

session = sdm_connect("192.168.0." + sys.argv[1], 4200)

print("Connection succesful!")

sdm_cmd(session, SDM_CMD_STOP)
sdm_rx(session, SDM_REPLY_STOP)
print("RX succesful!")

sdm_cmd(session, SDM_CMD_CONFIG, 350, 0, 3)
print("Config succesful!")
# sdm_rx(session, SDM_REPLY_REPORT, SDM_REPLY_REPORT_CONFIG, 1)
# print("RX succesful!")

data  = sdm_load_samples(session, "../../examples/1834_polychirp_re_down.dat", 1024)
data += sdm_load_samples(session, "../../examples/1834_polychirp_re_down.dat", 1024)
print("Sample load succesful!")

sdm_cmd_tx(session, data)
print("TX succesful!")

sdm_rx(session, SDM_REPLY_REPORT, SDM_REPLY_REPORT_TX_STOP)
print("RX succesful!")
