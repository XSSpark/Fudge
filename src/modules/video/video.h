struct video_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int ichannel;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int (*setcmap)(unsigned int source, unsigned int count, void *data);
    unsigned int (*setmode)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp);


};

void video_notifymode(struct video_interface *interface, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*setcmap)(unsigned int source, unsigned int count, void *data), unsigned int (*setmode)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp));
