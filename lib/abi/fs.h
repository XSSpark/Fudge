unsigned int fs_auth(char *path);
unsigned int fs_list(unsigned int target, unsigned int id, unsigned int offset, struct record *records, unsigned int nrecords);
unsigned int fs_map(unsigned int target, unsigned int id);
unsigned int fs_read(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int fs_read_full(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int fs_read_all(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int fs_stat(unsigned int target, unsigned int id, struct record *record);
unsigned int fs_walk(unsigned int target, unsigned int parent, char *path);
unsigned int fs_write(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int fs_write_all(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int fs_spawn(char *path);
unsigned int fs_spawn_relative(char *path, char *parent);
