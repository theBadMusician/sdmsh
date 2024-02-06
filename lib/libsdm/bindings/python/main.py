#!/usr/bin/python
import sys
import time
import sdm

def calc_delta(start, end):
    if (end < start):
        end = end + (0xffffffff - start)
        start = 0
    return end - start

def calc_distance(active, passive):
    active.delta  = calc_delta(active.send.time.tx,  active.receive.time.rx - ref_sample_number)
    passive.delta = calc_delta(passive.receive.time.rx - ref_sample_number, passive.send.time.tx)

    us = (active.delta - passive.delta) / 2.
    m  = us / 1000000. * sound_speed

    print("active.delta: %.02f, passive.delta: %.02f" % (active.delta / 1000000., passive.delta / 1000000.))
    print("%0.2f us. distance %0.2f m" % (us, m))


#sound_speed = 1450. # water
sound_speed = 340.  # air

ref_sample_number = 1024 * 16
usbl_samble_number = 51200
ref_signal_file = "/home/beng/Projects/sdmsh/examples/1834_polychirp_re_up.dat"
tx_signal_file = "/home/beng/Projects/sdmsh/examples/SIGNAL.dat"

sdm.var.log_level = sdm.FATAL_LOG | sdm.ERR_LOG | sdm.WARN_LOG
#sdm.var.log_level |= sdm.INFO_LOG | sdm.DEBUG_LOG

#########################################################################

##### >>>>>>>>>>>>>>>>>>>>> Session Setup
session = sdm.create_session("test", "192.168.0.199")

# 2 s wait in expect
session.timeout = 2000

sdm.send_config(session, 350, 0, 1, 0)
sdm.expect(session, sdm.REPLY_REPORT, sdm.REPLY_REPORT_CONFIG);

sdm.send_usbl_config(session, 0, usbl_samble_number, 3, 5);
sdm.expect(session, sdm.REPLY_REPORT, sdm.REPLY_REPORT_USBL_CONFIG);

sdm.send_ref_file(session, ref_signal_file)
print(session)

##### >>>>>>>>>>>>>>>>>>>>> RX State Setup
# sdm.add_sink_membuf(session)
#sdm.add_sink(session, "rcv-" + session.name + ".raw");

# sdm.send_rx(session, 1024)

##### >>>>>>>>>>>>>>>>>>>>> TX Setup
sdm.send_signal_file(session, ref_signal_file, tx_signal_file)
session.send.time = sdm.receive_systime(session)

print(session.send.time.tx)

##### >>>>>>>>>>>>>>>>>>>>> RX Data
try:
    sdm.expect(session, sdm.REPLY_STOP);
except sdm.TimeoutError as err:
    print("Fail to receive signal on %s side" % session.name)
    sdm.send_stop(session)
    exit(1)

session.receive.data = sdm.get_membuf(session);

session.receive.usbl_data = sdm.receive_usbl_data(session, usbl_samble_number, '')
session.receive.time      = sdm.receive_systime(session)

print(session.receive)