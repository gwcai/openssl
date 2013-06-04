#include<stdio.h>
#include"user.h"

int mount = 0;
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
/*
int main()
{
	struct user *root = NULL;
	struct user *node;

	inituser(&root);  //初始化

	int i = 5;
	char name[20];
	while(i>0)
	{
		node = (struct user*)malloc(sizeof(struct user));
		memset(node,0,sizeof(struct user));
		scanf("%s",node->name);
		node->next = NULL;
		insert(&root,node);
		i--;
	}
	if(search(root,"hello"))
	  printf("YES\n");
  	else printf("NO\n");
  	if(delete_user(root,"hello"))
	  printf("sucess\n");
  	else printf("failed\n");
	printuser(root);
}*/
