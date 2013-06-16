#define SIZE 50
char buf1[] = "请输入用户名";
char buf2[] = "用户名已存在";
char buf3[] = "該IP重复登录";
char buf4[] = "登录成功";
char buf5[] = "请输入密码";
char buf6[] = "用户不存在";
char buf7[] = "注册成功";
char buf8[] = "密码输入错误";

char name[SIZE];
char passwd[SIZE];
char toall[MAXBUF];
typedef struct user{
        char name[SIZE];
        SSL *ssl;
        char passwd[SIZE];
        struct user *next;
}user;

/*user information*/
void printuser(user *root,SSL *ssl);
int delete_user(user *root,char *name);
user *search(user *root,char *name);
int insert(user **root,user *node);
int inituser(user **root);

user *user_register(SSL *ssl);
user *login(SSL *ssl);
/*handle message*/
void *send_data(void *arg);
void *recv_data(void *arg);
user *beforechat(SSL **ssl);
pthread_t thread[MAXTHREAD];

int mount = 0;  //the number of client
int count = 0;  //count of thread
char data[MAXBUF+1];
char recvdata[MAXBUF+1];
struct user *root;

pthread_mutex_t work_mutex; //claim mutex 
SSL_CTX *ctx;
