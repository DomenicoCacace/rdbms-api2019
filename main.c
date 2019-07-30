#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- GLOBAL VARIABLES AND CONSTANTS ---

//TODO: test different prime numbers to find the best ones
#define HASH_SIZE_ENT 113
#define HASH_MULTIPLIER 51
#define  MAX_STRING_SIZE 100

// --- DATA TYPES DEFINITIONS ---

typedef struct _entity {
    char name[MAX_STRING_SIZE];
    struct _entity *next;
}t_entity;

typedef struct _entityAddr {
    t_entity *address;
}t_entityAddr;

t_entityAddr entityTable[HASH_SIZE_ENT];
// --- FUNCTIONS PROTOTYPES ---
int getCommand(char*, char*, char*, char*);
void executeCommand(char*, char*, char*, char*);

void addEntity(char*);
void deleteEntity(char*);
void addRelation(char*, char*, char*);
void deleteRelation(char*, char*, char*);
void printReport(void);

void addToEntList(t_entity*, char*);
void delFromEntList(t_entityAddr*, char*);

unsigned int hash(char*, int, int);

int main(){
    char command[7],
            entName1[MAX_STRING_SIZE],
            entName2[MAX_STRING_SIZE],
            relName[MAX_STRING_SIZE];

    freopen("TestCases/1_Monotone/batch1.2.in", "r", stdin);      //redirecting standard input, used for debugging in CLion

    while(getCommand(command, entName1, entName2, relName) != 1) {
        executeCommand(command, entName1, entName2, relName);
    }
    return 0;
}


/*
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
 * --- RETURN VALUES ---
 * 1: if the function is parsing the last line, containing only the string 'end'
 * 0: in any other case
 */
int getCommand(char *command, char *ent1, char *ent2, char *rel) {
  int i, c = 0;

  for(i = 0; i < 6; i++)  //reading the first 6 chatacters, containing the command
    command[i] = (char)getchar();
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

/*
 * unsigned int hash(char* string, int mult, int mod)
 *
 * just a hash function that hopefully won't need more space than this
 *
 * --- DESCRIPTION ---
 * this is a basic string hashing function, that sums the ASCII values of every single
 * character in the string, each multiplied by the i-th power of a multiplier, where i is the
 * position of the char in the string. Both the multiplayer and the modulus must be prime to
 * ensure an acceptable collision rate, on average
 *
 * --- PARAMETERS ---
 * string: the input string to be analyzed and hashed
 * mult: the multiplier, see above
 * mod: the divider, see below
 *
 * --- RETURN VALUES ---
 * the function returns the remaindr of the division of the weighted sum of the ASCII values and
 * the divider
 */

unsigned int hash(char* string, int mult, int mod) {
    unsigned int result = string[0] - '_';
    int i = 1;
    while(string[i] != '\0') {
        //result = mult * (string[i] - '_' + result); //probably exceeds the maximum integer size, should be tested on large strings
        result = (mult * (string[i] - '_' + result) % mod);
        i++;
    }
    return result /*% mod*/;
}

/*
 * void executeCommand(char* command, char* ent1, char* ent2, char* rel)
 *
 * --- DESCRIPTION ---
 * executes different commands based on the request, called in the main for every input line parsed
 * not everytime all the parameters are used, it depends on the requested command
 *
 * --- PARAMETERS ---
 * command: a 6-char-long string containing the command name, as parsed from the file
 * ent1: the first entity name
 * ent2: the second entity name
 * rel: the relation name
 *
 * --- RETURN VALUES ---
 * none
 */
void executeCommand(char* command, char* ent1, char* ent2, char* rel) {
    if (strcmp(command, "addent") == 0) {
        addEntity(ent1);
        return;
    } else if (strcmp(command, "addrel") == 0) {
        addRelation(ent1, ent2, rel);
        return;
    } else if (strcmp(command, "delent") == 0) {
        deleteEntity(ent1);
        return;
    } else if (strcmp(command, "delrel") == 0) {
        deleteRelation(ent1, ent2, rel);
        return;
    } else if (strcmp(command, "report") == 0) {
        printReport();
        return;
    }
}

/*
 * adds a new entity to the table, if absent
 */
void addEntity(char* entName) {
    unsigned int hashValue = hash(entName, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *temp = entityTable[hashValue].address;

    while (temp != NULL) {
        if (strcmp(temp->name, entName) != 0)    //element already exists
            return;
        temp = temp->next;
    }

    t_entity *newEnt = (t_entity*)malloc(sizeof(t_entity));
    strcpy(newEnt->name, entName);
    newEnt->next = entityTable[hashValue].address;
    entityTable[hashValue].address = newEnt;
}

void deleteEntity(char* entName){
    unsigned int hashValue = hash(entName, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *temp = entityTable[hashValue].address;

    while (temp != NULL) {
        if (strcmp(temp->name, entName) != 0) {    //element exists
            strcpy(temp->name, "\0");
            return;
        }
        temp = temp->next;
    }
}

void addRelation(char* orig, char* dest, char* relName) {

}

void deleteRelation(char* orig, char* dest, char* name) {

}

void printReport(void) {

}
