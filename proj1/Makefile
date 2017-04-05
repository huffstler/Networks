CC= /usr/bin/gcc
all:	tcpclient tcpserver udpclient udpserver nonblock-udpclient

tcpclient: tcpclient.c;
	${CC} tcpclient.c -o tcpclient

tcpserver: tcpserver.c;
	${CC} tcpserver.c -o tcpserver

udpclient: udpclient.c;
	${CC} udpclient.c -o udpclient

udpserver: udpserver.c;
	${CC} udpserver.c -o udpserver

nonblock-udpclient:	nonblock-udpclient.c;
	$(CC) nonblock-udpclient.c -o nonblock-udpclient

clean:
	rm tcpclient tcpserver udpclient udpserver nonblock-udpclient
