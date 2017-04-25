#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>

char *execfile;
int minsize, maxsize;
char *mindate, *maxdate;
FILE *file;
char *basename;
char *filename;
int start_errno;
int value=0;

char *display_permission ( int st_mode );

int finddir(char *folder)
{
        
	DIR *dfd;
	struct dirent *dir;
	struct stat buff;
	char *fullpath, *filepath;
	
	setlocale(LC_TIME,"ru_RU.UTF-8");

	fullpath = (char *)malloc(sizeof(char)*256);
	filepath = (char *)malloc(sizeof(char)*256);
	
	
	if ((dfd = opendir(folder)) == NULL)
	{
		fprintf(stderr, "%s : %s : %s\n", basename, strerror(errno), realpath(folder, fullpath));
		fullpath=(char*)realloc(fullpath,strlen(fullpath));
		fprintf(file, "%s : %s : %s\n", basename, strerror(errno), fullpath);
		return errno;
	}
	start_errno = errno;

	while ((dir = readdir(dfd)) != NULL)
	{
			fullpath = (char *)realloc(fullpath,sizeof(char)*256);
			filepath = (char *)realloc(filepath,sizeof(char)*256);
		
		/*if (dir->d_type == DT_LNK)
		{
			continue;
		}
		*/
		if (dir->d_type != DT_DIR)
		{
			value++;
			if (realpath(folder, filepath) != NULL)
			{
				strcat(filepath,"/");
				strcat(filepath,dir->d_name);
				filepath=(char*)realloc(filepath,strlen(filepath)+1);
			}
		else
			{
				fprintf(stderr, "%s : %s : %s\n", basename, strerror(errno), realpath(folder, fullpath));
				fullpath=(char*)realloc(fullpath,strlen(fullpath));
				return errno; 
			}
		
			
			if (stat(filepath, &buff) == 0 )
			{
				if ((buff.st_size >= minsize) && (buff.st_size <= maxsize))
				{
						printf("%s %ld %s\n",filepath, buff.st_size, display_permission(buff.st_mode));
						fprintf(file,"%s %ld %s\n",filepath, buff.st_size, display_permission(buff.st_mode));
						display_permission(buff.st_mode);
												
				}
			}
			else
			{
				fprintf(stderr, "%s : %s : %s\n", basename, strerror(errno), filepath);
				//fprintf(file, "%s : %s : %s\n", basename, strerror(errno), filepath);
				continue;
			}
		}
		else
		{
			filepath = (char *)malloc(sizeof(char)*256);
			if ((dir->d_type == DT_DIR) && (strcmp(dir->d_name, ".") != 0) && (strcmp(dir->d_name, "..") != 0))
			{
				if (realpath(folder, filepath) != NULL)
				{
					strcat(filepath,"/");
					strcat(filepath,dir->d_name);
					filepath=(char*)realloc(filepath,strlen(filepath)+1);
				}
				else
				{
					
					fprintf(stderr, "%s : %s : %s\n", basename, strerror(errno), realpath(folder, fullpath));
					fullpath=(char*)realloc(fullpath,strlen(fullpath));
					fprintf(file, "%s : %s : %s\n", basename, strerror(errno), fullpath);
					return errno; 
				}
				finddir(filepath);
			}
		}
	}
	if(start_errno != errno)
	{
		fprintf(stderr, "%s : %s : %s\n", basename, strerror(errno), filepath);
		//fprintf(file, "%s : %s : %s\n", basename, strerror(errno), filepath);

	}
	
	if (closedir(dfd) == -1)
	{
		fprintf(stderr, "%s : %s : %s\n", basename, strerror(errno), realpath(folder, fullpath));
		fullpath=(char*)realloc(fullpath,strlen(fullpath));
		//fprintf(file, "%s : %s : %s\n", basename, strerror(errno), fullpath);
		return errno; 		
	}

	free(fullpath);
	free(filepath);
} 
char *display_permission ( int st_mode )
{
	char *str = (char *)malloc(sizeof(char) * 10);
    	if (S_ISDIR(st_mode)) str[0] = 'd'; else str[0] = '-';
    	if (st_mode & S_IRUSR) str[1] = 'r'; else str[1] = '-';
	if (st_mode & S_IWUSR) str[2] = 'w'; else str[2] = '-';
	if (st_mode & S_IXUSR) str[3] = 'x'; else str[3] = '-';
	if (st_mode & S_IRGRP) str[4] = 'r'; else str[4] = '-';
 	if (st_mode & S_IWGRP) str[5] = 'w'; else str[5] = '-';
	if (st_mode & S_IXGRP) str[6] = 'x'; else str[6] = '-';
	if (st_mode & S_IROTH) str[7] = 'r'; else str[7] = '-';
	if (st_mode & S_IWOTH) str[8] = 'w'; else str[8] = '-';
	if (st_mode & S_IXOTH) str[9] = 'x'; else str[9] = '-';
	str[10] = '\0';
	return str;

}

char *realname(char *arg)
{
	int len = strlen(arg);
	int i = len-1;

	while(arg[i] != '/')
		i--;
	
	int lentemp = len - i;
	char *tempstr = malloc(sizeof(char)*(lentemp));
	int j,k = 0;
	
	i++;
	
	for(j = i; j < len; j++)
		tempstr[k++]=arg[j];
	tempstr[k]=0;

	return tempstr;	
}

int main(int argc, char *argv[]) 
{
	char *dirname;
	int file_exist = 0;

	
	basename = realname(argv[0]);
	filename=argv[4];
	file=fopen(filename,"w");
		
	minsize = atoi(argv[2]);
	maxsize = atoi(argv[3]);
	dirname = argv[1];
	
	finddir(dirname);
	printf("%d\n",value);
	
	fclose(file);
	return 0;
};
