#ifndef HAT2_H_INCLUDED
#define HAT2_H_INCLUDED

int send_msg(SOCKET sock, int ver, u_char *msg, size_t len, int timeout);

#endif // HAT2_H_INCLUDED
