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

void print_header_summary(Elf64_Ehdr *ehdr){
	char output[1024];
	

	strcpy(output,"Object file type : ");
	switch (ehdr->e_type) {
		case ET_NONE : strncat(output,"ET_NONE",strlen("ET_NONE"));
			break;
		case ET_REL : strncat(output,"ET_REL",strlen("ET_REL"));
			break;
		case ET_EXEC : strncat(output,"ET_EXEC",strlen("ET_EXEC"));
			break;
		case ET_DYN : strncat(output,"ET_DYN",strlen("ET_DYN"));
			break;
		case ET_CORE : strncat(output,"ET_CORE",strlen("ET_CORE"));
			break;
		case ET_LOPROC : strncat(output,"ET_LOPROC",strlen("ET_LOPROC"));
			break;
		case ET_HIPROC : strncat(output,"ET_HIPROC",strlen("ET_HIPROC"));
			break;
		default: strncat(output,"** INVALID **",strlen("** INVALID **"));
			break;
	} 
	puts(output);

//typedef struct
//{
//  unsigned char	e_ident[EI_NIDENT];/* Magic number and other info */
//  Elf64_Half	e_type;		/* Object file type */
//  Elf64_Half	e_machine;	/* Architecture */
//  Elf64_Word	e_version;	/* Object file version */
//  Elf64_Addr	e_entry;	/* Entry point virtual address */
//  Elf64_Off	e_phoff;	/* Program header table file offset */
//  Elf64_Off	e_shoff;	/* Section header table file offset */
//  Elf64_Word	e_flags;	/* Processor-specific flags */
//  Elf64_Half	e_ehsize;	/* ELF header size in bytes */
//  Elf64_Half	e_phentsize;	/* Program header table entry size */
//  Elf64_Half	e_phnum;	/* Program header table entry count */
//  Elf64_Half	e_shentsize;	/* Section header table entry size */
//  Elf64_Half	e_shnum;	/* Section header table entry count */
//  Elf64_Half	e_shstrndx;	/* Section header string table index */
//} Elf64_Ehdr;

}

int main(int argc, char **argv, char **envp){
	Elf_object eo;
	char *filename;
	if(argc < 2){
		printf("Usage: $s <program>\n",argv[0]);
		exit(0);
	}
	if((filename = strdup(argv[1]))==NULL) 
		die("strdup NULL progname");
			
	if((eo.mem = (uint8_t*) load_elf(filename)) == NULL){
		die("Could not load ELF file");
	}

	eo.ehdr = (Elf64_Ehdr *)  eo.mem;	
	print_header_summary(eo.ehdr);
	eo.phdr = (Elf64_Phdr *) (eo.mem + eo.ehdr->e_phoff);
	eo.shdr = (Elf64_Shdr *) (eo.mem + eo.ehdr->e_shoff);
	
	if(eo.ehdr->e_shstrndx == 0 || eo.ehdr->e_shoff == 0 || eo.ehdr->e_shnum == 0){
		printf("Section header table not found\n");
		exit(-1);
	}
	puts("End of program");	
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


