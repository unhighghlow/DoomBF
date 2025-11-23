// Assuming that windows is always little-endian

unsigned int htonl(unsigned int x) {
    return (x >> 24) | 
           ((x >> 8) & 0x0000FF00) | 
           ((x << 8) & 0x00FF0000) | 
           (x << 24);
}

#define ntohl htonl
