#!/usr/bin/python
import sys
import time
import sdm

#sound_speed = 1450. # water
sound_speed = 340.  # air

ref_sample_number = 1024 * 16
usbl_samble_number = 51200
ref_signal_file = "/home/beng/Projects/sdmsh/examples/1834_polychirp_re_up.dat"
tx_signal_file = "/home/beng/Projects/sdmsh/examples/SIGNAL.dat"

sdm.var.log_level = sdm.FATAL_LOG | sdm.ERR_LOG | sdm.WARN_LOG
sdm.var.log_level |= sdm.INFO_LOG | sdm.DEBUG_LOG

#########################################################################

##### >>>>>>>>>>>>>>>>>>>>> Session Setup
session = sdm.create_session("test", "192.168.0.189")

# 2 s wait in expect
session.timeout = 2000

sdm.send_config(session, 350, 0, 2, 0)
sdm.expect(session, sdm.REPLY_REPORT, sdm.REPLY_REPORT_CONFIG);

sdm.send_usbl_config(session, 0, usbl_samble_number, 3, 5);
sdm.expect(session, sdm.REPLY_REPORT, sdm.REPLY_REPORT_USBL_CONFIG);

sdm.send_ref_file(session, ref_signal_file)
print(session)

##### >>>>>>>>>>>>>>>>>>>>> RX State Setup
sdm.add_sink_membuf(session)
print(1)
sdm.add_sink(session, "~/Projects/sdmsh/examples/rcv-" + session.name + ".dat");
print(1)
sdm.send_rx(session, 1024)
print(1)
##### >>>>>>>>>>>>>>>>>>>>> TX Setup
# sdm.send_signal_file(session, ref_signal_file, tx_signal_file)
# session.send.time = sdm.receive_systime(session)

# print(session.send.time.tx)

##### >>>>>>>>>>>>>>>>>>>>> RX Data
sdm.expect(session, sdm.REPLY_STOP);
print(2)
# try:
#     sdm.expect(session, sdm.REPLY_STOP);
# except sdm.TimeoutError as err:
#     print("Fail to receive signal on %s side" % session.name)
#     sdm.send_stop(session)
#     exit(1)
print(3)
session.receive.data = sdm.get_membuf(session);

# print(4)
# session.receive.usbl_data = sdm.receive_usbl_data(session, usbl_samble_number, '')

# print(5)
# session.receive.time      = sdm.receive_systime(session)

print(session.receive.data)