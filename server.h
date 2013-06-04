char *buf1 = "request your name:";
char *buf2 = "name already exit!change one!";
char *buf3 = "該IP重复登录!";
char *buf4 = "登录成功!";
char *buf5 = "request your password:";
char name[20];
char passwd[20];

typedef struct user{
        char name[20];
        SSL *ssl;
        char passwd[10];
        struct user *next;
}user;

/*user information*/
void printuser(user *root);
int delete_user(user *root,char *name);
int search(user *root,char *name);
int insert(user **root,user *node);
int inituser(user **root);

/*handle message*/
void *send_data(void *arg);
void *recv_data(void *arg);
void beforechat(SSL *ssl);
pthread_t thread[MAXTHREAD];

int mount = 0;  //the number of client
int count = 0;  //count of thread
char data[MAXBUF+1];
char recvdata[MAXBUF+1];
struct user *root;

pthread_mutex_t work_mutex; //claim mutex 

SSL_CTX *ctx;
SSL *ssl;
