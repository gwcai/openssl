#include"chat.h"

void *receivemessage(void *arg);
void beforechat(SSL *ssl);

pthread_t thread;
char data[MAXBUF+1];
char buffer[MAXBUF + 1];
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
    int fd;
    void *buf;
    int result;
    void *status;
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
	/*initialize signal*/
	sigemptyset(&set);
	sigaddset(&set,SIGUSR2);
	sigaddset(&set,SIGUSR1);
	sigprocmask(SIG_SETMASK,&set,NULL);
	beforechat(ssl);
	pthread_create(&thread,NULL,receivemessage,(void *)ssl);
	pthread_kill(thread,SIGUSR1); //开始信号
	 while(fgets(data,sizeof(data),stdin))
        {
		data[strlen(data)-1] = '\0';
		if(strcmp("quit",data) == 0)
		{
			pthread_kill(thread,SIGUSR2);//结束线程信号
			break; 
		}      
                SSL_write(ssl,data,strlen(data));
                bzero(data,sizeof(data));
        }
    close(sockfd);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
}
void help_infor()
{
	printf("*********欢迎来到openssl聊天室**********\n");
	printf("********注册成功后默认为群聊模式********\n");
	printf("****私聊请用如下格式:to [user name]:****\n");
	printf("*********查看在线用户列表:!show*********\n");
}
int login(SSL *ssl)
{
	char result[50];
	bzero(result,sizeof(result));
	SSL_write(ssl,"LOGIN",5);
	SSL_read(ssl,result,50);
	printf("%s\n",result);
	bzero(result,sizeof(result));
	/*输入用户名*/
	scanf("%s",result);
	SSL_write(ssl,result,strlen(result));
	bzero(result,sizeof(result));
	SSL_read(ssl,result,100);
	printf("%s\n",result);
	if(strcmp("请输入密码",result) == 0)
	{
		while(1)
		{
			bzero(result,sizeof(result));
			/*输入密码*/
			scanf("%s",result);
			SSL_write(ssl,result,strlen(result));
			bzero(result,sizeof(result));
			SSL_read(ssl,result,50);
			printf("%s\n",result);
			if(strcmp("登录成功",result) == 0)
				break;
 		}
		return 1;
	}
	return 0;
}
int user_register(SSL *ssl)
{
	char result[50];
	bzero(result,sizeof(result));
        SSL_write(ssl,"REGISTER",8);
        SSL_read(ssl,result,50);
        printf("%s\n",result);
	while(1)
	{
        	bzero(result,sizeof(result));
		scanf("%s",result);
		SSL_write(ssl,result,strlen(result));
		bzero(result,sizeof(result));
		SSL_read(ssl,result,50);
		printf("%s\n",result);
		if(strcmp("OK",result) == 0)
			break;
	}
	bzero(result,sizeof(result));
        SSL_read(ssl,result,50);
        printf("%s\n",result);
        if(strcmp("请输入密码",result) == 0)
        {
               while(1) {
			bzero(result,sizeof(result));
			scanf("%s",result);
                        SSL_write(ssl,result,strlen(result));
			bzero(result,sizeof(result));
                        SSL_read(ssl,result,50);
                        printf("%s\n",result);
                        if(strcmp("OK",result) == 0)
                                break;
                }
		bzero(result,sizeof(result));
		SSL_read(ssl,result,50);
		printf("%s\n",result);
		if(strcmp("注册成功",result) == 0)
                	return 1;
        }
        return 0;	
}
void beforechat(SSL *ssl)
{
	char choice;
	char temp;	
	help_infor();
	while(1)
	{
		printf("请选择要进行的操作:\n");
        	printf("a、登录服务器\tb、注册\n");
		scanf("\n%c",&choice);
		//printf("%c",choice);
		if(choice == 'a')
		{
			if(login(ssl) == 1)
				break;
		}else if(choice == 'b'){
			if(user_register(ssl) == 1)
					break;
		}else
			printf("重新选择\n");
	}
}

void *receivemessage(void *arg)
{
 	SSL *ssl = (SSL*)arg;
	int signum;
	sigwait(&set,&signum);
	int recbytes;
	if(signum == SIGUSR1)
	{	
		while(1)
		{
			bzero(buffer,sizeof(buffer));
       			recbytes = SSL_read(ssl,buffer,MAXBUF);
       			if(-1 == recbytes)
       			{
               			printf("read data fail !\r\n");
               			break;
       			}else if(0 == recbytes){
                		printf("the server has terminal the chat!\n");
               			break;
       			}	
       			else
               			printf("%s\n",buffer);
       		}
	}
	if(SIGUSR2 == signum)
		pthread_exit(NULL);	
}
