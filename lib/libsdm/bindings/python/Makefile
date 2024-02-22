PROJ = sdm
BUILD_DIR = ./
LIBSDM_DIR     = ../..
LIBSDM_BUILD_DIR = $(LIBSDM_DIR)/build
LIBSTREAM_DIR  = ../../../libstream
LIBSTREAM_BUILD_DIR = $(LIBSTREAM_DIR)/build
LIBS  = -L$(LIBSDM_BUILD_DIR) -I$(LIBSDM_DIR) -L$(LIBSTREAM_BUILD_DIR) -I$(LIBSTREAM_DIR)
OBJS = $(LIBSDM_BUILD_DIR)/sdm.o $(LIBSDM_BUILD_DIR)/janus.o $(LIBSTREAM_BUILD_DIR)/stream.o $(LIBSDM_BUILD_DIR)/utils.o $(LIBSTREAM_BUILD_DIR)/stream_raw.o $(LIBSTREAM_BUILD_DIR)/stream_ascii.o $(LIBSTREAM_BUILD_DIR)/stream_popen.o $(LIBSTREAM_BUILD_DIR)/stream_tcp.o
PYTHON_INC = /usr/include/python3.10
TESTS_BASE = tests
TEST = usbl

WRAP_SRC  = $(PROJ)_wrap.c
SWIG_OPT  = -python -globals var
SWIG_OPT += -py3
CC = gcc

TEST_NODE_active = 219
TEST_NODE_passive = 221

.PHONY: build
build: $(BUILD_DIR) lib $(LIBSDM_BUILD_DIR)/libsdm.so $(LIBSTREAM_BUILD_DIR)/libstream.so $(BUILD_DIR)/_$(PROJ).so

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(WRAP_SRC): $(PROJ).i
	swig -v -Wall $(SWIG_OPT) -I$(LIBSDM_DIR) -I$(LIBSTREAM_DIR) $(PROJ).i

$(BUILD_DIR)/_$(PROJ).so: $(PROJ)_wrap.c
	$(CC) -DLOGGER_ENABLED $(LIBS) -I$(PYTHON_INC) -shared -fPIC -l$(PROJ) $(OBJS) -o $@ $(WRAP_SRC)

.PHONY: lib
lib:
	make -C $(LIBSDM_DIR) && make -C $(LIBSTREAM_DIR)

$(LIBSDM_BUILD_DIR)/libsdm.so:
	make -C $(LIBSDM_DIR)

$(LIBSTREAM_BUILD_DIR)/libstream.so:
	make -C $(LIBSTREAM_DIR)

active passive: build
	LD_LIBRARY_PATH=$(LIBSDM_BUILD_DIR):$(LIBSTREAM_BUILD_DIR) PYTHONPATH=. $(TESTS_BASE)/$(TEST)/$(TEST)-$@.py $(TEST_NODE_$@)

clean:
	rm -f $(PROJ).py $(PROJ)_wrap.* *~ *.pyc .*.sw? $(BUILD_DIR)/*
