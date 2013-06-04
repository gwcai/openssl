#include"chat.h"

void *sendmessage(void *arg);
pthread_t thread;
char data[MAXBUF+1];
void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("Infor of Cert:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Cert%s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer:%s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("No Cert Infor\n");
}

int main(int argc, char **argv)
{
    int sockfd, len;
    struct sockaddr_in dest;
    char buffer[MAXBUF + 1];
    int fd;
    int result;
    SSL_CTX *ctx;
    SSL *ssl;
    int recbytes;

    if (argc != 3) {
        printf
            ("error.use like this:\n\t\t%s IP Port\n\tex:\t%s 127.0.0.1 80\n",argv[0], argv[0]);
        exit(0);
    }

    /*initialize the SSL library*/
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /*Create socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }
    printf("socket created\n");

    /*initialize server address and port infor*/
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        perror(argv[1]);
        exit(errno);
    }
    printf("address created\n");

    /*connect the server*/
    if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        exit(errno);
    }
    printf("server connected\n");

    /*create a new ssl based on the ctx*/
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    /*receive message from server*/
    if (SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);
    else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);
    }
	printf("Chat now!!\n");
	pthread_create(&thread,NULL,sendmessage,(void *)ssl);
	while(1)
	{
		
	bzero(buffer,sizeof(buffer));
	recbytes = SSL_read(ssl,buffer,MAXBUF);
 	if(-1 == recbytes)
 	{
    		printf("read data fail !\r\n");
		return -1;
 	}else if(0 == recbytes){
		printf("the server has terminal the chat!\n");
		return 0;
	}
	else
 		printf("REC:%s\n",buffer);
   	}
  finish:
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    return 0;
}
void *sendmessage(void *arg)
{
 SSL *ssl = (SSL*)arg;
 bzero(data,sizeof(data));
 while((scanf("%s",data)) != EOF)
 {
  SSL_write(ssl,data,strlen(data));
 }
	pthread_exit(NULL);
}
 

