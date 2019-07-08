#include <stdio.h>
#include <string.h>

int getCommand(char*, char*, char*, char*);

int main(){


}


/*
 *
 * int getCommand(char* command, char* ent1, char* ent2, char* rel)
 *
 * probably the worst parser you'll ever see
 *
 * --- DESCRIPTION ---
 * this function parses the input file, one row at a time
 * returns an integer, 1 if the last line read is the 'end' command,
 * 0 otherwise. The 4 parameters are used to return the attributes needed to
 * execute the required operations:
 *
 * --- PARAMETERS ---
 * command: can assume the values 'addrel', 'addent', 'delrel', 'delent', 'report' (without quotes)
 * ent1: the first entity found, used in addrel, addent, delrel, delent
 * ent2: the second entity fount, used in addrel and delrel
 * rel: the relationship between ent1 and ent2, used in addrel and delrel
 *
 * -- RETURN VALUES ---
 * 1: if the function is parsing the last line, containing only the string 'end'
 * 0: in any other case
 */
int getCommand(char* command, char* ent1, char* ent2, char* rel) {
  int i, c = 0;

  for(i = 0; i < 6; i++)  //reading the first 6 chatacters, containing the command
    command[i]=getchar();
  command[6] = '\0';

  if(command[0] == 'e' &&   //if it encounters the last line, it cannot compare strings with
     command[1] == 'n' &&   //strcmp due to segfault, so it compares the first three characters
     command[2] == 'd')     //of the command string
     return 1;


  ent1[0] = '\0'; //assigning NULL strings to avoid garbage in the parameters
  ent2[0] = '\0';
  rel[0] = '\0';


  if(strcmp(command, "report") != 0){ //if the command is not a report, it requires at least one attribute to work
    getchar();  //dump the space and quote before the attribute
    getchar();  //not very elegant, but it works
    i = 0;
    do {
      ent1[i] = getchar();
      i++;
    } while(ent1[i-1] != '"');
    ent1[i-1] = '\0';

    if ((strcmp(command, "addrel") == 0) || (strcmp(command, "delrel") == 0)) { //if the command works on relatonships, it needs all three attributes
      getchar();
      getchar();
      i = 0;
      do {
        ent2[i] = getchar();
        i++;
      } while(ent2[i-1] != '"');
      ent2[i-1] = '\0';

      getchar();
      getchar();
      i = 0;
      do {
        rel[i] = getchar();
        i++;
      } while(rel[i-1] != '"');
      rel[i-1] = '\0';
    }
  }

  while(getchar() != '\n'){}  //dump any other character. You don't wanna remove this
  return 0;
}
