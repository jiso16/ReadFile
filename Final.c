#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#define LOGTYPE_SYSTEM  0x01  // 1
#define LOGTYPE_AUTH    (0x0001<<1) //2
#define LOGTYPE_NETWORK (0x0001<<2) //4
#define LOGTYPE_STREAM  (0x0001<<3) //8
#define LOGTYPE_CAMERA  (0x0001<<4) //16
#define LOGTYPE_CODEC   (0x0001<<5) //32

int DataCount();
int FileOpen();
int Search();
int InputCategory();
char* Time_Get();

#define MAX 150

typedef struct LogData
{
    char data[MAX];
}Log;


/*
Data Count:
-  info.log[n] 파일의 행 카운트
*/
int DataCount()
{
    char pLine;
    int count = 0;
    int max[] = {};

    /*Pointer for directory entry*/
    struct dirent *de;  
    /* opendir() returns a pointer of DIR type. */
    DIR *dr = opendir(".");  // Open Current Directory.


    if (dr == NULL)  
    {
        printf("Could not open current directory" );
        return 0;
    } 
    while ((de = readdir(dr)) != NULL)   //파일이랑 디렉토리 읽음
    {
        if (de->d_type == DT_REG)       //파일일 경우 "info.log"와 파일명 앞 여덟글자 비교
        {
            if(strncmp(de->d_name,"info.log",8)==0)  //파일명이 일치하면 파일의 행 카운트하기
            {    
                FILE *fp = fopen(de->d_name, "r"); 
                if(fp == NULL)
                {
                    printf("File Open Error\n");
                }
                pLine = fgetc(fp);
                while((pLine = fgetc(fp)) != EOF)
                {                    
                    if(pLine == '\n')
                        count++;                       
                }
                fclose(fp);
            }  
        }
    }
    return count;  //카운트 값 반환
}

/*
FileOpen
- 입력받은 시간, 카테고리, 출력 최대값 조건에 부합하는 로그데이터 하는 함수
*/
int FileOpen(Log *list,int count,int mask_, int max, char* startTime, char* endTime) 
{
    int i = 0, j = 0, k = 0, counting = 1;
    int category_count = 1;
    int count2 = 0;

    struct dirent *de;  
    char *data;
    char tmp[MAX];
    /* opendir() returns a pointer of DIR type. */
    DIR *dr = opendir(".");
    if (dr == NULL)  
    {
        printf("Could not open current directory" );
        return 0;
    } 
    while ((de = readdir(dr)) != NULL) 
    {  
       if(strncmp(de->d_name,"info.log",8)==0)
        {    
            FILE *fp = fopen(de->d_name, "r");
            if(fp == NULL)
            {
                printf("File Open Error\n");
            }
            for (; i < count; i++)
            {
                data = (char*)malloc(sizeof(char) * MAX);
                fgets(data, MAX, fp); //Save all rows to array during count value.

                //카테고리랑 시간이 일치하는 데이터만 list배열에 저장
                if((strncmp(startTime,data,21)<0 | strncmp(startTime,data,21)==0))     
                {
                    if(Search(mask_,data)==0)
                    {
                        if(strncmp(endTime,data,21)<0)   
                        {
                            break;
                        }        
                        memcpy(list[category_count].data,data, MAX);
                        //조건에 맞는 배열이 있으면 category_count값 증가
                        category_count++;        
                    }
                }

                if(feof(fp))
                {
                    break;
                }
                free(data);                  
            }
            fclose(fp);
        }  
    }

    /*Bubble Sort
     list배열에 저장된 데이터를 시간순으로 정렬
   */
    for (int j = 0; j<category_count - 1; j++)
	{
		for (k = 0; k<category_count - 1 - j; k++)
		{
			if (strncmp(list[k].data, list[k + 1].data,21) > 0)
			{
				strcpy(tmp, list[k].data);
				strcpy(list[k].data, list[k + 1].data);
				strcpy(list[k + 1].data, tmp);
			}
		}
	}
    
    category_count = category_count-1;

    //만약 category_count = 1000, max = 200, 1부터 200까지 거꾸로 출력
    if(category_count>max)
    {
        count2 = max;
        for(int i = 0; i<max; i++)
        {
            printf("[%d] %s",counting,list[count2].data);
            count2--;
            //category_count--;
            counting++;
        }
    }
    //만약 category_count = 200 and max = 1000, 1부터 200까지 거꾸로 출력
   
    else if(category_count<max)
    {
        count2 = category_count;
        for(int i = 0; i<category_count; i++)
        {
            printf("[%d]%s",counting,list[count2].data);
            count2--;
            counting++;
        }
    }    
    return 0;
}

