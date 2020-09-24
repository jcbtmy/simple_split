#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>

#define ERROR -1
#define FULL_PATH_SIZE (NAME_MAX * 2) + 2

//Partition Error Messages
#define PARTITION_SIZE_ERROR "Error on Partiton size"
#define PARTITION_FILE_ERROR "Error on opening file"
#define FILE_EXT_ERROR "Error on file extension not supported"
#define FILE_READ_ERROR "Error on file read"

//container for file data setup
typedef struct p_file{

	char dir_name[NAME_MAX + 1];  	//name of directory to create for file

	char* file_name; 				//filename
	char* input_path;			  	//path of to source file

	unsigned char* f_buffer;		//file_contents

	size_t f_size;				  	//size of source file
	size_t partition_scheme;		//how many partitions



} FileInfo;


void Usage(){

	printf("./split [-s(split) | -g(get)] <filename> <partitions_number>\n");
	exit(EXIT_SUCCESS);
}

int number_string(char* string){

	while(*string)
		if(!isdigit(*string++))
			return -1;

	return 0;
}

//init_FileInfo(char* filename, int partition_size)
//==================================================================
//checks the size of file against partition scheme
//checks supported extensions
//parses input file path
//loads data into fileinfo structure
//returns 0 on success | returns -1 on error(partitions too big for)


void 	_init_FileInfo(FileInfo* file, char* argv[]);

void 	_destory_FileInfo(FileInfo *file);


void 	_split_cmd(char* argv[]);

void 	_get_cmd(char* file_name);


void 	read_to_write(FILE* in, FILE* out);



int 	read_file_contents(FileInfo *file);

void 	split_file(FileInfo *file);

void 	create_file_path(char* dir_name, char* file_name, char* partition_path_name, size_t *partition);




size_t 	get_file_size(char* file_path);

size_t 	check_partition_scheme(size_t *p_size, size_t *file_size);

char* 	check_file_path(char* file_path);

FILE* 	partition_file_handler(char* file_name);

size_t 	get_chunk_size( FileInfo* file ,size_t* p_num);




void 	create_dir_name(char* file_name, char* dir_name);
	
int 	create_dir(char* dir_name );





int main(int argc, char* argv[]){

	

	if(argc < 3 || argv[1][0] != '-')
		Usage();

	switch(argv[1][1]){

		case 's':

			if(	argc < 4 || (number_string(argv[3])) || (atol(argv[3])) <= 0 )
				Usage();

			_split_cmd(argv);

			break;

		case 'g':

			_get_cmd(argv[2]);

			break;


		default:
			Usage();


	}


	return 0;
}

void 	_get_cmd(char* file_name){

		char dir_name[NAME_MAX + 1];
		char path_file_name[FULL_PATH_SIZE];
		FILE *fptr, *split_ptr;
		size_t file_counter = 0;

		if((fptr = partition_file_handler(file_name)) == NULL)
			exit(1);

		create_dir_name(file_name, dir_name);
		create_file_path(dir_name, file_name, path_file_name, &file_counter);

		for(file_counter = 1; (split_ptr = fopen(path_file_name, "rb")) != NULL ; file_counter++){

			read_to_write(fptr, split_ptr);
			create_file_path(dir_name, file_name, path_file_name, &file_counter);
			fclose(split_ptr);

		}

		fclose(fptr);


}


void 	_split_cmd(char* argv[]){

	FileInfo file;

	bzero((void*)&file, sizeof(FileInfo) );

	//check parition_size, extension, and load in file data to struct
	_init_FileInfo(&file, argv);

		//create directory for partition files, copy into into dir_name
	if( create_dir(file.dir_name) )
		exit(EXIT_FAILURE);

	if( read_file_contents(&file) )
		split_file(&file);

	_destory_FileInfo(&file);

	return;

}

void 	read_to_write(FILE* out, FILE* in){

		char* buffer;
		size_t bytes_read, file_size;

		fseek(in, 0, SEEK_END);
		file_size = ftell(in);
		fseek(in, 0, SEEK_SET);

		buffer = malloc(file_size);

		fread(buffer, sizeof(unsigned char), file_size, in);
		fwrite(buffer, sizeof(unsigned char), file_size, out);

		free(buffer);
}


