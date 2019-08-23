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

typedef struct _entityTree {
    struct _entityTree *rightChild;
    struct _entityTree *leftChild;
    int height;

    t_entity *entity;
    int version;
} t_entityTree;

typedef struct _relInstance {
    struct _relInstance *rightChild;
    struct _relInstance *leftChild;
    int height;

    t_entity *recipient;
    int numSenders;
    unsigned short int recVersion;
    t_entityTree *senderList;
} t_relInstance;

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

t_entityTree *addSender(t_entityTree*, t_entity*, int*);
t_entityTree *delSender(t_entityTree*, t_entity*, int*);
int countTreeNodes (t_entityTree*);

int main(){
    char command[7],
            entName1[MAX_STRING_SIZE],
            entName2[MAX_STRING_SIZE],
            relName[MAX_STRING_SIZE];
      //freopen("TestCases/3_Mixup/batch3.1.in", "r", stdin);      //redirecting standard input, used for debugging in CLion

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
            newRel->maxSenders = -1;
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
        newNode->height = 1;
        newNode->numSenders = 0;
        newNode->senderList = NULL;
        newNode->recVersion = recipient->version;

        int hasBeenAdded = 0;
        newNode->senderList = addSender(newNode->senderList, sender, &hasBeenAdded);

        newNode->numSenders++;
        recipient->relations = addToRelTree(recipient->relations, rel);
        sender->relations = addToRelTree(sender->relations, rel);


        if (rel->maxSenders == newNode->numSenders)
            rel->recipients = addToRecipientTree(rel->recipients, recipient);
        else if (newNode->numSenders > rel->maxSenders) {
            rel->recipients = delTree(rel->recipients);
            rel->recipients = NULL;
            rel->recipients = addToRecipientTree(rel->recipients, recipient);
            rel->maxSenders = newNode->numSenders;
        }

        return newNode;

    }

    else if(strcmp(recipient->name, node->recipient->name) < 0)
        node->leftChild = addRelationInstance(rel, node->leftChild, sender, recipient);
    else if (strcmp(recipient->name, node->recipient->name) > 0)
        node->rightChild = addRelationInstance(rel, node->rightChild, sender, recipient);

    else {
        if (node->recVersion < recipient->version) {
            node->senderList = delTree(node->senderList);
            node->senderList = NULL;
            if(node->numSenders == rel->maxSenders) {
                rel->recipients = delItem(rel->recipients, recipient);
                if (rel->recipients == NULL)
                    rel->recalc = true;
            }
            node->numSenders = 0;
            node->recVersion = recipient->version;
        }

        int hasBeenAdded = 0;
        node->senderList = addSender(node->senderList, sender, &hasBeenAdded);
        if (hasBeenAdded == 1) {
            node->numSenders++;
            if (rel->maxSenders == node->numSenders)
                rel->recipients = addToRecipientTree(rel->recipients, recipient);
            else if (node->numSenders > rel->maxSenders)/*&&!ecakc*/ {
                rel->recipients = delTree(rel->recipients);
                rel->recipients = NULL;
                rel->recipients = addToRecipientTree(rel->recipients, recipient);
                rel->maxSenders = node->numSenders;
            }
            sender->relations = addToRelTree(sender->relations, rel);
            recipient->relations = addToRelTree(recipient->relations, rel);
        }
        return node;
    }
    node->height =  max(getHeight(node->rightChild), getHeight(node->leftChild)) + 1;

    if (getBalance(node) > 1) {
        if (strcmp(recipient->name, node->leftChild->recipient->name) < 0)
            return rotateRight(node);
        else {
            node->leftChild = rotateLeft(node->leftChild);
            return rotateRight(node);
        }
    }
    else if (getBalance(node) < -1) {
        if (strcmp(recipient->name, node->rightChild->recipient->name) > 0)
            return rotateLeft(node);
        else {
            node->rightChild = rotateRight(node->rightChild);
            return rotateLeft(node);
        }
    }

    return node;
}

