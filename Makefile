default:ssl_client ssl_server
	make clean
ssl_client:ssl_client.o
	g++ -o ssl_client ssl_client.o  -L /usr/local/ssl/lib -lssl -lpthread -lcrypto -ldl
ssl_client.o:ssl_client.cpp chat.h
	g++ -c ssl_client.cpp -I/usr/local/ssl/include -L/usr/local/ssl/lib -lssl -lcrypto -ldl -lpthread
ssl_server:ssl_server.o
	g++ -o ssl_server ssl_server.o -lpthread  -L/usr/local/ssl/lib -lssl -lcrypto -ldl
ssl_server.o:ssl_server.cpp chat.h server.h
	g++ -c ssl_server.cpp -I/usr/local/ssl/include -L/usr/local/ssl/lib -lssl -lcrypto -ldl -lpthread	
clean:
	rm *.o
