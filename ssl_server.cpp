#include "chat.h"
#include "server.h"

pthread_t r_thread,m_thread;
int main(int argc, char **argv)
{
	int sockfd, new_fd;
    	socklen_t len;
	SSL *ssl;
	struct user *node;
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
    	if(ctx == NULL) {
        	ERR_print_errors_fp(stdout);
        	exit(1);
    	}
    	/*certificate the user's ctx*/
    	if(SSL_CTX_use_certificate_file(ctx, argv[4], SSL_FILETYPE_PEM) <= 0) {
        	ERR_print_errors_fp(stdout);
        	exit(1);
    	}
    	/*load privatekey*/
    	if(SSL_CTX_use_PrivateKey_file(ctx, argv[5], SSL_FILETYPE_PEM) <= 0) {
        	ERR_print_errors_fp(stdout);
        	exit(1);
    	}
    	/*certificate privatekey*/
    	if(!SSL_CTX_check_private_key(ctx)) {
        	ERR_print_errors_fp(stdout);
        	exit(1);
    	}

    	/*create a socket*/
    	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        	perror("socket");
        	exit(1);
    	}else
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
	/*initialize signal*/
	sigemptyset(&set);
	sigaddset(&set,SIGUSR1);
	sigaddset(&set,SIGUSR2);
	sigprocmask(SIG_SETMASK,&set,NULL);
	inituser(&root);  //初始化用户链表        
	while (1) {
       		len = sizeof(struct sockaddr);
        	/*accept connection*/
        	if ((new_fd =
             		accept(sockfd, (struct sockaddr *) &their_addr,
                    	&len)) == -1) {
            			perror("accept");
            			exit(errno);
        	}else{
              		printf("server: got connection from %s, port %d, socket %d\n",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port), new_fd);
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
		node = beforechat(&ssl);
		pthread_mutex_lock(&work_mutex); 
 		result = pthread_create(thread+(count++),NULL,recv_data,node);
		r_thread = thread[count-1];
		pthread_kill(r_thread,SIGUSR1);
 		//pthread_create(thread+(count++),NULL,send_data,ssl);
		if(result !=0){
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
		//send_data(ssl);
		pthread_mutex_unlock(&work_mutex);
	}
       }
    if(ssl != NULL)
    {
	SSL_shutdown(ssl);
    	SSL_free(ssl);
    }
    close(sockfd);
    SSL_CTX_free(ctx);
    return 0;
}
void *send_data(void *arg)
{
	struct user *p;
	SSL *ssl = (SSL *)arg;
	bzero(data,sizeof(data));
	while(fgets(data,sizeof(data),stdin))
 	{
		data[strlen(data) -1] = '\0';
		if(strcmp("quit",data) == 0)
		{
			pthread_kill(r_thread,SIGUSR2);
			goto finish;
		}
		SSL_write(ssl,data,strlen(data));
		p = root->next;
		while(p != NULL)
                {
                       if(p->ssl != NULL)
                            SSL_write(p->ssl,data,strlen(data));
                       p = p->next;
                }
 	}
	finish:
                SSL_shutdown(ssl);
                SSL_free(ssl);
                pthread_exit(NULL);
}
void talkprivate(char *pos,user *node)
{
        int c = 3;
        char temp[20];
        char all[] = " Said to ";
	memset(toall,'\0',sizeof(toall));
        strcat(toall,node->name);
        strcat(toall,all);
	bzero(temp,sizeof(temp));
        /*获取用户姓名*/
        while(recvdata[c] != ':')
        {
              temp[c-3] = recvdata[c];
              c++;
        }
        user *q = search(root,temp);
        if(q != NULL)
        {
              strcat(toall,temp);
              strcat(toall,pos);
              SSL_write(q->ssl,toall,strlen(toall));
              printf("%s\n",toall);
        }else
              printf("用户%s不存在!\n",temp);
}
void talkpublic(user *node)
{
	struct user *p = root->next;
	bzero(toall,sizeof(toall));
	strcat(toall,node->name);
	strcat(toall,":");
        strcat(toall,recvdata);
        printf("%s\n",toall);
        while(p != NULL)
        {
             if(p->ssl != NULL && p->ssl != node->ssl)
	            SSL_write(p->ssl,toall,strlen(toall));
              p = p->next;
        }

}
void *recv_data(void *arg)
{
	char *pos = NULL;
        char find = ':';
	user *node = (user *)arg;
	//SSL * ssl = (SSL *)arg;
	int recbytes;
	struct user *p;
	int signum;
	int ret = 0;
	/*初始化signal set*/
	printf("ID = %d\n",pthread_self());
	sigwait(&set,&signum);
	if(signum == SIGUSR1){
	while(1)
	{
		p = root->next;
		bzero(recvdata,sizeof(recvdata));
		recbytes = SSL_read(node->ssl,recvdata,MAXBUF);	
		if(-1 == recbytes)
 		{
    			printf("read data fail !\r\n");
			goto finish;
 		}else if(0 == recbytes){
			bzero(toall,sizeof(toall));
			bzero(recvdata,sizeof(toall));
			strcat(toall,"user ");
			strcat(toall,node->name);
			strcat(toall," exit!");
			strcpy(recvdata,toall);
			printf("%s\n",recvdata);
			talkpublic(node);
			goto finish;
		}
		else{
			pos = strchr(recvdata,find);
			if(strcmp("!show",recvdata) == 0)
				printuser(root,node->ssl);
			else{
				if(pos !=NULL)
					/*私聊*/
					talkprivate(pos,node);
				else
					/*群聊*/
					talkpublic(node);
		     	}
		}
	}
     }
    if(signum == SIGUSR2)
	goto finish;
    finish:
	SSL_shutdown(node->ssl);
       	SSL_free(node->ssl);
	node->ssl = NULL;
	pthread_exit(NULL);
}
user *user_register(SSL *s)
{
	struct user *node;
	struct user *ret;
	char data1[] = "重新输入";
	char data2[] = "确认密码"; 
	char ok[] = "OK";
	char temp[50];
	bzero(name,sizeof(name));
        bzero(passwd,sizeof(passwd));
	SSL_write(s,buf1,strlen(buf1));
        while(1)
        {
              SSL_read(s,name,50);
		if(search(root,name) != NULL)
                	SSL_write(s,buf2,strlen(buf2));
		else{
			SSL_write(s,ok,strlen(ok));
			break;
		}
        }
	SSL_write(s,buf5,strlen(buf5));
        SSL_read(s,passwd,50);
	SSL_write(s,data2,strlen(data2));
	while(1)
	{
		bzero(temp,sizeof(temp));
		SSL_read(s,temp,50);
		if(strcmp(passwd,temp) == 0)
		{
			SSL_write(s,ok,strlen(ok));
			break;
		}
		else
			SSL_write(s,data1,strlen(data1));
	}
        node = (struct user*)malloc(sizeof(struct user));
        memset(node,0,sizeof(user));
        node->next = NULL;
        strcpy(node->name,name);
        strcpy(node->passwd,passwd);
        node->ssl = s;
        if(insert(&root,node))//注册成功
	{
                SSL_write(s,buf7,strlen(buf7));
		return node;
	}
	return NULL;
}
user *login(SSL *s)
{
	struct user *ret;
	bzero(name,sizeof(name));
        bzero(passwd,sizeof(passwd));
	SSL_write(s,buf1,strlen(buf1));
        SSL_read(s,name,50);
        ret = search(root,name);
	if(ret == NULL)
        {
                SSL_write(s,buf6,strlen(buf6));
		return NULL;
        }else if(ret->ssl != NULL)
	{
		SSL_write(s,buf3,strlen(buf3));
		return NULL;
	}
	else{
		SSL_write(s,buf5,strlen(buf5));
		while(1)
		{
			SSL_read(s,passwd,50);
			if(strcmp(passwd,ret->passwd) == 0)
			{
				SSL_write(s,buf4,strlen(buf4));
				ret->ssl = s;
				return ret;
			}else{//密码输入错误
				SSL_write(s,buf8,strlen(buf8));
			}
		}
	}
	return NULL;
}
user *beforechat(SSL **ssl)
{
	user *ret;
	char choice[20];
	SSL *s = *ssl;
	bzero(choice,sizeof(choice));
	
	while(1)
	{
		SSL_read(s,choice,20);
		if(strcmp("LOGIN",choice) == 0)
			if((ret = login(s)) != NULL) 
				return ret;
		if(strcmp("REGISTER",choice) == 0)
			if((ret = user_register(s)) != NULL) 
				return ret;
	}
	return NULL;
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
	if(search(*root,node->name) != NULL)
		return 0;
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
user *search(user *root,char *name)
{
	struct user *p;
	p = root;
	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->name,name) == 0)
		{
			return p;
		}	
	}
	return NULL;
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
void printuser(user *root,SSL *ssl)
{
	struct user *q = root->next;
	char data[] = "当前在线用户列表:";
	//printf("%s\n",(*root)->next->name);
	printf("当前在线用户列表:\n");
	SSL_write(ssl,data,strlen(data));
	while(q != NULL)
	{
		if(q->ssl != NULL)
		{
			printf("%s\n",q->name);
			SSL_write(ssl,q->name,strlen(q->name));
		}
		q = q->next;
	}
}