void split_file(FileInfo *file){


	char partition_path_name[FULL_PATH_SIZE];
	FILE* p_file;
	size_t file_counter;
	size_t chunk;
	char* temp_buffer = file->f_buffer;


	for(file_counter = 0; file_counter < file->partition_scheme; file_counter++){

		create_file_path(file->dir_name, file->file_name, partition_path_name, &file_counter);

		if( !(p_file = partition_file_handler(partition_path_name)) ) 
			continue;

		chunk = get_chunk_size(file, &file_counter);

		fwrite(temp_buffer, sizeof(unsigned char), chunk, p_file);
		fclose(p_file);

		temp_buffer += chunk;
		file->f_size -= chunk;

	}

}

size_t get_chunk_size( FileInfo* file ,size_t* p_num){

	 return ((file->partition_scheme == (*p_num + 1 )) && (file->f_size != file->partition_scheme)) ? file->f_size : file->partition_scheme;

}

FILE* partition_file_handler(char* file_name){

	FILE* fp;

	if((fp = fopen(file_name, "wb+")) == NULL){
		printf("%s: %s\n", PARTITION_FILE_ERROR, file_name);
	}

	return fp;

}


int read_file_contents(FileInfo *file){

	FILE* fptr;
	size_t bytes_read;

	if( ( fptr = fopen( file->input_path, "rb" ) ) == NULL){

		printf("%s: %s\n", PARTITION_FILE_ERROR, file->input_path);
		return 0;
	}


	if( (bytes_read = fread( file->f_buffer, sizeof(unsigned char), file->f_size ,fptr)) < file->f_size){

		printf("Error reading %s \t %ld / %ld bytes \n", file->input_path, bytes_read, file->f_size);
		fclose(fptr);
		return -1;
	}


	fclose(fptr);

	return bytes_read;
}




void create_file_path(char* dir_name, char* file_name,char* partition_path_name, size_t *partition){

	bzero((void*)partition_path_name, FULL_PATH_SIZE);

	snprintf(partition_path_name, FULL_PATH_SIZE ,"%s/%s.%ld", dir_name, file_name, *partition );
}



int create_dir(char* dir_name){

	return mkdir(dir_name, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH )); //return the status of creating a dir
}


void _init_FileInfo(FileInfo* file, char* argv[]){

	size_t p_size = atol(argv[3]);

	//retrun -1 on file_information error
	file->input_path = argv[2];
	file->f_size = get_file_size(argv[2]);
	file->partition_scheme = check_partition_scheme( &p_size, &file->f_size);
	file->file_name = check_file_path(file->input_path);
	file->f_buffer = malloc(file->f_size + 1);

	bzero((void*) file->f_buffer, file->f_size + 1);

	create_dir_name(file->file_name, file->dir_name);

	return;

}

void _destory_FileInfo(FileInfo* file){


	if(file->f_buffer != NULL){
		free(file->f_buffer);
	}

}



void create_dir_name(char* file_name, char* dir_name){

	char *delim, *dir_name_end;

	bzero((void*)dir_name, NAME_MAX+1);

	dir_name_end = dir_name + NAME_MAX + 1;

	if(!(delim = strrchr(file_name, '.'))){

		printf("%s\n", FILE_EXT_ERROR);
		exit(1);
	}

	while( (file_name != delim) && (dir_name != dir_name_end)) //cpy buffer until end of dirname or delimeter is met
		*dir_name++ = *file_name++;

}


char* check_file_path(char* file_path){

	char* temp;

	return (temp = strrchr(file_path, '/')) ? temp++ : file_path;
}


size_t check_partition_scheme(size_t *p_size, size_t *file_size){

	if(*p_size > *file_size){

		printf("%s: %ld: %ld\n", PARTITION_SIZE_ERROR, *file_size, *p_size);
		exit(1);
	}

	return *p_size;

}


size_t get_file_size(char* file_path){

	struct stat st;

	if( (stat(file_path, &st)) < 0){

		printf("%s: %s\n", PARTITION_FILE_ERROR, file_path);
		exit(1);

	}

	return st.st_size;

}
