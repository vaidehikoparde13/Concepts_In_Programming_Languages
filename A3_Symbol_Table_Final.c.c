#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NAME_SIZE 50
#define HASH_SIZE 29

/*------------------------------------------Structures-----------------------------------------------*/

typedef struct HashNode_Tag
{
    char name[NAME_SIZE];
    int value;
    struct HashNode_Tag *next;
} HashNode;

typedef struct ScopeNodeTag
{
    HashNode *hashArr[HASH_SIZE]; // does this get allotted on heap or stack?
    struct ScopeNodeTag *next;
} ScopeNode;

/*------------------------------------------Initialise Scope top-----------------------------------------------*/

ScopeNode *top = NULL;

/*------------------------------------------Create Node Functions-----------------------------------------------*/

ScopeNode *CreateScopeNode()
{
    ScopeNode *nptr = (ScopeNode *)malloc(sizeof(ScopeNode));
    for (int i = 0; i < HASH_SIZE; i++)
    {
        nptr->hashArr[i] = NULL;
    }
    nptr->next = NULL;
    return nptr;
}

HashNode *CreateHashNode(char c[], int val)
{
    HashNode *nptr;
    nptr = (HashNode *)malloc(sizeof(HashNode));
    strcpy(nptr->name, c);
    nptr->value = val;
    nptr->next = NULL;
    return nptr;
}

/*------------------------------------------Hash function-----------------------------------------------*/

int hash(char c[])
{
    int index, sum = 0;
    for (int i = 0; c[i] != '\0'; i++)
    {
        sum += c[i];
    }
    index = sum % HASH_SIZE;
    // printf("\nid: %d, var: %s", index, c);
    return index;
}

/*------------------------------------------begin, assign, print, end functions-----------------------------------------------*/

void begin() // begin -> updates top
{
    ScopeNode *nptr;
    nptr = CreateScopeNode();
    nptr->next = top;
    top = nptr;
}

void assign(char c[], int val)
{
    HashNode *nptr = CreateHashNode(c, val);
    nptr->next = top->hashArr[hash(c)];
    top->hashArr[hash(c)] = nptr;
}

void print(char c[])
{
    ScopeNode *sptr = top;
    int index = hash(c), found = 0;
    while ((!found) && sptr != NULL)
    {
        HashNode *hptr;
        hptr = sptr->hashArr[index];
        while ((!found) && hptr != NULL)
        {
            if (strcmp(c, hptr->name) == 0)
            {
                //printf("\n%d", hptr->value);
                printf("\n%s = %d", hptr->name, hptr->value);
                found = 1;
            }
            else
            {
                hptr = hptr->next;
            }
        }
        if (!found)
        {
            printf("\nSearch in surrounding scope.");
            sptr = sptr->next;
        }
    }
    if (!found)
    {
        printf("\nOut of all scopes.");
    }
}

void end()
{
    if (top != NULL)
    {
        ScopeNode *nptr;
        nptr = top;
        top = top->next; // memory leak, go in and delete all hash nodes also!
        for(int i = 0; i < HASH_SIZE; i++)
        {
            HashNode* hptr = nptr->hashArr[i];
            while(hptr != NULL)
            {
                HashNode* dptr = hptr;
                hptr = hptr -> next;
                free(dptr);
            }
        }
        free(nptr);
        nptr = NULL;
        //printf("\nScope node was deleted.");
    }
    else
    {
        printf("\nNo scopes to delete.");
    }
}

/*------------------------------------------Main function-----------------------------------------------*/

int main()
{

    begin();
    assign("a", 1);
    assign("b", 2);
    begin();
    assign("a", 3);
    assign("c", 4);
    print("b");
    begin();
    assign("c", 5);
    print("b");
    print("a");
    print("c");
    end();
    print("a");
    print("c");
    end();
    print("a");
    end();
    return 0;
}

/*------------------------------------------Print functions-----------------------------------------------*/

/*
void PrintAll(HashNode *arr[])
{
    HashNode *ptr;
    int d_cap;
    char pname_cap[NAME_SIZE];
    for (int i = 0; i < HASH_SIZE; i++)
    {
        ptr = arr[i];
        while (ptr != NULL)
        {
            d_cap = ptr->value;
            strcpy(pname_cap, ptr->name);
            ptr = ptr->next;
            printf("\n %s =  %d ", pname_cap, d_cap);
        }
    }
    printf("\n");
}

void PrintScopes()
{
    if (top == NULL)
    {
        printf("\nNo scopes.");
    }
    else
    {
        ScopeNode *ptr = top;
        while (ptr != NULL)
        {
            PrintAll(ptr->hashArr);
            ptr = ptr->next;
        }
    }
} */