#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>

int main(int ac, char **av)
{
	struct stat file_stat;
	int fd = open(av[1], O_RDWR);
	if (fstat(fd, &file_stat) != 0)
	{
		printf("fsatat failed\n");
		exit(1);
	}
	void *data = mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
	{
		perror("mmap failed\n");
		exit(2);
	}
	
	struct mach_header_64 *header = data;
	int64_t ncmds = header->ncmds;
	struct load_command *load_cmd;
	struct section_64 *sec_64;
	struct nlist_64 *symtab;
	struct symtab_command *symtab_cmd;
	struct segment_command_64 *segment_cmd_64; 
	
	char *strtab;
	char *symname;
	int64_t offset = sizeof(struct mach_header_64);

	for (size_t i = 0; i < ncmds; i++)
	{
		load_cmd = data + offset;
		if (load_cmd->cmd == LC_SEGMENT_64)
		{
			segment_cmd_64 = (struct segment_command_64 *)load_cmd;
			if (!strcmp(segment_cmd_64->segname, "__TEXT") || !strcmp(segment_cmd_64->segname, "__TEXT"))
			{
				printf("|%s|\n", segment_cmd_64->segname);
				//sec_64 = (struct section_64 *)data + segment_cmd_64->fileoff;
				printf("sizeof %lu\n", sizeof((struct section_64 *)&segment_cmd_64[i]));
				sec_64 = (struct section_64 *)&segment_cmd_64[i];
				for (size_t i = 0; i < segment_cmd_64->nsects; i++)
				{
					sec_64 += sec_64->size;
					printf("%llu\n", sec_64->size);
					printf("%s\n", sec_64->sectname);
				}
				
			}
		}
		if (load_cmd->cmd == LC_SYMTAB)
		{
			symtab_cmd = (struct symtab_command *)load_cmd;
			printf("size %p %p\n", (struct nlist_64*)data, (struct nlist_64*)data + 1);
			symtab = (struct nlist_64*)(data + symtab_cmd->symoff);
			strtab = (char *)data + symtab_cmd->stroff;
			for (size_t i = 0; i < symtab_cmd->nsyms; i++)
			{
				symname = strtab + (symtab + i)->n_un.n_strx;
				printf("%016llx %s\n", (symtab + i)->n_value, symname);
			}
			 
			load_cmd = (struct load_command *)load_cmd;
		}
		
		offset += load_cmd->cmdsize;
	}
	

	return (0);
}
