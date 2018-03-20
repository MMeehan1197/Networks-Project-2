CC= /usr/bin/gcc
all:	tcpclient tcpserver udpclient udpserver

tcpclient: tcpclient.c;
	${CC} tcpclient.c -o tcpclient

tcpserver: tcpserver2.c;
	${CC} tcpserver2.c -o tcpserver

udpclient: udpclient.c;
	${CC} udpclient.c -o udpclient

udpserver: udpserver.c;
	${CC} udpserver.c -o udpserver

clean:
	rm tcpclient tcpserver udpclient udpserver
