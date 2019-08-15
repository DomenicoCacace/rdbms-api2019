// --- LIBRARIES ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// --- CONSTANTS ---

#define HASH_SIZE_ENT 271
#define HASH_SIZE_REL 271
#define HASH_MULTIPLIER 31
#define  MAX_STRING_SIZE 100


// --- DATA TYPES DEFINITIONS ---

typedef struct _entity {
    char name[MAX_STRING_SIZE];
    unsigned short int version;   //odd if valid, even otherwise
    struct _entity *next;
} t_entity, *t_entityAddr;

typedef struct _senderList {
    t_entity *address;
    unsigned short int version;
    struct _senderList *next;
} t_senderList;

typedef struct _relInstance {
    struct _relInstance *rightChild;
    struct _relInstance *leftChild;
    int height;
    //may need to add father and balancing attributes

    t_entity *recipient;
    int numSenders;
    unsigned short int recVersion;
    t_senderList *senderList;
} t_relInstance;

typedef struct _relation {
    char name[MAX_STRING_SIZE];
    t_relInstance *root;
    struct _relation *next;
} t_relation, *t_relAddr;

typedef struct _relationList {
    t_relation *relationAddr;
    t_senderList *senderList;
    int counter;
    struct _relationList *next;
} t_relationList;


// --- GLOBAL VARIABLES ---

t_entityAddr entityTable[HASH_SIZE_ENT];
t_relAddr relTable[HASH_SIZE_REL];
t_relationList *queue;
int recalcMaxSenders;

// --- FUNCTIONS PROTOTYPES ---

//Command parsing and execution
int getCommand(char*, char*, char*, char*);
void executeCommand(char*, char*, char*, char*);
void addEntity(char*);
void deleteEntity(char*);
void addRelation(char*, char*, char*);
void deleteRelation(char*, char*, char*);
void printReport(void);

//Relation insertion and deletion
t_entity *getEntityAddr(t_entity*, char*);
t_relInstance *addRelationInstance(t_relInstance *node, t_entity *sender, t_entity *recipient);
void delRelationInstance(t_relInstance *node, t_entity *sender, t_entity *recipient);

//Queue management
void push(t_relationList*, t_entity*);
t_senderList *pop(t_relationList*);
void emptyQueue(t_relationList*);

//Report printing and support
void getMaxReceivers(t_relationList *relList, t_relInstance *node, int *currMax);
int countInstanceSenders(t_relInstance*);
void assemblePrintQueues();
void printOrderedLists();

//AVL support
int getBalance(t_relInstance*);
int getHeight(t_relInstance*);

// AVL rotation
t_relInstance *doubleRotateLeft(t_relInstance*);
t_relInstance *doubleRotateRight(t_relInstance*);
t_relInstance *rotateLeft(t_relInstance*);
t_relInstance *rotateRight(t_relInstance*);

//misc
int max(int, int);
unsigned int hash(const char*, int, int);


int main(){
    recalcMaxSenders = 1;
    queue = NULL;
    char command[7],
            entName1[MAX_STRING_SIZE],
            entName2[MAX_STRING_SIZE],
            relName[MAX_STRING_SIZE];
    //freopen("TestCases/2_Dropoff/batch2.2.in", "r", stdin);      //redirecting standard input, used for debugging in CLion

    while(getCommand(command, entName1, entName2, relName) != 1) {
        executeCommand(command, entName1, entName2, relName);
    }
    return 0;
}



// --- FUNCTIONS IMPLEMENTATION ---

