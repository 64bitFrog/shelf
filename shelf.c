#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/mman.h>

typedef struct handle {
   Elf64_Ehdr *ehdr;
   Elf64_Phdr *phdr;
   Elf64_Shdr *shdr;
   uint8_t *mem;
   char *symname;
   Elf64_Addr symaddr;
   struct user_regs_struct pt_reg;
   char *exec;
}handle_t;

extern int errno;

Elf64_Addr lookup_symbol(handle_t *, const char *);
void die(char *);

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
	if(st.st_size < 2048){
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
	/*h.ehdr = (Elf64_Ehdr *) h.mem;	
	h.phdr = (Elf64_Phdr *) (h.mem + h.ehdr->e_phoff);
	h.shdr = (Elf64_Shdr *) (h.mem + h.ehdr->e_shoff);*/
	if (mem[0] != 0x7F || strncmp((char *)&mem[1], "ELF",3)){
 		printf("%s is not an ELF file\n",filename);
		return NULL; 
	}
	printf("Opened %s\n",filename);

	if(((Elf64_Ehdr *)(mem))->e_type == ET_EXEC) 
		printf("%s is of type executable (ET_EXEC)\n", filename);
	
	return mem;
}

int main(int argc, char **argv, char **envp){
	//int fd;
	handle_t h;
	//struct stat st;
	long trap, orig;
	int status, pid;
	char *args[2];
	if(argc < 2){
		printf("Usage:   $s   <program>\n",argv[0]);
		exit(0);
	}
	if((h.exec = strdup(argv[1]))==NULL) 
		die("strdup NULL progname");
			
	args[0] = h.exec;
	args[1] = NULL;
/*	if((h.symname = strdup(argv[2])) == NULL){
		die("strdup");
	}*/
	if((h.mem = (uint8_t*) load_elf(argv[1])) == NULL){
		die("Could not load ELF file");
	}
        puts("Stop good here");
	exit(0);
	h.ehdr = (Elf64_Ehdr *) h.mem;	
	h.phdr = (Elf64_Phdr *) (h.mem + h.ehdr->e_phoff);
	h.shdr = (Elf64_Shdr *) (h.mem + h.ehdr->e_shoff);
	if (h.mem[0] != 0x7F || strncmp((char *)&h.mem[1], "ELF",3)){
 		printf("%s is not an elf file\n",argv[1]);
		exit(1); 
	}
	if(h.ehdr->e_type != ET_EXEC) {
		printf("%s is not an ELF executable\n", h.exec);
		exit(1);
	}
	if(h.ehdr->e_shstrndx == 0 || h.ehdr->e_shoff == 0 || h.ehdr->e_shnum == 0){
		printf("Section header table not found\n");
		exit(-1);
	}
/*	if((h.symaddr = lookup_symbol(&h, h.symname)) == 0){
		printf("Unable to find symbol %s in executable\n", h.symname);
		exit(-1);
	}*/
	puts("so far, so good");	
}

Elf64_Addr lookup_symbol(handle_t *h, const char *symname){
   int i, j;
   char *strtab;
   Elf64_Sym *symtab;
   for (i=0; i < h->ehdr->e_shnum; i++){
      if(h->shdr[i].sh_type == SHT_SYMTAB) {
         strtab = (char *)&h->mem[h->shdr[h->shdr[i].sh_link].sh_offset];
         symtab = (Elf64_Sym *)&h->mem[h->shdr[i].sh_offset];
         for( j=0; j < h->shdr[i].sh_size/sizeof(Elf64_Sym); j++){
            if(strcmp(&strtab[symtab->st_name], symname) ==0)
               return (symtab->st_value);
            symtab++;
         }
      }
   }
   return 0;
}

void die(char* err){
	perror(err);
	exit(-1);
}


