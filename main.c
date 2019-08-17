// --- LIBRARIES ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --- CONSTANTS ---

#define HASH_SIZE_ENT 271
#define HASH_SIZE_REL 271
#define HASH_MULTIPLIER 31
#define  MAX_STRING_SIZE 100


// --- DATA TYPES DEFINITIONS ---

typedef struct _entity {
    char name[MAX_STRING_SIZE];
    unsigned short int version;   //odd if valid, even otherwise
    struct _relationTree *relations;
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

    t_entity *recipient;
    int numSenders;
    unsigned short int recVersion;
    t_senderList *senderList;
} t_relInstance;

typedef struct _entityTree {
    struct _entityTree *rightChild;
    struct _entityTree *leftChild;
    int height;

    t_entity *entity;
} t_entityTree;

typedef struct _relation {
    char name[MAX_STRING_SIZE];
    t_relInstance *root;
    int maxSenders;
    bool recalc;
    t_entityTree *recipients;
    struct _relation *next;
} t_relation, *t_relAddr;

typedef struct _relationTree {
    struct _relationTree *rightChild;
    struct _relationTree *leftChild;
    int height;

    t_relation *relation;
} t_relationTree;



// --- GLOBAL VARIABLES ---

t_entityAddr entityTable[HASH_SIZE_ENT];
t_relAddr relTable[HASH_SIZE_REL];
t_relationTree *relRoot;

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
t_relInstance *addRelationInstance(t_relation*, t_relInstance*, t_entity*, t_entity*);
void delRelationInstance(t_relation*, t_relInstance*, t_entity*, t_entity*);

//Queue management
t_entityTree *addToRecipientTree(t_entityTree*, t_entity*);
t_entityTree *delItem(t_entityTree*, t_entity*);
t_entityTree *delTree(t_entityTree *node);

//Report printing and support
int countInstanceSenders(t_relation*, t_relInstance*);
void recalcRecipients(t_relation*, t_relInstance*);
int printSingleReport(t_relation*);
int printRelations(t_relationTree*);


//AVL support
int getBalance(t_relInstance*);
int getHeight(t_relInstance*);

int ent_getBalance(t_entityTree*);
int ent_getHeight(t_entityTree*);

int rel_getBalance(t_relationTree*);
int rel_getHeight(t_relationTree*);

t_entityTree *ent_minValueNode(t_entityTree*);
void ent_printTree (t_entityTree*);

t_relationTree *addToRelTree(t_relationTree*, t_relation*);

// AVL rotation
t_relInstance *doubleRotateLeft(t_relInstance*);
t_relInstance *doubleRotateRight(t_relInstance*);
t_relInstance *rotateLeft(t_relInstance*);
t_relInstance *rotateRight(t_relInstance*);

t_entityTree *ent_doubleRotateLeft(t_entityTree*);
t_entityTree *ent_doubleRotateRight(t_entityTree*);
t_entityTree *ent_rotateLeft(t_entityTree*);
t_entityTree *ent_rotateRight(t_entityTree*);

t_relationTree *rel_doubleRotateLeft(t_relationTree*);
t_relationTree *rel_doubleRotateRight(t_relationTree*);
t_relationTree *rel_rotateLeft(t_relationTree*);
t_relationTree *rel_rotateRight(t_relationTree*);

//misc
int max(int, int);
unsigned int hash(const char*, int, int);
void refreshFlags(t_relationTree *node);


int main(){
    char command[7],
            entName1[MAX_STRING_SIZE],
            entName2[MAX_STRING_SIZE],
            relName[MAX_STRING_SIZE];
    //freopen("TestCases/6_MultipleRepeated/batch6.2.in", "r", stdin);      //redirecting standard input, used for debugging in CLion

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
    newEnt->relations = NULL;
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
            if(temp->version % 2 == 0) {
                temp->version++;
            }
            refreshFlags(temp->relations);
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
                    temp->root = addRelationInstance(temp, temp->root, senderAddr, recipientAddr);
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
            newRel->maxSenders = 0;
            newRel->recipients = NULL;
            newRel->recalc = false;
            newRel->root = addRelationInstance(newRel, newRel->root, senderAddr, recipientAddr);

            relRoot = addToRelTree(relRoot, newRel);
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
                delRelationInstance(temp, temp->root, senderAddr, recipientAddr);
                return;
            }
            temp = temp->next;
        }
    }
}

void printReport() {
    int count = 0;
    count = printRelations(relRoot);
    if (count == 0)
        fputs("none", stdout);
    fputs("\n", stdout);
}

