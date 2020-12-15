#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <elf.h>

int merge_pt_load(Elf64_Phdr *phdr,int e_phnum)
{
	  int i;
	  int num=0;
	  Elf64_Phdr *phdrtmp=phdr;
	  Elf64_Phdr *phdrnew=(Elf64_Phdr *)malloc(sizeof(Elf64_Phdr)*e_phnum);
	  memset(phdrnew,0,sizeof(Elf64_Phdr)*e_phnum);
	  int lastmergenum=-1;
	  int j=0;
    for (i = 0; i < e_phnum; i++, phdr++) {
    	
    	  if(phdr->p_type==PT_LOAD && phdr->p_offset==phdr->p_vaddr && phdr->p_vaddr==phdr->p_paddr)
    	  {
    	  	if(lastmergenum==-1)
    	  	{
    	  		(phdrnew+j)->p_type=phdr->p_type;
    	  		(phdrnew+j)->p_flags=phdr->p_flags;
    	  		(phdrnew+j)->p_offset=phdr->p_offset;
    	  		(phdrnew+j)->p_vaddr=phdr->p_vaddr;
    	  		(phdrnew+j)->p_paddr=phdr->p_paddr;
    	  		(phdrnew+j)->p_filesz=phdr->p_filesz;
    	  		(phdrnew+j)->p_memsz=phdr->p_memsz;
    	  		(phdrnew+j)->p_align=phdr->p_align;
    	  		lastmergenum =j;
    	  		j++;
    	  	}
    	  	else
    	  	{
    	  		if((phdrnew+lastmergenum)->p_offset+(phdrnew+lastmergenum)->p_filesz<=phdr->p_offset)
    	  		{
    	  			(phdrnew+lastmergenum)->p_flags=(phdrnew+lastmergenum)->p_flags|phdr->p_flags;
    	  			(phdrnew+lastmergenum)->p_filesz=phdr->p_offset+phdr->p_filesz;
    	  			(phdrnew+lastmergenum)->p_memsz=phdr->p_offset+phdr->p_memsz;
    	  		}
    	  		else
    	  		{
    	  			printf("maybe error,seq %d\n",i);
    	  			memcpy(phdrnew+j,phdr,sizeof(Elf64_Phdr));  
    	  			j++;
    	  		}    	  		
    	  	}
    	  }
    	  else
    	  {
    	  	memcpy(phdrnew+j,phdr,sizeof(Elf64_Phdr));  
    	  	j++;    	  	
    	  }
    }

		memcpy(phdrtmp,phdrnew,sizeof(Elf64_Phdr)*e_phnum);  
		free(phdrnew);
    return 0;
}

int savenewfile(char *newfilename,Elf64_Ehdr *ehdr,Elf64_Phdr *phdr, int e_phnum,Elf64_Shdr *shdr,int e_shnum,unsigned char *orignuf)
{

	FILE *fp;
	if(orignuf==NULL)
	{
		printf("orignuf null error\n");
		return 0;	
	}
	if(ehdr->e_ehsize!=sizeof(Elf64_Ehdr))
	{
		printf("Elf64_Ehdr size e_ehsize error\n");
		return 0;
	}
	if(ehdr->e_phentsize!=sizeof(Elf64_Phdr))
	{
		printf("Elf64_Phdr size e_phentsize error\n");
		return 0;
	}
  if(ehdr->e_phnum!=e_phnum)
	{
		printf("Elf64_Phdr num e_phnum error\n");
		return 0;
	}
	if(ehdr->e_shentsize!=sizeof(Elf64_Shdr))
	{
		printf("Elf64_Shdr size e_shentsize error\n");
		return 0;
	}	
	if(ehdr->e_shnum!=e_shnum)
	{
		printf("Elf64_Shdr num e_shnum error\n");
		return 0;
	}
	//只支持elfheader之后是program head，最后是section head的结构
	if(ehdr->e_shoff<ehdr->e_phoff)
	{
		printf("ehdr->e_shoff<ehdr->e_phoff error\n");
		return 0;
	
	}
	int mainsz=ehdr->e_shoff-ehdr->e_phoff-ehdr->e_phentsize*ehdr->e_phnum;
	char *mainstart=orignuf+ehdr->e_ehsize+ehdr->e_phentsize*ehdr->e_phnum;
	fp = fopen(newfilename, "wb");
	printf("savenewfile\n");
	fwrite(ehdr,1,sizeof(Elf64_Ehdr),fp);
	fwrite(phdr,1,sizeof(Elf64_Phdr)*e_phnum,fp);
	fwrite(mainstart,1,mainsz,fp);
	fwrite(shdr,1,sizeof(Elf64_Shdr)*e_shnum,fp);
	fclose(fp);
	return 1;
}

int main(int argc, char** argv){
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Shdr *shdr;
    int fd;
    FILE *fp;
    struct stat stbuf;
    char newfilename[255]={0};
		if(argc!=2)
		{
			printf("elfmodify filename\n");
			return 0;
		}
		
    fd = open(argv[1], O_RDONLY);
    assert(fd);
    fp = fdopen(fd, "rb");
    assert(fp);

    fstat(fd, &stbuf);
    if(stbuf.st_size<=0)
    {
    	printf("%s size<0",argv[1]);
    	return 0;
    }
    sprintf(newfilename,"%s.new",argv[1]);
    unsigned char buf[stbuf.st_size];
    assert(fread(buf, 1, sizeof(buf), fp) == (unsigned long)stbuf.st_size);
    fclose(fp);

    ehdr = (Elf64_Ehdr *)buf;

    phdr = (Elf64_Phdr *)(&buf[ehdr->e_phoff]);

    shdr = (Elf64_Shdr *)(&buf[ehdr->e_shoff]);
    merge_pt_load(phdr, ehdr->e_phnum);

		savenewfile(newfilename,ehdr,phdr, ehdr->e_phnum,shdr, ehdr->e_shnum,buf);
    return 0;
}
