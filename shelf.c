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

#include "shelfio.h"

typedef struct _Elf_object {
   Elf64_Ehdr *ehdr;
   Elf64_Phdr *phdr;
   Elf64_Shdr *shdr;
   uint8_t *mem;
   char *symname;
   Elf64_Addr symaddr;
   struct user_regs_struct pt_reg;
   char *exec;
} Elf_object;

extern int errno;

Elf64_Addr lookup_symbol(Elf_object *, const char *);
void die(char *);


int main(int argc, char **argv, char **envp){
	Elf_object h;
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

Elf64_Addr lookup_symbol(Elf_object *elf, const char *symname){
   int i, j;
   char *strtab;
   Elf64_Sym *symtab;
   for (i=0; i < elf->ehdr->e_shnum; i++){
      if(elf->shdr[i].sh_type == SHT_SYMTAB) {
         strtab = (char *)&elf->mem[elf->shdr[elf->shdr[i].sh_link].sh_offset];
         symtab = (Elf64_Sym *)&elf->mem[elf->shdr[i].sh_offset];
         for( j=0; j < elf->shdr[i].sh_size/sizeof(Elf64_Sym); j++){
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