int printRelations(t_relationTree *node) {
    if (node == NULL)
        return 0;
    int count = 0;
    printRelations(node->leftChild);

    if (node->relation->recalc == true) {
        node->relation->maxSenders = -1;
        recalcRecipients(node->relation, node->relation->root);
        node->relation->recalc = false;
    }

    count+=printSingleReport(node->relation);
    printRelations(node->rightChild);
    return count;
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

t_relInstance *addRelationInstance(t_relation *rel, t_relInstance *node, t_entity *sender, t_entity *recipient) {
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

        recipient->relations = addToRelTree(recipient->relations, rel);
        sender->relations = addToRelTree(sender->relations, rel);

        if (rel->maxSenders == newNode->numSenders)
            rel->recipients = addToRecipientTree(rel->recipients, recipient);
        else if (newNode->numSenders > rel->maxSenders) {
            rel->recipients = delTree(rel->recipients);
            rel->recipients = addToRecipientTree(rel->recipients, recipient);
            rel->maxSenders = newNode->numSenders;
        }

        return newNode;

    } else if(strcmp(recipient->name, node->recipient->name) < 0) {
        node->leftChild = addRelationInstance(rel, node->leftChild, sender, recipient);
        if(getBalance(node) > 1) {
            if(strcmp(node->recipient->name, node->leftChild->recipient->name) < 0)
                node = rotateLeft(node);
            else
                node = doubleRotateLeft(node);
        }
    }
    else if (strcmp(recipient->name, node->recipient->name) > 0) {
        node->rightChild = addRelationInstance(rel, node->rightChild, sender, recipient);
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

            if(node->numSenders == rel->maxSenders)
                rel->recipients = delItem(rel->recipients, recipient);
            node->numSenders = 0;
        }

        t_senderList *temp = node->senderList;
        node->recVersion = recipient->version;
        while (temp != NULL) {
            if (temp->address == sender) {
                if (sender->version > temp->version) {
                    temp->version = sender->version;
                    node->numSenders++;

                    if (rel->maxSenders == node->numSenders)
                        rel->recipients = addToRecipientTree(rel->recipients, recipient);
                    else if (node->numSenders > rel->maxSenders) {
                        rel->recipients = delTree(rel->recipients);
                        rel->recipients = addToRecipientTree(rel->recipients, recipient);
                        rel->maxSenders = node->numSenders;
                    }
                    sender->relations = addToRelTree(sender->relations, rel);
                }
                return node;
            }
            temp = temp->next;
        }

        recipient->relations = addToRelTree(recipient->relations, rel);
        sender->relations = addToRelTree(sender->relations, rel);
        t_senderList *newSender = (t_senderList*)malloc(sizeof(t_senderList));
        newSender->address = sender;
        newSender->version = sender->version;
        newSender->next = node->senderList;
        node->senderList = newSender;
        node->numSenders++;

        if (rel->maxSenders == node->numSenders)
            rel->recipients = addToRecipientTree(rel->recipients, recipient);
        else if (node->numSenders > rel->maxSenders) {
            rel->recipients = delTree(rel->recipients);
            rel->recipients = addToRecipientTree(rel->recipients, recipient);
            rel->maxSenders = node->numSenders;
        }
        return node;
    }
    node->height =  max(getHeight(node->rightChild), getHeight(node->leftChild));
    return node;
}