void delRelationInstance(t_relation *rel, t_relInstance *node, t_entity *sender, t_entity *recipient) {
    if (node == NULL) {
        return;
    }
    if (node->recipient == recipient) {
        int hasBeenDeleted = 0;
        node->senderList = delSender(node->senderList, sender, &hasBeenDeleted);

        if (hasBeenDeleted == 1) {
            if (node->numSenders == rel->maxSenders) {
                rel->recipients = delItem(rel->recipients, recipient);
                if (rel->recipients == NULL) {
                    rel->maxSenders = 0;
                    rel->recalc = true;
                }
            }
            node->numSenders--;
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
        newItem->version = newEntity->version;
        newItem->height = 1;
        return newItem;
    }
    else if (strcmp(newEntity->name, node->entity->name) < 0)
        node->leftChild = addToRecipientTree(node->leftChild, newEntity);
    else if (strcmp(newEntity->name, node->entity->name) > 0)
        node->rightChild = addToRecipientTree(node->rightChild, newEntity);

    node->height =  max(ent_getHeight(node->rightChild), ent_getHeight(node->leftChild)) + 1;


    if (ent_getBalance(node) > 1) {
        if (strcmp(newEntity->name, node->leftChild->entity->name) < 0)
            return ent_rotateRight(node);
        else {
            node->leftChild = ent_rotateLeft(node->leftChild);
            return ent_rotateRight(node);
        }
    }
    else if (ent_getBalance(node) < -1) {
        if (strcmp(newEntity->name, node->rightChild->entity->name) > 0)
            return ent_rotateLeft(node);
        else {
            node->rightChild = ent_rotateRight(node->rightChild);
            return ent_rotateLeft(node);
        }
    }

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
            else {   //only one child
                *node = *temp;
                /*node->entity = temp->entity;
                node->version = temp->version;*/
            }
            free(temp);
        }
        else {
            t_entityTree *temp = ent_minValueNode(node->rightChild);
            node->entity = temp->entity;
            node->version = temp->version;
            node->rightChild = delItem(node->rightChild, temp->entity);
        }
    }

    if (node == NULL)
        return node;

    node->height = max(ent_getHeight(node->rightChild), ent_getHeight(node->leftChild)) + 1;

    if (ent_getBalance(node) > 1) {
        if (ent_getBalance(node->leftChild) >= 0)
            return ent_rotateRight(node);
        else {
            node->leftChild = ent_rotateLeft(node->leftChild);
            return ent_rotateRight(node);
        }
    }
    else if (ent_getBalance(node) < -1) {
        if (ent_getBalance(node->rightChild) <= 0)
            return ent_rotateLeft(node);
        else {
            node->rightChild = ent_rotateRight(node->rightChild);
            return ent_rotateLeft(node);
        }
    }
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
    return countTreeNodes(node->senderList);

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
        return 0;
    return node->height;
}

int ent_getHeight(t_entityTree *node) {
    if (node == NULL)
        return 0;
    return node->height;
}

int rel_getHeight(t_relationTree *node) {
    if (node == NULL)
        return 0;
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
    t_relInstance *ret = node->rightChild;
    t_relInstance *temp = ret->leftChild;

    ret->leftChild = node;
    node->rightChild = temp;

    node->height = max(getHeight(node->leftChild), getHeight(node->rightChild)) + 1;
    ret->height = max(getHeight(ret->leftChild), getHeight(ret->rightChild)) + 1;
    return ret;
}

t_entityTree *ent_rotateLeft(t_entityTree *node) {
    t_entityTree *ret = node->rightChild;
    t_entityTree *temp = ret->leftChild;

    ret->leftChild = node;
    node->rightChild = temp;

    node->height = max(ent_getHeight(node->leftChild), ent_getHeight(node->rightChild)) + 1;
    ret->height = max(ent_getHeight(ret->leftChild), ent_getHeight(ret->rightChild)) + 1;
    return ret;
}

t_relationTree *rel_rotateLeft(t_relationTree *node) {
    t_relationTree *ret = node->rightChild;
    t_relationTree *temp = ret->leftChild;

    ret->leftChild = node;
    node->rightChild = temp;

    node->height = max(rel_getHeight(node->leftChild), rel_getHeight(node->rightChild)) + 1;
    ret->height = max(rel_getHeight(ret->leftChild), rel_getHeight(ret->rightChild)) + 1;
    return ret;
}


t_relInstance *rotateRight(t_relInstance *node) {
    t_relInstance *ret = node->leftChild;
    t_relInstance *temp = ret->rightChild;

    ret->rightChild = node;
    node->leftChild = temp;

    node->height = max(getHeight(node->leftChild), getHeight(node->rightChild)) + 1;
    ret->height = max(getHeight(ret->leftChild), getHeight(ret->rightChild)) + 1;
    return ret;
}

t_entityTree *ent_rotateRight(t_entityTree *node) {
    t_entityTree *ret = node->leftChild;
    t_entityTree *temp = ret->rightChild;

    ret->rightChild = node;
    node->leftChild = temp;

    node->height = max(ent_getHeight(node->leftChild), ent_getHeight(node->rightChild)) + 1;
    ret->height = max(ent_getHeight(ret->leftChild), ent_getHeight(ret->rightChild)) + 1;
    return ret;
}

