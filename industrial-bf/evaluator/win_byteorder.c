// Assuming that windows is always little-endian

uint32_t htonl(uint32_t x) {
    return (x >> 24) | 
           ((x >> 8) & 0x0000FF00) | 
           ((x << 8) & 0x00FF0000) | 
           (x << 24);
}

#define ntohl htonl
