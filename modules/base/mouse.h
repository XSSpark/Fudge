struct base_mouse_interface
{

    struct base_interface base;
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

struct base_mouse_node
{

    struct system_node base;
    struct system_node data;
    struct base_device *device;
    struct base_mouse_interface *interface;

};

void base_mouse_register_interface(struct base_mouse_interface *interface);
void base_mouse_register_node(struct base_mouse_node *node);
void base_mouse_unregister_interface(struct base_mouse_interface *interface);
void base_mouse_unregister_node(struct base_mouse_node *node);
void base_mouse_init_interface(struct base_mouse_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_mouse_init_node(struct base_mouse_node *node, struct base_device *device, struct base_mouse_interface *interface);
void base_mouse_setup();
