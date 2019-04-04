#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>

#define MAX_NAME_SIZE 500

void list_dir(char* rel_path, int has_perm_write, int max_file_size)
{
	char path[MAX_NAME_SIZE];
	strcpy(path, rel_path);
	
	char abs_path[MAX_NAME_SIZE];
	char root[MAX_NAME_SIZE];
	char name[MAX_NAME_SIZE];
	
	getcwd(root, sizeof(root));
	
	sprintf(abs_path, "%s/%s", root, path);
	
	DIR* directory = opendir(abs_path);
	struct dirent* dir_entry;
	struct stat inode;
	
	if(directory == 0)
	{
		printf("Invalid directory\n");
		return;
	}
	
	printf("SUCCESS\n");
	
	dir_entry = readdir(directory);
	
	while(dir_entry != NULL)
	{
		if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)
		{
			int flag = 1;
			
			sprintf(name, "%s/%s", path, dir_entry->d_name);
			
			lstat(name, &inode);
			
			if(has_perm_write == 1)
			{
				if((inode.st_mode & S_IWUSR) == 0) flag = 0;
			}
			
			if(S_ISDIR(inode.st_mode) && flag)
			{
				if(max_file_size == -1) printf("%s\n", name);
			}
			
			else if((S_ISREG(inode.st_mode) || S_ISLNK(inode.st_mode)) && flag)
			{
				if(S_ISREG(inode.st_mode))
				{
					if(max_file_size != -1)
					{
						//check if file size is less with inode
						if(inode.st_size < max_file_size)
						{
							printf("%s\n", name);
						}
					}
					
					else printf("%s\n", name);
				}
				
				else
				{
					printf("%s\n", name);
				}
			}
		}
		
		dir_entry = readdir(directory);
	}
}

void list_rec(char* rel_path, int has_perm_write, int max_file_size)
{
	char path[MAX_NAME_SIZE];
	strcpy(path, rel_path);
	
	char abs_path[MAX_NAME_SIZE];
	char root[MAX_NAME_SIZE];
	
	getcwd(root, sizeof(root));
	
	sprintf(abs_path, "%s/%s", root, path);
	
	DIR* directory = opendir(abs_path);
	struct dirent* dir_entry;
	struct stat inode;
	
	char name[MAX_NAME_SIZE];
	
	dir_entry = readdir(directory);
	
	while(dir_entry != NULL)
	{
		if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)
		{
			int flag = 1;
			
			sprintf(name, "%s/%s", path, dir_entry->d_name);
			
			lstat(name, &inode);
			
			if(has_perm_write == 1)
			{
				if((inode.st_mode & S_IWUSR) == 0) flag = 0;
			}
			
			if(S_ISDIR(inode.st_mode))
			{
				if(flag && max_file_size == -1) printf("%s\n", name);
				
				list_rec(name, has_perm_write, max_file_size);
			}
			
			else if((S_ISREG(inode.st_mode) || S_ISLNK(inode.st_mode)) && flag)
			{
				if(S_ISREG(inode.st_mode))
				{
					if(max_file_size != -1)
					{
						//check if file size is less with inode
						if(inode.st_size < max_file_size)
						{
							printf("%s\n", name);
						}
					}
					
					else printf("%s\n", name);
				}
				
				else
				{
					printf("%s\n", name);
				}
				
			}
		}
		
		dir_entry = readdir(directory);
	}
}

int list_rec_wrapper(char* rel_path, int f1, int f2)
{
	char path[MAX_NAME_SIZE];
	strcpy(path, rel_path);
	
	char abs_path[MAX_NAME_SIZE];
	char root[MAX_NAME_SIZE];
	
	getcwd(root, sizeof(root));
	
	sprintf(abs_path, "%s/%s", root, path);
	
	DIR* directory = opendir(abs_path);
	
	if(directory == 0)
	{
		printf("Invalid directory\n");
		return -1;
	}
	
	printf("SUCCESS\n");
	list_rec(rel_path, f1, f2);
	
	closedir(directory);
	
	return 1;
}

int main(int argc, char *argv[])
{	
	if(argc == 1)
	{
		printf("Usage: ./a1 [OPTIONS] [PARAMS]\n");
	}
	
    else if(argc >= 2)
	{
		//Main selection
		
        if(strcmp(argv[1], "variant") == 0)
		{
            printf("61151\n");
        }
        
        else if(strcmp(argv[1], "list") == 0)
        {
        	int has_perm_w_flag = 0;
        	int rec_flag = 0;
        	int small_size = -1;
        	char path[MAX_NAME_SIZE];
        	
        	for(int i = 2; i < argc; i++)
        	{
        		if(strncmp(argv[i], "path=", 5) == 0)
        		{
        			strcpy(path, "");
        			strcpy(path, argv[i] + 5);
        		}
        		
        		else if(strcmp(argv[i], "recursive") == 0) rec_flag = 1;
        		else if(strncmp(argv[i], "size_smaller=", 13) == 0)
        		{
        			//set elements size for listing
        			small_size = atoi(argv[i] + 13);
        		}
        		else if(strcmp(argv[i], "has_perm_write") == 0) has_perm_w_flag = 1;
        	}
        	
        	//printf("Params: %d %d %d %s\n", has_perm_w_flag, rec_flag, small_size, path);
        	
        	if(!rec_flag)
        	{
        		//start regular listing with params
        		list_dir(path, has_perm_w_flag, small_size);
        	}
        	
        	else list_rec_wrapper(path, has_perm_w_flag, small_size);
        }
        
    }
    
    return 0;
    
}
