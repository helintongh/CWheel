


/*
 使用sqlite数据库来完成学生成绩管理系统
*/



#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>    //包含数据库sqlite3 的头文件
#include <string.h>

struct student
{
	int stuno;
	char name[50];
	int age;
	char major[50];
	float score;
};

typedef struct student student_t;


sqlite3 *mydb = NULL;



//删除指定姓名的学生信息


int del_student_record(char *name)
{
	char sqlcmd[100] ={0};
	int iret = 0;
	char *errmsg = NULL;

	sprintf(sqlcmd,"delete from stuinfo where name=\"%s\"",name);

	printf("sql: %s\n", sqlcmd);

	iret = sqlite3_exec(mydb,sqlcmd,NULL,NULL, &errmsg);

	if(iret != SQLITE_OK)
	{
	 	  printf("sqlite3_exec error :%s\n",errmsg);

			return -1;
	}

	printf("delete  ok\n");

	return 0;
}


//根据学生姓名查找学生信息

//根据成绩从高分到低分进行排序


//修改指定学生的分数
/*
name : 要修改学生姓名的信息

*/

int update_student_record(char *name, float newScore)
{
	char sqlcmd[100] ={0};
	int iret = 0;
	char *errmsg = NULL;

	sprintf(sqlcmd,"update stuinfo set score=%f where name=\"%s\"",newScore,name);

	printf("sql: %s\n", sqlcmd);

	iret = sqlite3_exec(mydb,sqlcmd,NULL,NULL, &errmsg);

	if(iret != SQLITE_OK)
	{
	 	  printf("sqlite3_exec error :%s\n",errmsg);

			return -1;
	}

	printf("update  ok\n");

	return 0;
}




//定义回调函数mycallback

int mycallback(void *para, int f_num, char **f_value, char **f_name)
{
	 int i = 0;

	 for(i = 0;i< f_num; i++)
	 {
	 	  printf("%s  ", f_value[i]);
	 }

	 printf("\n");

	
	 return 0;
}



//数据信息的查询 select * from stuinfo

int query_all_student_record(void)
{
	  char sqlcmd[100] ={0};
		int iret = 0;
		char *errmsg = NULL;

		sprintf(sqlcmd,"select * from stuinfo");

		printf("sql: %s\n", sqlcmd);

	 iret = sqlite3_exec(mydb,sqlcmd,mycallback,NULL, &errmsg);

	 if(iret != SQLITE_OK)
	 {
	 	  printf("sqlite3_exec error :%s\n",errmsg);

			return -1;
	 }

	 return 0;		
		
}




int query_all_student_recordx(char ***result)
{
	  char sqlcmd[100] ={0};
		int iret = 0;
		char *errmsg = NULL;


		int row = 0;
		int col = 0;
		

		sprintf(sqlcmd,"select * from stuinfo");

		printf("sql: %s\n", sqlcmd);

	 iret = sqlite3_get_table(mydb,sqlcmd,result,&row, &col,&errmsg);

	 if(iret != SQLITE_OK)
	 {
	 	  printf("sqlite3_exec error :%s\n",errmsg);

			return -1;
	 }

	 printf("row:%d  col:%d\n", row,col);

	 return row;		
		
}



/*
  函数功能: 往学生信息表中，增加一条学生信息记录
  参数:
       db 数据库指针
       student_t *pstu ; 学生信息结构体

  返回值: 成功，返回 0,失败返回 -1
  
*/


int add_student_record(sqlite3 *db, student_t *pstu)
{
		char sqlcmd[100] ={0};
		int iret = 0;
		char *errmsg = NULL;


		sprintf(sqlcmd,"insert into stuinfo values(%d,\"%s\",%d,\"%s\",%f)",
		pstu->stuno, pstu->name,pstu->age,pstu->major,pstu->score);

	  printf("sql: %s\n", sqlcmd);

	  iret = sqlite3_exec(db,sqlcmd,NULL,NULL, &errmsg);

	 if(iret != SQLITE_OK)
	 {
	 	  printf("sqlite3_exec error :%s\n",errmsg);

			return -1;
	 }

	 return 0;		
	   
}


void input_stu_info(void)
{
  	student_t stu;

	while(1)
	{
		printf("input stuno:");
		scanf("%d", &stu.stuno);
		while(getchar()!='\n');

		if(stu.stuno == 0) 
		{
			 break;
		}

		printf("input name:");
		scanf("%s", stu.name);
		while(getchar()!='\n');

		printf("input age:");
		scanf("%d", &stu.age);
		while(getchar()!='\n');

		printf("input major:");
		scanf("%s", stu.major);
		while(getchar()!='\n');

		printf("input score:");
		scanf("%f", &stu.score);
		while(getchar()!='\n');

		add_student_record(mydb,&stu);		
	}
}



void show_menu(void)
{
	 printf("-------------------------\n");
	 printf("1 add student record!\n");
	 printf("2 delete student record!\n");
	 printf("3 show student record with callback!\n");
	 printf("4 exit!\n");
	 printf("5 display student info no callback\n");
	 printf("6 update student score\n");
	 printf("-------------------------\n");
	 printf("<input>:");
	 
}



int main(int argc, char *argv[])
{


	 char *errmsg = NULL;

	 int iret = 0;
	 int user_input = 0;

	 char **record;
	 int i = 0;
	 int j = 0;

	 int record_num = 0;
	 char stuName[50]={0};
	 float newScore = 0;
	 
	 iret = sqlite3_open("student.db", &mydb);
	 if(iret != 0)
	 {
	 		printf("open error:%s\n", sqlite3_errmsg(mydb));

		 	return 0;
	 }

	 printf("open database OK!!!\n");

	 while(1)
	 {
			show_menu();
			scanf("%d", &user_input);

			switch(user_input)
			{
				case 1:
					    input_stu_info();
							break;
				case 2:
						//删除学生信息
						memset(stuName,0,50);
						printf("input delete student Name:");
						scanf("%s",stuName);
						del_student_record(stuName);
						
						  break;
				case 3:
						   //查询学生信息
					   query_all_student_record();
						 break;
				case 4:
					  goto out;
				case 5:
					record_num = query_all_student_recordx(&record);

					for(i = 0; i< 5;i++)
					{
						 printf("%s\n", record[i]);
					}

					for(i = 1; i <= record_num; i++)
					{
						  for(j = 0; j< 5;j++)
						  {
								 printf("%s  ", record[(i*5+j)]);
						  }
						  printf("\n");
					}
					
					break;

				case 6:
					//更新学生分数
					memset(stuName,0,50);
					printf("input update student Name and score:");
					scanf("%s%f",stuName,&newScore);

					update_student_record(stuName, newScore);
					break;
					
				default:
					   break;
					
			}
	

	 }


out:
	 
	 sqlite3_close(mydb); 
	 
	 printf("close database OK!!!\n");

	 return  0 ;

	 
}