void delRelationInstance(t_relation *rel, t_relInstance *node, t_entity *sender, t_entity *recipient) {
    if (node == NULL) {
        return;
    }
    else if (node->recipient == recipient) {
        t_senderList *curr = node->senderList;
        if (node->senderList == NULL) {
            return;
        }

        if(node->senderList->address == sender) {
            if (node->senderList->version == sender->version && sender->version % 2 == 0) {
                if (node->numSenders == rel->maxSenders) {
                    rel->recipients = delItem(rel->recipients, recipient);
                    if (rel->recipients == NULL) {
                        rel->maxSenders = -1;
                        rel->recalc = true;
                    }
                }
                node->numSenders--;
            }

            node->senderList = curr->next;
            free(curr);
           return;
        }
        t_senderList *prev = curr;
        curr = curr->next;

        while(curr != NULL) {
            if(curr->address == sender) {

                if (curr->version == sender->version && sender->version % 2 == 0) {
                    if (node->numSenders == rel->maxSenders) {
                        rel->recipients = delItem(rel->recipients, recipient);

                        if (rel->recipients == NULL) {
                            rel->maxSenders = -1;
                            rel->recalc = true;
                        }
                    }
                    node->numSenders--;
                }
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
            delRelationInstance(rel, node->leftChild, sender, recipient);
        else if (strcmp(recipient->name, node->recipient->name) > 0)
            delRelationInstance(rel, node->rightChild, sender, recipient);
    }
}

//Queue management

t_entityTree *addToRecipientTree(t_entityTree *node, t_entity *newEntity) {
    if (node == NULL) {
        t_entityTree *newItem = (t_entityTree*) malloc(sizeof(t_entityTree));
        newItem->entity = newEntity;
        newItem->rightChild = NULL;
        newItem->leftChild = NULL;
        newItem->height = 0;
        return newItem;
    }
    else if (strcmp(newEntity->name, node->entity->name) < 0) {
        node->leftChild = addToRecipientTree(node->leftChild, newEntity);
        if(ent_getBalance(node) > 1) {
            if (strcmp(node->entity->name, node->leftChild->entity->name) < 0)
                node = ent_rotateLeft(node);
            else
                node = ent_doubleRotateLeft(node);
        }
    }
    else if (strcmp(newEntity->name, node->entity->name) > 0) {
        node->rightChild = addToRecipientTree(node->rightChild, newEntity);
        if (ent_getBalance(node) > 1) {
            if(strcmp(node->entity->name, node->leftChild->entity->name) < 0)
                node = ent_rotateRight(node);
            else
                node = ent_doubleRotateRight(node);
        }
    }
    node->height = max(ent_getHeight(node->rightChild), ent_getHeight(node->leftChild));
    return node;
}

t_entityTree *delItem(t_entityTree *node, t_entity *entity) {
    if (node == NULL)
        return NULL;

    if(strcmp(entity->name, node->entity->name) < 0)
        node->leftChild = delItem(node->leftChild, entity);
    else if(strcmp(entity->name, node->entity->name) > 0)
        node->rightChild = delItem(node->rightChild, entity);
    else {
        if (node->leftChild == NULL || node->rightChild == NULL) {
            t_entityTree *temp = node->leftChild ? node->leftChild : node->rightChild;

            if (temp == NULL) { //no children
                temp = node;
                node = NULL;
            }
            else   //only one child
                *node = *temp;

            free(temp);
        }
        else {
            t_entityTree *temp = ent_minValueNode(node->rightChild);
            node->entity = temp->entity;
            node->rightChild = delItem(node->rightChild, temp->entity);
        }
    }



    if (node == NULL)
        return node;

    node->height = max(ent_getHeight(node->rightChild), ent_getHeight(node->leftChild));
    return node;
}

t_entityTree *delTree(t_entityTree *node) {
    if (node == NULL)
        return NULL;
    node->leftChild = delTree(node->leftChild);
    node->rightChild = delTree(node->rightChild);
     free(node);
     return NULL;
}



int countInstanceSenders(t_relation *rel, t_relInstance *node) {
    if (rel->recalc == false)
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


//AVL support

int getBalance(t_relInstance *node) {
    if (node == NULL)
        return 0;
    return getHeight(node->leftChild) - getHeight(node->rightChild);
}

int ent_getBalance(t_entityTree *node) {
    if (node == NULL)
        return 0;
    return ent_getHeight(node->leftChild) - ent_getHeight(node->rightChild);
}

int rel_getBalance(t_relationTree *node) {
    if (node == NULL)
        return 0;
    return rel_getHeight(node->leftChild) - rel_getHeight(node->rightChild);
}

int getHeight(t_relInstance *node) {
    if (node == NULL)
        return -1;
    return node->height;
}

int ent_getHeight(t_entityTree *node) {
    if (node == NULL)
        return -1;
    return node->height;
}

int rel_getHeight(t_relationTree *node) {
    if (node == NULL)
        return -1;
    return node->height;
}


t_entityTree *ent_minValueNode(t_entityTree *node) {
    t_entityTree *temp = node;
    while (temp->leftChild != NULL)
        temp = temp->leftChild;
    return temp;
}

void ent_printTree (t_entityTree *node) {
    if (node == NULL)
        return;
    ent_printTree(node->leftChild);
    fputs(node->entity->name, stdout);
    ent_printTree(node->rightChild);
}


//AVL rotation

t_relInstance *doubleRotateLeft(t_relInstance *node) {
    node->leftChild = rotateRight(node->leftChild);
    return rotateLeft(node);
}

t_entityTree *ent_doubleRotateLeft(t_entityTree *node) {
    node->leftChild = ent_rotateRight(node->leftChild);
    return ent_rotateLeft(node);
}

t_relationTree *rel_doubleRotateLeft(t_relationTree *node) {
    node->leftChild = rel_rotateRight(node->leftChild);
    return rel_rotateLeft(node);
}


t_relInstance *doubleRotateRight(t_relInstance *node) {
    node->rightChild = rotateLeft(node->rightChild);
    return rotateRight(node);
}

t_entityTree *ent_doubleRotateRight(t_entityTree *node) {
    node->rightChild = ent_rotateLeft(node->rightChild);
    return ent_rotateRight(node);
}
t_relationTree *rel_doubleRotateRight(t_relationTree *node) {
    node->rightChild = rel_rotateLeft(node->rightChild);
    return rel_rotateRight(node);
}


t_relInstance *rotateLeft(t_relInstance *node) {
    t_relInstance *temp = node->leftChild;
    node->leftChild = temp->rightChild;
    temp->rightChild = node;

    node->height = max(getHeight(node->leftChild), getHeight(node->rightChild)) + 1;
    temp->height = max(getHeight(temp->leftChild), getHeight(node->rightChild)) + 1;

    return temp;
}

t_entityTree *ent_rotateLeft(t_entityTree *node) {
    t_entityTree *temp = node->leftChild;
    node->leftChild = temp->rightChild;
    temp->rightChild = node;

    node->height = max(ent_getHeight(node->leftChild), ent_getHeight(node->rightChild)) + 1;
    temp->height = max(ent_getHeight(temp->leftChild), ent_getHeight(node->rightChild)) + 1;

    return temp;
}

t_relationTree *rel_rotateLeft(t_relationTree *node) {
    t_relationTree *temp = node->leftChild;
    node->leftChild = temp->rightChild;
    temp->rightChild = node;

    node->height = max(rel_getHeight(node->leftChild), rel_getHeight(node->rightChild)) + 1;
    temp->height = max(rel_getHeight(temp->leftChild), rel_getHeight(node->rightChild)) + 1;

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

t_entityTree *ent_rotateRight(t_entityTree *node) {
    t_entityTree *temp = node->rightChild;
    node->rightChild = temp->leftChild;
    temp->leftChild = node;

    node->height = max(ent_getHeight(node->leftChild), ent_getHeight(node->rightChild)) + 1;
    temp->height = max(ent_getHeight(temp->leftChild), ent_getHeight(node->rightChild)) + 1;
    return temp;
}

t_relationTree *rel_rotateRight(t_relationTree *node) {
    t_relationTree *temp = node->rightChild;
    node->rightChild = temp->leftChild;
    temp->leftChild = node;

    node->height = max(rel_getHeight(node->leftChild), rel_getHeight(node->rightChild)) + 1;
    temp->height = max(rel_getHeight(temp->leftChild), rel_getHeight(node->rightChild)) + 1;
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

void recalcRecipients(t_relation *rel, t_relInstance *node) {
    if (node != NULL) {
        recalcRecipients(rel, node->rightChild);

        if (node->recVersion == node->recipient->version) {
            int tempCount = countInstanceSenders(rel, node);
            if (tempCount > 0) {
                if (tempCount == rel->maxSenders) {
                    rel->recipients = addToRecipientTree(rel->recipients, node->recipient);
                }
                else if (tempCount > rel->maxSenders) {
                    delTree(rel->recipients);
                    rel->recipients = NULL;
                    rel->recipients = addToRecipientTree(rel->recipients, node->recipient);
                    rel->maxSenders = tempCount;
                }
            }
        }

        recalcRecipients(rel, node->leftChild);
    }
}

int printSingleReport(t_relation *relation) {
    if (relation->maxSenders == 0)
        return 0;
    if (relation->maxSenders < 0)
        return 1;
    fputs(relation->name, stdout);
    ent_printTree(relation->recipients);
    printf("%d; ", relation->maxSenders);
    return 1;
}

t_relationTree *addToRelTree(t_relationTree *node, t_relation *newRel) {
    if (node == NULL) {
        t_relationTree *newNode = (t_relationTree *) malloc(sizeof(t_relationTree));
        newNode->rightChild = NULL;
        newNode->leftChild = NULL;
        newNode->height = 0;
        newNode->relation = newRel;
        return newNode;
    }
    else if (strcmp(newRel->name, node->relation->name) < 0) {
        node->leftChild = addToRelTree(node->leftChild, newRel);
        if (rel_getBalance(node) > 1) {
            if (strcmp(node->relation->name, node->leftChild->relation->name) < 0)
                node = rel_rotateLeft(node);
            else
                node = rel_doubleRotateLeft(node);
        }
    }
    else if (strcmp(newRel->name, node->relation->name) > 0) {
        node->rightChild = addToRelTree(node->rightChild, newRel);
        if (rel_getBalance(node) > 1) {
            if (strcmp(node->relation->name, node->leftChild->relation->name) < 0)
                node = rel_rotateRight(node);
            else
                node = rel_doubleRotateRight(node);
        }
    }
    node->height = max(rel_getHeight(node->rightChild), rel_getHeight(node->leftChild));
    return node;
}

void refreshFlags(t_relationTree *node) {
    if (node == NULL)
        return;
    refreshFlags(node->leftChild);
    refreshFlags(node->rightChild);
    node->relation->recalc = true;

}