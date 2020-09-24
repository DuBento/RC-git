all: tcp udp

tcp: tcp_server tcp_client

udp: udp_server udp_client

tcp_server: TCP/tcp_server.c
	gcc TCP/tcp_server.c -o TCP/tcp_server

udp_server: UDP/udp_server.c
	gcc UDP/udp_server.c -o UDP/udp_server

tcp_client: TCP/tcp_client.c
	gcc TCP/tcp_client.c -o TCP/tcp_client

udp_client: UDP/udp_client.c
	gcc UDP/udp_client.c -o UDP/udp_client

clean:
	rm TCP/tcp_server
	rm TCP/tcp_client
	rm UDP/udp_server
	rm UDP/udp_client
