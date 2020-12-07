struct socket_tcp
{

    unsigned int state;
    unsigned int seq;
    unsigned char port[TCP_PORTSIZE];

};

struct socket_udp
{

    unsigned char port[UDP_PORTSIZE];

};

struct socket
{

    unsigned int resolved;
    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned char paddress[IPV4_ADDRSIZE];
    union { struct socket_tcp tcp; struct socket_udp udp; } info;

};

unsigned int socket_handle_arp(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int count, void *buffer);
unsigned int socket_handle_icmp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer);
unsigned int socket_handle_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_handle_udp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_send_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata);
unsigned int socket_send_udp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata);
void socket_listen(unsigned int descriptor, unsigned char protocol, struct socket *local);
void socket_connect(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote, struct socket *router);
void socket_resolveremote(unsigned int descriptor, struct socket *local, struct socket *remote);
void socket_resolvelocal(unsigned int descriptor, struct socket *socket);
void socket_bind(struct socket *socket, unsigned char address[IPV4_ADDRSIZE]);
void socket_bind_tcp(struct socket *socket, unsigned char port[TCP_PORTSIZE], unsigned int seq);
void socket_bind_udp(struct socket *socket, unsigned char port[UDP_PORTSIZE]);
void socket_init(struct socket *socket);
