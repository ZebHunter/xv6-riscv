#include "kernel/types.h"
#include "user.h"

#define DEFAULT_SIZE 5

void* realloc(void* ptr, uint64 size){
    if(!ptr){
        ptr = malloc(size);
        return ptr;
    } 

    void* new_ptr;
    new_ptr = malloc(size);

    memcpy(new_ptr, ptr, size);
    free(ptr);
    return new_ptr;
}

char*
getstring(char *buf)
{
  int i = 0;
  int cc;
  char c;

  while(buf[i] != '\n'){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf = realloc(buf, (i + 1) * sizeof(c));
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf = realloc(buf, (i + 1) * sizeof(c));
  buf[i] = '\0';
  return buf;
}



int main(uint argc, char** argv[]){
    int p[2];

    char* message = malloc(DEFAULT_SIZE);

    if (pipe(p) < 0){
        exit(-1);
    }
   

    int pid = fork();

    
    if (pid < 0){
        printf("fork failed");
        exit(1);
    }

    if(pid == 0){
        read(p[0], message, sizeof(message) * 8);
        close(p[0]);
        printf("<child pid: %d>: got %s\n", getpid(), message);

        write(p[1], getstring(message), sizeof(message));
        close(p[1]);

        exit(0);
    }
    else{
        write(p[1], getstring(message), sizeof(message));
        close(p[1]);
        wait(0);
        read(p[0], message, sizeof(message));
        close(p[0]);
        printf("<parent pid: %d>: got %s\n", getpid(), message);

        exit(0);
    }
    
    exit(0);
}


