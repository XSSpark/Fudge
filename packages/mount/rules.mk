BIN_$(DIR):=$(BUILD_BIN)/mount
OBJ_$(DIR):=$(DIR)/mount.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(PACKAGES_LIBS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
