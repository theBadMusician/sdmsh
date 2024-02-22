PROJ = libsdm
BUILD_DIR = build

# Include directories in VPATH
VPATH = .:janus

SRC = sdm.c utils.c janus.c
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

CFLAGS = -W -Wall -I. -I../libstream -L../libstream/build -lstream -lm -ggdb -DLOGGER_ENABLED -fPIC

all: $(BUILD_DIR) $(BUILD_DIR)/$(PROJ).so $(BUILD_DIR)/$(PROJ).a

$(BUILD_DIR)/$(PROJ).so: $(OBJ)
	$(CC) -shared $(CFLAGS) -o $@ $^

$(BUILD_DIR)/$(PROJ).a: $(OBJ)
	$(AR) rcs $@ $^

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# General rule for object files
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/$(PROJ).so $(BUILD_DIR)/$(PROJ).a $(BUILD_DIR)/* *~ .*.sw?

.PHONY: all clean