/*
Search
- Input_Category()에서 반환한 값과 master 비트연산을 통해 입력된 카테고리 확인
*/
int Search(int mask, char* data)
{
    if((mask & LOGTYPE_SYSTEM)>0)
    {
        if(strncmp(&data[22],"System",6)==0)
        {
            return 0;
        }
    }
    if((mask & LOGTYPE_NETWORK)>0)
    {
        if(strncmp(&data[22],"Network",7)==0)
        {
            return 0;
        }
    }
    if((mask & LOGTYPE_AUTH)>0)
    {
        if(strncmp(&data[22],"Auth",4)==0)
        {
            return 0;
        }
    }
    if((mask & LOGTYPE_CAMERA)>0)
    {      
        if(strncmp(&data[22],"Camera",6)==0)
        {
            return 0;
        }
    }
    if((mask & LOGTYPE_CODEC)>0)
    {
        if(strncmp(&data[22],"Codec",5)==0)
        {
            return 0;
        }
    }
    if((mask & LOGTYPE_STREAM)>0)
    {
        if(strncmp(&data[22],"Stream",6)==0)
        {
            return 0;
        }
    }
	return -1;
}

/*
Time_Get()
- 검색하려는 시간을 입력하면 배열에 저장되어있는 시간과 비교할 수 있도록 문자열로 변환
- 변환된 문자열을 반환 함
*/
char* Time_Get()
{
    char time[30] = "";    
    char year[5] = "",day[5] = "",mon[5] = "",hour[5] = "", min[5] = "",sec[5] = "";
    char *s1 = malloc(sizeof(char)*30);
    char str[30]="";

    scanf("%s", time);

    if(sscanf(time, "%4s%2s%2s%2s%2s%2s",year, mon, day, hour, min,sec)!=6)
        printf("Fail\n");

    sprintf(str, "[%4s-%2s-%2s %2s:%2s:%2s]",year, mon, day, hour, min, sec);

    strcpy(s1,str);

    return s1;
}

/*
InputCategory
- 검색하려는 카테고리를 입력받고 입력된 순서에 따라 mask[i]값에 정수로 저장
- 카테고리 입력이 끝나면 전체 mask[i]값을 다 더해서 mask_ 값으로 저장후 반환
*/
int InputCategory()
{
    int i = 0;
    int mask[6] = {0};
    char category[50];
    char* str_p[6]={NULL,};

    char *ptr = strtok(category," ");

    char *ary []= {"System", "Network","Camera","Auth","Stream","Codec"};

    printf("Enter the Catecory:");
    gets(category);

    while(ptr != NULL)
    {
        str_p[i] = ptr;
        if(strncmp(str_p[i],"System",strlen(ary[0])) == 0)
        {
            mask[i] = 1;
        }
        else if(strncmp(str_p[i],"Auth",strlen(ary[1])) == 0)
        {
            mask[i] = 2;
        }
        else if(strncmp(str_p[i],"Network",strlen(ary[2])) == 0)
        {
            mask[i] = 4;
        }
        else if(strncmp(str_p[i],"Stream",strlen(ary[3])) == 0)
        {
            mask[i] = 8;
        }
        else if(strncmp(str_p[i],"Camera",strlen(ary[4])) == 0)
        {
            mask[i] = 16;
        }
        else if(strncmp(str_p[i],"Codec",strlen(ary[5])) == 0)
        {
            mask[i] = 32;
        }
        else
            mask[i] = 0;
        i++;
        ptr = strtok(NULL, " ");
    }
    
    int mask_ = mask[0]|mask[1]|mask[2]|mask[3]|mask[4]|mask[5];
    
    return mask_;


}
int main()
{
    Log *list;
    int count = 0;
    int max = 0;
    int mask_ = 0;

    char *startTime;
    char *endTime;
    
    printf("-----------------------------\n");

    mask_ = InputCategory();

    printf("Number of Outputs:");
    scanf("%d",&max);

    printf("Enter the Start Time (ex:YYYYMMDDhhmmss):");
    startTime = Time_Get();
    printf("Start:%s\n\n",startTime);

    printf("Enter the End Time (ex:YYYYMMDDhhmmss):");
    endTime = Time_Get();
    printf("End: %s\n\n",endTime);  

    printf("\n----------Log Data------------\n");

    count = DataCount();
    list = (Log*)malloc(sizeof(Log)*count);

    FileOpen(list, (count+1), mask_ , max, startTime, endTime);

    free(list);
    free(startTime);
    free(endTime);
}