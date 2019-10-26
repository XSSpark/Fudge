unsigned int call_auth(unsigned int descriptor, unsigned int backend, unsigned int protocol);
unsigned int call_close(unsigned int descriptor);
unsigned int call_create(unsigned int descriptor, char *name, unsigned int length);
unsigned int call_debug(void);
unsigned int call_destroy(unsigned int descriptor, char *name, unsigned int length);
unsigned int call_despawn(void);
unsigned int call_load(unsigned int descriptor);
unsigned int call_mount(unsigned int pdescriptor, unsigned int cdescriptor);
unsigned int call_open(unsigned int descriptor);
unsigned int call_pick(struct ipc_header *header, void *data);
unsigned int call_place(unsigned int id, struct ipc_header *header, void *data);
unsigned int call_read(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int call_seek(unsigned int descriptor, unsigned int offset);
unsigned int call_spawn(unsigned int descriptor);
unsigned int call_step(unsigned int descriptor);
unsigned int call_unload(unsigned int descriptor);
unsigned int call_walk(unsigned int descriptor, unsigned int pdescriptor, char *path, unsigned int length);
unsigned int call_write(unsigned int descriptor, void *buffer, unsigned int count);
