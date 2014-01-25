#include "packet.hpp"
#include <winsock2.h>

int send_msg(SOCKET sock, int ver, u_char *msg, size_t len, int timeout) {
  	const u_char end[] = {0x64, 0x01, 0x00, 0x00, 0x00};
	u_char *STp = 0;
	int ret_code = 0, err_code;
	u_int idx = 0;
	u_char pack[0x96];
 	fd_set fd;
	struct timeval tv;
	int nBytesSent;

	STp = (u_char *) malloc(len + 5);
	memcpy(STp, msg, len);
	memcpy(STp + len, end, 5);

	memset(pack, 0, 8);

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	len += 5;

	while (idx < len) {
		if (len - idx > 0x8E) {
			pack[0] = 0x8E;
			pack[7] = 0;
		} else {
			pack[0] = (u_char) (len - idx);
			pack[7] = 1;
		}
		memcpy(pack + 8, STp + idx, pack[0]);
		PACKET_XorByKey(pack + 8, pack[0], ver);

		nBytesSent = 0;
		while (nBytesSent < pack[0] + 8) {
			FD_ZERO(&fd);
			FD_SET(sock, &fd);
			tv.tv_sec = timeout;
			tv.tv_usec = 0;
			if (select(sock + 1, NULL, &fd, NULL, &tv) == 0) {
				ret_code = -2;
				goto ex; /// timeout
			}
			err_code = send(sock, (const char *)pack, pack[0] + 8, 0);
			if (err_code == SOCKET_ERROR || !err_code) {
				ret_code = -1;
				goto ex; /// send error
			}
			nBytesSent += err_code;
		}
		idx += pack[0];
	}

ex:
	if (STp) free(STp);
	return ret_code;
}
