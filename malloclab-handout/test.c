# include <sys/mman.h>
# include "csapp.h"

void mmapcopy(int fd, size_t file_len){

    FILE *mapped_file;

    mapped_file = Mmap(NULL, file_len, PROT_READ, MAP_PRIVATE, fd, 0);

    Write(1, mapped_file, file_len);
}

int main(int argc, char *argv[]){

    int fd;
    struct stat st;
    if (argc < 2){
        printf("Usage a.out <filename>\n");
        exit(1);
    }

    fd = Open(argv[1], O_RDONLY, S_IREAD);
    Fstat(fd, &st);

    size_t file_len = st.st_size;
    mmapcopy(fd, file_len);

}