t_relationTree *rel_rotateRight(t_relationTree *node) {
    t_relationTree *ret = node->leftChild;
    t_relationTree *temp = ret->rightChild;

    ret->rightChild = node;
    node->leftChild = temp;

    node->height = max(rel_getHeight(node->leftChild), rel_getHeight(node->rightChild)) + 1;
    ret->height = max(rel_getHeight(ret->leftChild), rel_getHeight(ret->rightChild)) + 1;
    return ret;
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
            node->numSenders = countInstanceSenders(rel, node);
            if (node->numSenders > 0) {
                if (node->numSenders == rel->maxSenders) {
                    rel->recipients = addToRecipientTree(rel->recipients, node->recipient);
                }
                else if (node->numSenders > rel->maxSenders) {
                    rel->recipients = delTree(rel->recipients);
                    rel->recipients = NULL;
                    rel->recipients = addToRecipientTree(rel->recipients, node->recipient);
                    rel->maxSenders = node->numSenders;
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
        newNode->height = 1;
        newNode->relation = newRel;
        return newNode;
    }
    else if (strcmp(newRel->name, node->relation->name) < 0)
        node->leftChild = addToRelTree(node->leftChild, newRel);
    else if (strcmp(newRel->name, node->relation->name) > 0)
        node->rightChild = addToRelTree(node->rightChild, newRel);

    node->height = max(rel_getHeight(node->rightChild), rel_getHeight(node->leftChild)) + 1;

    if (rel_getBalance(node) > 1) {
        if (strcmp(newRel->name, node->leftChild->relation->name) < 0)
            return rel_rotateRight(node);
        else {
            node->leftChild = rel_rotateLeft(node->leftChild);
            return rel_rotateRight(node);
        }
    }
    else if (rel_getBalance(node) < -1) {
        if (strcmp(newRel->name, node->rightChild->relation->name) > 0)
            return rel_rotateLeft(node);
        else {
            node->rightChild = rel_rotateRight(node->rightChild);
            return rel_rotateLeft(node);
        }
    }

    return node;
}

void refreshFlags(t_relationTree *node) {
    if (node == NULL)
        return;
    node->relation->recalc = true;
    refreshFlags(node->leftChild);
    refreshFlags(node->rightChild);
}

t_entityTree *addSender(t_entityTree *node, t_entity *sender, int *flag) {
    if (node == NULL && *(flag) == 0) { //the node doesn't exist
        t_entityTree *newSender = (t_entityTree*) malloc(sizeof(t_entityTree));
        newSender->rightChild = NULL;
        newSender->leftChild = NULL;
        newSender->height = 1;
        newSender->entity = sender;
        newSender->version = sender->version;

        *(flag) = 1;
        return newSender;
    }

    else if(strcmp(sender->name, node->entity->name) < 0)
        node->leftChild = addSender(node->leftChild, sender, flag);
    else if (strcmp(sender->name, node->entity->name) > 0)
        node->rightChild = addSender(node->rightChild, sender, flag);

    else {
        if (node->version < sender->version) {
            node->version = sender->version;
            *(flag) = 1;
        }
        return node;
    }

    node->height = max(ent_getHeight(node->rightChild), ent_getHeight(node->leftChild)) + 1;

    if (ent_getBalance(node) > 1) {
        if (strcmp(sender->name, node->leftChild->entity->name) < 0)
            return ent_rotateRight(node);
        else {
            node->leftChild = ent_rotateLeft(node->leftChild);
            return ent_rotateRight(node);
        }
    }
    else if (ent_getBalance(node) < -1) {
        if (strcmp(sender->name, node->rightChild->entity->name) > 0)
            return ent_rotateLeft(node);
        else {
            node->rightChild = ent_rotateRight(node->rightChild);
            return ent_rotateLeft(node);
        }
    }

    return node;
}

t_entityTree *delSender(t_entityTree *node, t_entity *sender, int *flag) {
    if (node == NULL)
        return node;


    if (strcmp(sender->name, node->entity->name) < 0)
        node->leftChild = delSender(node->leftChild, sender, flag);
    else if (strcmp(sender->name, node->entity->name) > 0)
        node->rightChild = delSender(node->rightChild, sender, flag);
    else {
        if (*flag == 0) {
            *flag = -1;
            if (node->version == node->entity->version && node->version % 2 == 0) {
                *(flag) = 1;
            }
        }


        if (node->leftChild == NULL || node->rightChild == NULL) {
            t_entityTree *temp = node->leftChild ? node->leftChild : node->rightChild;
            if (temp == NULL) {
                temp = node;
                node = NULL;
            }
            else
                *node = *temp;

            free(temp);
        }
        else {
            t_entityTree *temp = ent_minValueNode(node->rightChild);
            node->entity = temp->entity;
            node->version = temp->version;
            node->rightChild = delSender(node->rightChild, temp->entity, flag);
        }
    }

    if (node == NULL)
        return node;

    node->height =  max(ent_getHeight(node->rightChild), ent_getHeight(node->leftChild)) + 1;

    if (ent_getBalance(node) > 1) {
        if (ent_getBalance(node->leftChild) >= 0)
            return ent_rotateRight(node);
        else {
            node->leftChild = ent_rotateLeft(node->leftChild);
            return ent_rotateRight(node);
        }
    }
    else if (ent_getBalance(node) < -1) {
        if (ent_getBalance(node->rightChild) <= 0)
            return ent_rotateLeft(node);
        else {
            node->rightChild = ent_rotateRight(node->rightChild);
            return ent_rotateLeft(node);
        }
    }

    return node;
}

int countTreeNodes (t_entityTree *node) {
    if (node == NULL)
        return 0;
    int count = 0;
    if (node->entity->version == node->version && node->version % 2 == 0)
        count++;

    count+=(countTreeNodes(node->leftChild) + countTreeNodes(node->rightChild));
    return count;
}