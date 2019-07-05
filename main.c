#include <stdio.h>
#include <string.h>

void read(char*);

int main(){


}


/*
 * void read(char* buffer)
 * reads the next line from the input command file
 * and stores it in the *buffer parameter
*/
void read(char* buffer){
  int i, c = 0;
  while(c=getchar()){
    if (c == '\n') break;
    buffer[i] = c;
    i++;
  }
  buffer[i] = '\0';
}
