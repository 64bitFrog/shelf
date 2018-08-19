#include <stdio.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>

#include "shelfio.h"

uint8_t *load_elf(char *filename){
	int fd;
	struct stat st;
	uint8_t *mem;
	if((fd = open(filename, O_RDONLY)) < 0) {
		printf("Could not open %s\n",filename);
		return NULL;
	}
	
	if(fstat(fd, &st) < 0) {
		printf("Could not stat %s\n",filename);
		close(fd);
		return NULL;
	}
	if(st.st_size < 1024){ /*arbitray size limit */
        	printf("Not trusting file %s, only %d bytes\n",filename,st.st_size);
		return NULL;        
	}			
	mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mem == MAP_FAILED){
		printf("Could not mmap %s\n",filename);
		close(fd);
		return NULL;
	}
	close(fd);
	
	if (mem[0] != 0x7F || strncmp((char *)&mem[1], "ELF",3)){
 		printf("%s is not an ELF file\n",filename);
		return NULL; 
	}
	printf("Opened %s\n",filename);

	if(((Elf64_Ehdr *)(mem))->e_type == ET_EXEC) 
		printf("%s is of type executable (ET_EXEC)\n", filename);
	
	return mem;
}