//Command parsing and execution

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
    int i = 0;

    for(i = 0; i < 6; i++)  //reading the first 6 chatacters, containing the command
        command[i] = (char)getchar_unlocked();
    command[6] = '\0';

    if(command[0] == 'e' &&   //if it encounters the last line, it cannot compare strings with
       command[1] == 'n' &&   //strcmp due to segfault, so it compares the first three characters
       command[2] == 'd')     //of the command string
        return 1;


    ent1[0] = '\0'; //assigning NULL strings to avoid garbage in the parameters
    ent2[0] = '\0';
    rel[0] = '\0';


    if(strcmp(command, "report") != 0){ //if the command is not a report, it requires at least one attribute to work
        getchar_unlocked();  //dump the space and quote before the attribute
        getchar_unlocked();  //not very elegant, but it works
        i = 1;
        ent1[0] = '"';
        do {
            ent1[i] = (char)getchar_unlocked();
            i++;
        } while(ent1[i-1] != '"');
        ent1[i-1] = '"';
        ent1[i] = ' ';
        ent1[i+1] = '\0';

        if ((strcmp(command, "addrel") == 0) || (strcmp(command, "delrel") == 0)) { //if the command works on relatonships, it needs all three attributes
            getchar_unlocked();
            getchar_unlocked();
            i = 1;
            ent2[0] = '"';
            do {
                ent2[i] = (char)getchar_unlocked();
                i++;
            } while(ent2[i-1] != '"');
            ent2[i-1] = '"';
            ent2[i] = ' ';
            ent2[i+1] = '\0';

            getchar_unlocked();
            getchar_unlocked();
            i = 1;
            rel[0] = '"';
            do {
                rel[i] = (char)getchar_unlocked();
                i++;
            } while(rel[i-1] != '"');
            rel[i-1] = '"';
            rel[i] = ' ';
            rel[i+1] = '\0';
        }
    }

    while((char)getchar_unlocked() != '\n'){}  //dump any other character. You don't wanna remove this
    return 0;
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
    unsigned long hashValue = hash(entName, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *temp = entityTable[hashValue];

    while (temp != NULL) {
        if (strcmp(temp->name, entName) == 0) {   //element already exists
            if (temp->version % 2 != 0)
                temp->version++;
            return;
        }
        temp = temp->next;
    }

    t_entity *newEnt = (t_entity*)malloc(sizeof(t_entity));
    strcpy(newEnt->name, entName);
    newEnt->version = 0;
    newEnt->next = entityTable[hashValue];
    entityTable[hashValue] = newEnt;
}
/*
 * looks for an entity. If found, it marks it as deleted changing i
 * its name to "\0", to avoid possible collisions with the other entities names
 */
