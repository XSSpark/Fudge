struct video_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct system_node colormap;
    unsigned int id;
    struct ctrl_videosettings settings;
    void (*setmode)(struct ctrl_videosettings *settings);
    unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned int (*rcolormap)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned int (*wcolormap)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);

};

void video_registerinterface(struct video_interface *interface, unsigned int id);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, void (*setmode)(struct ctrl_videosettings *settings), unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int size, unsigned int count, void *buffer));
