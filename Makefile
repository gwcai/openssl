SSL_INCLUDE=-I/usr/local/ssl/include
LD=-L/usr/local/ssl/lib -lssl -lcrypto -ldl
default:ssl_client ssl_server
	make clean
ssl_client:ssl_client.o
	g++ -o ssl_client ssl_client.o  -lpthread $(LD) $(SSL_INCLUDE)
ssl_client.o:ssl_client.cpp chat.h
	g++ -c ssl_client.cpp -lpthread $(LD) $(SSL_INCLUDE)
ssl_server:ssl_server.o
	g++ -o ssl_server ssl_server.o -lpthread $(LD) $(SSL_INCLUDE)
ssl_server.o:ssl_server.cpp chat.h server.h
	g++ -c ssl_server.cpp -lpthread	$(LD) $(SSL_INCLUDE)
clean:
	rm *.o