void deleteEntity(char* entName){
    unsigned long hashValue = hash(entName, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *temp = entityTable[hashValue];

    while (temp != NULL) {
        if (strcmp(temp->name, entName) == 0) {    //element exists
            if(temp->version % 2 == 0)
                temp->version++;
            recalcMaxSenders = 1;
            return;
        }
        temp = temp->next;
    }
}

void addRelation(char* orig, char* dest, char* relName) {
    unsigned long hashValue;

    hashValue = hash(orig, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *senderAddr = getEntityAddr(entityTable[hashValue], orig);
    hashValue = hash(dest, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *recipientAddr = getEntityAddr(entityTable[hashValue], dest);

    if(senderAddr != NULL && recipientAddr != NULL) {   //checks if the entities have been created
        if (senderAddr->version % 2 == 0 &&
            recipientAddr->version % 2 == 0) {    //checks if the entities have not been deleted

            hashValue = hash(relName, HASH_MULTIPLIER, HASH_SIZE_REL);
            t_relation *temp = relTable[hashValue];

            while (temp != NULL) {
                if (strcmp(temp->name, relName) == 0) {   //element already exists
                    temp->root = addRelationInstance(temp->root, senderAddr, recipientAddr);
                    return;
                }
                temp = temp->next;
            }

            //adds a new relation type in the hash table
            t_relation *newRel = (t_relation*)malloc(sizeof(t_relation));
            strcpy(newRel->name, relName);
            newRel->next = relTable[hashValue];
            relTable[hashValue] = newRel;
            newRel->root = NULL;
            newRel->root = addRelationInstance(newRel->root, senderAddr, recipientAddr);
        }
    }
}

void deleteRelation(char* orig, char* dest, char* relName) {
    unsigned long hashValue;

    hashValue= hash(orig, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *senderAddr = getEntityAddr(entityTable[hashValue], orig);
    hashValue = hash(dest, HASH_MULTIPLIER, HASH_SIZE_ENT);
    t_entity *recipientAddr = getEntityAddr(entityTable[hashValue], dest);

    if(senderAddr != NULL && recipientAddr != NULL) {   //checks if the entities have been created

        hashValue = hash(relName, HASH_MULTIPLIER, HASH_SIZE_REL);
        t_relation *temp = relTable[hashValue];

        while (temp != NULL) {
            if (strcmp(temp->name, relName) == 0) {   //element exists
                delRelationInstance(temp->root, senderAddr, recipientAddr);
                return;
            }
            temp = temp->next;
        }
    }
}

void printReport(void) {
    assemblePrintQueues();
    printOrderedLists();
    recalcMaxSenders = 0;
}


//Relation insertion and deletion

t_entity *getEntityAddr(t_entity *source, char *entName) {
    t_entity *temp = source;

    while (temp != NULL) {
        if (strcmp(temp->name, entName) == 0)    //element already exists
            return temp;
        temp = temp->next;
    }
    return NULL;
}

t_relInstance *addRelationInstance(t_relInstance *node, t_entity *sender, t_entity *recipient) {
    if (node == NULL) { //the node doesn't exist
        t_relInstance *newNode = (t_relInstance*)malloc(sizeof(t_relInstance));
        newNode->rightChild = NULL;
        newNode->leftChild = NULL;
        newNode->recipient = recipient;
        newNode->height = 0;
        newNode->numSenders = 1;
        newNode->recVersion = recipient->version;

        t_senderList *newListNode = (t_senderList*)malloc(sizeof(t_senderList));
        newListNode->address = sender;
        newListNode->version = sender->version;
        newListNode->next = NULL;
        newNode->senderList = newListNode;

        return newNode;

    } else if(strcmp(recipient->name, node->recipient->name) < 0) {
        node->leftChild = addRelationInstance(node->leftChild, sender, recipient);
        if(getBalance(node) > 1) {
            if(strcmp(node->recipient->name, node->leftChild->recipient->name) < 0)
                node = rotateLeft(node);
            else
                node = doubleRotateLeft(node);
        }
    }
    else if (strcmp(recipient->name, node->recipient->name) > 0) {
        node->rightChild = addRelationInstance(node->rightChild, sender, recipient);
        if (getBalance(node) > 1) {
            if (strcmp(node->recipient->name, node->leftChild->recipient->name) < 0)
                node = rotateRight(node);
            else
                node = doubleRotateRight(node);
        }
    }
    else {
        if (node->recVersion < recipient->version) {
            t_senderList *temp = node->senderList;
            while (temp != NULL) {
                temp = temp->next;
                free (node->senderList);
                node->senderList = temp;
            }
            node->numSenders = 0;
        }
        t_senderList *temp = node->senderList;
       node->recVersion = recipient->version;
        while (temp != NULL) {
            if (temp->address == sender) {
                if (sender->version > temp->version) {
                    temp->version = sender->version;
                    temp->address->version = sender->version;
                    node->numSenders++;
                }
                return node;
            }
            temp = temp->next;
        }
        t_senderList *newSender = (t_senderList*)malloc(sizeof(t_senderList));
        newSender->address = sender;
        newSender->version = sender->version;
        newSender->next = node->senderList;
        node->senderList = newSender;
        node->numSenders++;
        return node;
    }

    node->height =  max(getHeight(node->rightChild), getHeight(node->leftChild));
    return node;
}

void delRelationInstance(t_relInstance *node, t_entity *sender, t_entity *recipient) {
    if (node == NULL) {
        return;
    }
    else if (node->recipient == recipient) {
        t_senderList *curr = node->senderList;
        if (node->senderList == NULL) {
            return;
        }
        if(node->senderList->address == sender) {
            if (node->senderList->version == sender->version && sender->version % 2 == 0)
                node->numSenders--;
            node->senderList = curr->next;
            free(curr);
            return;
        }
        t_senderList *prev = curr;
        curr = curr->next;

        while(curr != NULL) {
            if(curr->address == sender) {
                if (curr->version == sender->version && sender->version % 2 == 0)
                    node->numSenders--;
                prev->next = curr->next;
                free(curr);
                return;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    else {
        if (strcmp(recipient->name, node->recipient->name) < 0)
            delRelationInstance(node->leftChild, sender, recipient);
        else if (strcmp(recipient->name, node->recipient->name) > 0)
            delRelationInstance(node->rightChild, sender, recipient);
    }
}

//Queue management

void push(t_relationList *relList, t_entity *newEntity) {
    t_senderList *newItem = (t_senderList*)malloc(sizeof(t_senderList));
    newItem->address = newEntity;

    newItem->next = relList->senderList;
    relList->senderList = newItem;
}

t_senderList *pop(t_relationList *relList) {
    if (relList->senderList == NULL)
        return NULL;
    t_senderList *temp = relList->senderList;
    relList->senderList = relList->senderList->next;
    return temp;
}

void emptyQueue(t_relationList *relList) {
    if (relList->senderList == NULL)
        return;
    t_senderList *temp = pop(relList);
    while(temp != NULL) {
        free(temp);
        temp = pop(relList);
    }
}


//Report printing and support

void getMaxReceivers(t_relationList *relList, t_relInstance *node, int *currMax) {
    if (node != NULL) {
        getMaxReceivers(relList, node->rightChild, currMax);

        if(node->recVersion == node->recipient->version /*&& node->recVersion % 2 == 0*/) {
            int tempCount = countInstanceSenders(node);
            if(tempCount > 0) {
                if (tempCount == *(currMax)) {
                    push(relList, node->recipient);
                } else if (tempCount > *(currMax)) {
                    *(currMax) = tempCount;
                    emptyQueue(relList);
                    push(relList, node->recipient);
                }
            }
        }

        getMaxReceivers(relList, node->leftChild, currMax);
    }
}

int countInstanceSenders(t_relInstance *node) {
    if (recalcMaxSenders != 1)
        return node->numSenders;
    t_senderList *temp = node->senderList;
    int counter = 0;

    while(temp != NULL) {
        if(temp->version == temp->address->version && temp->version % 2 == 0) {
            counter++;
        }
        temp = temp->next;
    }
    node->numSenders = counter;
    return counter;
}

void assemblePrintQueues() {
    t_relation *temp;
    for(int i = 0; i < HASH_SIZE_REL; i++) {
        temp = relTable[i];
        while (temp != NULL) {
            t_relationList *newNode = (t_relationList*)malloc(sizeof(t_relationList));
            newNode->relationAddr = temp;
            newNode->counter = 0;
            newNode->senderList = NULL;
            newNode->next = NULL;
            getMaxReceivers(newNode, temp->root, &(newNode->counter));

            t_relationList *prev, *curr;

            if (queue != NULL) {
                prev = NULL;
                curr = queue;
                while (curr != NULL && strcmp(newNode->relationAddr->name, curr->relationAddr->name) > 0) {
                    prev = curr;
                    curr = curr->next;
                }
                if (prev != NULL) {
                    prev->next = newNode;
                    newNode->next = curr;
                }
                else {
                    newNode->next = queue;
                    queue = newNode;
                }
            }
            else {
                queue = newNode;
            }
            temp = temp->next;
        }
    }
}

void printOrderedLists() {
    int checker = 0;
    t_relationList *temp = queue;
    t_senderList *senders;

    while (temp != NULL) {
        if (temp->counter > 0) {
            fputs(temp->relationAddr->name, stdout);
            senders = temp->senderList;
            while (senders != NULL) {
                fputs(senders->address->name, stdout);
                senders = senders->next;
                free(temp->senderList);
                temp->senderList = senders;
                checker++;
            }
            printf("%d; ", temp->counter);
        }

        temp = temp->next;
        free(queue);
        queue = temp;
    }
    if (checker == 0)
        fputs("none", stdout);
    printf("\n");

}


//AVL support

int getBalance(t_relInstance *node) {
    if (node == NULL)
        return 0;
    return getHeight(node->leftChild) - getHeight(node->rightChild);
}

int getHeight(t_relInstance *node) {
    if (node == NULL)
        return -1;
    return node->height;
}


//AVL rotation

t_relInstance *doubleRotateLeft(t_relInstance *node) {
    node->leftChild = rotateRight(node->leftChild);
    return rotateLeft(node);
}

t_relInstance *doubleRotateRight(t_relInstance *node) {
    node->rightChild = rotateLeft(node->rightChild);
    return rotateRight(node);
}

t_relInstance *rotateLeft(t_relInstance *node) {
    t_relInstance *temp = node->leftChild;
    node->leftChild = temp->rightChild;
    temp->rightChild = node;

    node->height = max(getHeight(node->leftChild), getHeight(node->rightChild)) + 1;
    temp->height = max(getHeight(temp->leftChild), getHeight(node->rightChild)) + 1;

    return temp;
}

t_relInstance *rotateRight(t_relInstance *node) {
    t_relInstance *temp = node->rightChild;
    node->rightChild = temp->leftChild;
    temp->leftChild = node;

    node->height = max(getHeight(node->leftChild), getHeight(node->rightChild)) + 1;
    temp->height = max(getHeight(temp->leftChild), getHeight(node->rightChild)) + 1;
    return temp;
}


//misc

int max(int x, int y) {
    if (x > y)
        return x;
    return y;
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
unsigned int hash(const char* string, int mult, int mod) {
    unsigned long long result = (int)string[1] - '_';
    int i = 2;
    while(string[i] != ' ') {
        //result = mult * (string[i] - '_' + result); //probably exceeds the maximum integer size, should be tested on large strings
        result = (mult*result + (int)string[i] - '_');
        i++;
    }
    return result % mod;
}