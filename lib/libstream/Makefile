PROJ = libstream
BUILD_DIR = build

SRC = stream.c stream_raw.c stream_ascii.c stream_tcp.c stream_popen.c
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

CFLAGS = -W -Wall -I. -lm -ggdb -DLOGGER_ENABLED -fPIC

all: $(BUILD_DIR) $(BUILD_DIR)/$(PROJ).so $(BUILD_DIR)/$(PROJ).a

$(BUILD_DIR)/$(PROJ).so: $(OBJ)
	$(CC) -shared $(CFLAGS) -o $@ $^

$(BUILD_DIR)/$(PROJ).a: $(OBJ)
	$(AR) rcs $@ $^

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/$(PROJ).so $(BUILD_DIR)/$(PROJ).a $(BUILD_DIR)/* *~ .*.sw?

.PHONY: all clean
