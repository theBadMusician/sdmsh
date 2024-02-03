PROJ = sdm
LIBSDM_DIR  = ../libsdm
LIBSTREAM_DIR  = ../libstream
PYTHON_INC = /usr/include/python3.10

TEST_NODE_rx = 131
TEST_NODE_tx = 127

.PHONY: build
build: lib $(LIBSDM_DIR)/libsdm.so _$(PROJ).so

$(PROJ)_wrap.c: $(PROJ).i
	#swig -python -E -I$(LIBSDM_DIR) $(PROJ).i
	#swig -python -debug-tmsearch -I$(LIBSDM_DIR) $(PROJ).i
	swig -python -I$(LIBSDM_DIR) $(PROJ).i

_$(PROJ).so: $(PROJ)_wrap.c
	$(CC) -DLOGGER_ENABLED -I$(LIBSDM_DIR) -I$(LIBSTREAM_DIR) -L$(LIBSDM_DIR) -I$(PYTHON_INC) -shared -fPIC $(LIBSDM_DIR)/sdm.o $(LIBSDM_DIR)/janus/janus.o $(LIBSTREAM_DIR)/stream.o $(LIBSDM_DIR)/utils.o $(LIBSTREAM_DIR)/stream_raw.o $(LIBSTREAM_DIR)/stream_ascii.o $(LIBSTREAM_DIR)/stream_popen.o $(LIBSTREAM_DIR)/stream_tcp.o -o $@ $(PROJ)_wrap.c

.PHONY: lib
lib:
	make -C $(LIBSDM_DIR)

$(LIBSDM_DIR)/libsdm.so:
	make -C $(LIBSDM_DIR)

rx tx: build
	LD_LIBRARY_PATH=$(LIBSDM_DIR) ./$@_test.py $(TEST_NODE_$@)
	#LD_LIBRARY_PATH=$(LIBSDM_DIR) valgrind --tool=memcheck --suppressions=valgrind-python.supp ./$@_test.py $(TEST_NODE_$@)

clean:
	rm -f $(PROJ).py $(PROJ)_wrap.* $(OBJ) *~ *.pyc .*.sw? *.so
