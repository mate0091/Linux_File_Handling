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

void list_rec_wrapper(char* rel_path, int f1, int f2)
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
		return;
	}
	
	printf("SUCCESS\n");
	list_rec(rel_path, f1, f2);
	
	closedir(directory);
	
	return;
}

void parse(char* rel_path)
{
	char path[MAX_NAME_SIZE];
	strcpy(path, rel_path);
	
	char abs_path[MAX_NAME_SIZE];
	char root[MAX_NAME_SIZE];
	
	getcwd(root, sizeof(root));
	
	sprintf(abs_path, "%s/%s", root, path);
	
	int no_sect = 0;
	int header_size = 0;
	int version = 0;
	
	int fd = open(abs_path, O_RDONLY);
	
	if(fd < 0)
	{
		printf("ERROR\nInvalid file\n");
		return;
	}
	
	char mg[2];
	strcpy(mg, "");
	
	//MAGIC
	lseek(fd, 0, SEEK_SET);
	
	read(fd, &mg, 2);
	
	if(strncmp(mg, "p4", 2) != 0)
	{
		printf("ERROR\nwrong magic\n");
		return;
	}
	
	//HEADER_SIZE
	read(fd, &header_size, 2);
	
	//VERSION
	if(read(fd, &version, 2) > 0)
	{
		if(version < 124 || version > 201)
		{
			printf("ERROR\nwrong version\n");
			return;
		}
	}
	
	//NO_SECTIONS
	if(read(fd, &no_sect, 1) > 0)
	{
		if(no_sect < 8 || no_sect > 10)
		{
			printf("ERROR\nwrong sect_nr\n");
			return;
		}
	}
	
	char sect_name[no_sect][14];
	int sect_offset[no_sect];
	int sect_size[no_sect];
	int sect_type[no_sect];
	
	for(int i = 0; i < no_sect; i++)
	{
		for(int j = 0; j < 14; j++) sect_name[i][j] = '\0';
	}
	
	for(int i = 0; i < no_sect; i++)
	{
		//read sect_name
		read(fd, &sect_name[i], 13);
		//read sect_type
		read(fd, &sect_type[i], 4);
		
		if(sect_type[i] != 71 && sect_type[i] != 60)
		{
			printf("ERROR\nwrong sect_types\n");
			return;
		}
		
		//read sect_offset
		read(fd, &sect_offset[i], 4);
		//read sect_size
		read(fd, &sect_size[i], 4);
	}
	
	printf("SUCCESS\n");
	printf("version=%d\n", version);
	printf("nr_sections=%d\n", no_sect);
	
	for(int i = 0; i < no_sect; i++)
	{
		printf("section%d: %s %d %d\n", i + 1, sect_name[i], sect_type[i], sect_size[i]);
	}
}

void extract(char* rel_path, int sect_no, int line_no)
{
	char path[MAX_NAME_SIZE];
	strcpy(path, rel_path);
	
	char abs_path[MAX_NAME_SIZE];
	char root[MAX_NAME_SIZE];
	
	getcwd(root, sizeof(root));
	
	sprintf(abs_path, "%s/%s", root, path);
	
	int fd = open(abs_path, O_RDONLY);
	if(fd == -1)
	{
		printf("ERROR\ninvalid file\n");
	}
	
	//check if invalid section
	lseek(fd, 6, SEEK_SET);
	
	int sect_cnt = 0;
	
	read(fd, &sect_cnt, 1);
	
	if(sect_no > sect_cnt)
	{
		printf("ERROR\ninvalid section\n");
		return;
	}
	
	//read section metadata from header
	int sect_offset = -1; //start
	int sect_size = -1; //length
	
	lseek(fd, 25 * (sect_no - 1) + 17, SEEK_CUR);
	
	read(fd, &sect_offset, 4);
	read(fd, &sect_size, 4);
	
	//check if line is correct
	char section[sect_size + 1];
	int sect_index = 0;
	
	lseek(fd, sect_offset, SEEK_SET);
	
	int line_count = 1;
	char buf;
	
	strcpy(section, "");
	
	for(int i = 0; i < sect_size; i++)
	{
		read(fd, &buf, 1);
		
		if(buf == '\n') line_count++;
		section[sect_index++] = buf;
	}
	
	if(line_no > line_count)
	{
		printf("ERROR\ninvalid line\n");
		return;
	}
	
	//position to line going backwards
	int current_line = 1;
	
	while(current_line < line_no)
	{
		if(section[sect_index] == '\n')
		{
			current_line++;
		}
		
		sect_index--;
	}
	
	printf("SUCCESS\n");
	
	while(section[sect_index] != '\n' && sect_index >= 0)
	{
		if(section[sect_index] != '\0') printf("%c", section[sect_index]);
		sect_index--;
	}
	
	printf("\n");
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
        int has_perm_w_flag = 0;
        int rec_flag = 0;
        int small_size = -1;
        char path[MAX_NAME_SIZE];
        int sel = -1;
        int sect_no = -1;
        int line = -1;
        	
        for(int i = 1; i < argc; i++)
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
        	
        	else if(strcmp(argv[i], "variant") == 0) sel = 0;
        	else if(strcmp(argv[i], "list") == 0) sel = 1;
        	else if(strcmp(argv[i], "parse") == 0) sel = 2;
        	else if(strcmp(argv[i], "extract") == 0) sel = 3;
        	else if(strcmp(argv[i], "findall") == 0) sel = 4;
        	else if(strncmp(argv[i], "section=", 8) == 0)
        	{
        		sect_no = atoi(argv[i] + 8);
        	}
        	else if(strncmp(argv[i], "line=", 5) == 0)
        	{
        		line = atoi(argv[i] + 5);
        	}
        }
        
        switch (sel)
        {
        	case -1:
        		printf("ERROR: invalid command\n");
        		break;
        	case 0:
        		printf("61151\n");
        		break;
        	case 1:
        		if(rec_flag == 1)
        		{
        			list_rec_wrapper(path, has_perm_w_flag, small_size);
        		}
        		
        		else
        		{
        			list_dir(path, has_perm_w_flag, small_size);
        		}
        		break;
        	case 2:
        		parse(path);
        		break;
        	case 3:
        		extract(path, sect_no, line);
        		break;
        	default:
        		break;
        }
        
    }
    
    return 0;
    
}
