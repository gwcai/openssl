#include "chat.h"
#include "server.h"

int main(int argc, char **argv)
{
    int sockfd, new_fd;
    socklen_t len;
    struct sockaddr_in my_addr, their_addr;
    unsigned int myport,lisnum;
    int result;	
    result = pthread_mutex_init(&work_mutex,NULL);//initialize the mutex    

    if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 7838;

    if (argv[2])
        lisnum = atoi(argv[2]);
    else
        lisnum = LISNUM;  //the max number of connected client

    /*SSl initialize*/
    SSL_library_init();    
    OpenSSL_add_all_algorithms(); /*load algorithms of SSL*/    
    SSL_load_error_strings();     /*load error infor*/
    /*create a ssl_ctx */
    ctx = SSL_CTX_new(SSLv23_server_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /*certificate the user's ctx*/
    if (SSL_CTX_use_certificate_file(ctx, argv[4], SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /*load privatekey*/
    if (SSL_CTX_use_PrivateKey_file(ctx, argv[5], SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /*certificate privatekey*/
    if (!SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /*create a socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    } else
        printf("socket created\n");

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myport);
    if (argv[3])
        my_addr.sin_addr.s_addr = inet_addr(argv[3]);
    else
        my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
        == -1) {
        perror("bind");
        exit(1);
    } else
        printf("binded\n");

    if (listen(sockfd, LISNUM) == -1) {
        perror("listen");
        exit(1);
    } else
        printf("begin listen\n");
        
   while (1) {
       
        len = sizeof(struct sockaddr);
        /*accept connection*/
        if ((new_fd =
             accept(sockfd, (struct sockaddr *) &their_addr,
                    &len)) == -1) {
            perror("accept");
            exit(errno);
        } else{
              printf("server: got connection from %s, port %d, socket %d\n",
              inet_ntoa(their_addr.sin_addr),
              ntohs(their_addr.sin_port), new_fd);
		/*create a new ssl*/
       	ssl = SSL_new(ctx);
		/*combinate the ssl with asocket*/
        	SSL_set_fd(ssl, new_fd);
       	 /*create a safe connection*/
        	if (SSL_accept(ssl) == -1) {
            		perror("accept");
            		break;
        	}
	         /*create a thread to solve the communication*/
		beforechat(ssl);
 
 		result = pthread_create(thread+(count++),NULL,recv_data,(void *)ssl);
 		result = pthread_create(thread+(count++),NULL,send_data,(void *)ssl);

		if(result !=0){
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}
       }
    close(sockfd);
    SSL_CTX_free(ctx);
    return 0;
}
void *send_data(void *arg)
{
	SSL *ssl = (SSL *)arg;
	
	bzero(data,sizeof(data));
	while((scanf("%s",data)) != EOF)
 	{
		if(strncmp("quit",data,4) == 0)
			break;
  		SSL_write(ssl,data,strlen(data));
 	}
	SSL_shutdown(ssl);
       SSL_free(ssl);
	pthread_exit(NULL);
}
void *recv_data(void *arg)
{
	SSL *ssl = (SSL *)arg;
	int recbytes;
	char toall[MAXBUF];
	char *all = "TO ALL:";
	struct user *p;
	bzero(recvdata,sizeof(recvdata));
	int ret = 0;
	while(1)
	{
		p = root->next;
		bzero(toall,sizeof(toall));
		recbytes = SSL_read(ssl,recvdata,MAXBUF);	
		if(-1 == recbytes)
 		{
    			printf("read data fail !\r\n");
			goto finish;
 		}else if(0 == recbytes){
			printf("the other side has terminal the chat!\n");
			break;
		}
		else{
 			strcat(toall,all);
			strcat(toall,recvdata);
			printf("%s\n",toall);	
			while(p != NULL)
			{
				SSL_write(ssl,toall,strlen(toall));
				p = p->next;
			}
		}
	}
	finish:
		SSL_shutdown(ssl);
        	SSL_free(ssl);
		pthread_exit(NULL);
}
void beforechat(SSL *ssl)
{
	int ret = 1;
	int n;
	struct user *node;
	inituser(&root);  //初始化
	bzero(name,sizeof(name));	
	bzero(passwd,sizeof(passwd));
	SSL_write(ssl,buf1,strlen(buf1));
	while((n = SSL_read(ssl,name,20) <= 0))
	{
		if(n > 0) break;
		SSL_write(ssl,buf1,strlen(buf1));
	}
	SSL_write(ssl,buf5,strlen(buf5));
	while((n = SSL_read(ssl,passwd,20) <= 0))
	{
		if(n > 0) break;
                SSL_write(ssl,buf5,strlen(buf5));
	}
	printf("name:%s\n",name);
	printf("passwd:%s\n",passwd);
	ret = search(root,name);
	if(!ret){
		node = (struct user*)malloc(sizeof(struct user));
		memset(node,0,sizeof(user));
		node->next = NULL;
		strcpy(node->name,name);
		strcpy(node->passwd,passwd);
		node->ssl = ssl;
		if(insert(&root,node))  //插入链表
			SSL_write(ssl,buf4,strlen(buf4));
		else SSL_write(ssl,buf3,strlen(buf3)); 
		free(node);
		//break;
	}else{
		SSL_write(ssl,buf2,strlen(buf2));
	}
}

int inituser(user **root)
{
  	*root = (struct user*)malloc(sizeof(struct user));
	if(*root != NULL)
	{
		memset(*root,0,sizeof(struct user));
		(*root)->next = NULL;
		return 1;
	}
	return 0;
}

int insert(user **root,user *node)
{
	
	struct user *head;
	struct user *temp;
	head = *root;
	temp = head;
	printf("开始插入\n");
	while(head->next != NULL)
	{	
		head = head->next;
	}
	head->next = node;
	*root= temp;
	mount++;
	return 1;
}
int search(user *root,char *name)
{
	struct user *p = root->next;
	if(mount == 0) return 0;
	while(p != NULL)
	{

		if(strcmp(p->name,name) == 0)
		return 1;		
		p = p->next;
	}
	return 0;
}
int delete_user(user *root,char *name)
{
	struct user *p = root->next;
	struct user *pre = root;
	while(p != NULL)
	{
		if(strcmp(p->name,name) == 0)
		{
			pre->next = p->next;
			free(p);  //释放节点
			mount--;
			return 1;
		}else{
		  pre = p;
		  p = p->next;
		}
	}
	return 0;
}
void printuser(user *root)
{
	struct user *q = root->next;
	//printf("%s\n",(*root)->next->name);
	printf("当前在线用户列表:\n");
	while(q != NULL)
	{
		printf("%s\n",q->name);
		q = q->next;
	}
}
