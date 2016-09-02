unsigned int call2_auth(unsigned int descriptor, unsigned int backend);
unsigned int call2_close(unsigned int descriptor);
unsigned int call2_create(unsigned int descriptor, char *name, unsigned int length);
unsigned int call2_destroy(unsigned int descriptor, char *name, unsigned int length);
unsigned int call2_despawn(void);
unsigned int call2_load(unsigned int descriptor);
unsigned int call2_mount(unsigned int pdescriptor, unsigned int cdescriptor);
unsigned int call2_open(unsigned int descriptor);
unsigned int call2_read(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int call2_seek(unsigned int descriptor, unsigned int offset);
unsigned int call2_spawn(void);
unsigned int call2_unload(unsigned int descriptor);
unsigned int call2_walk(unsigned int descriptor, unsigned int pdescriptor, char *path, unsigned int length);
unsigned int call2_write(unsigned int descriptor, void *buffer, unsigned int count);
