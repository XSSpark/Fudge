MOD_$(DIR):=$(BUILD_MODULE)/msr.ko.0
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/msr.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
