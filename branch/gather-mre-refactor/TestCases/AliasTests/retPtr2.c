#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define  HASHSIZE 101


struct nlist
{
   struct nlist *next;   /* next entry in chain */
   char *name;           /* defined name        */
   char *defn;           /* replacement text    */ 
};

static struct nlist *hashtab[HASHSIZE]; /* pointer table */


/* hash: form hash value for string s */
unsigned hash(char *s)
{
    unsigned hashval;

    for(hashval = 0; *s != '\0'; s++)
    {
        hashval = *s + 31 + hashval;
    }

    return (hashval % HASHSIZE) ;
}

/* lookup: look for s in hashtab */
struct nlist *lookup(char *s)
{
    struct nlist *np;

    for(np = hashtab[hash(s)]; np != NULL; np = np->next)
        if(strcmp(s, np->name) == 0)
        {
            return np;
        }
    return NULL;
}


/* install: put(name,defn) in hashtab */
struct nlist *install(char *name, char *defn)
{
    struct nlist *np;
    unsigned hashval;

    if((np = lookup(name)) == NULL)
    {
        /* Not Found */
        np = (struct nlist *) malloc(sizeof(*np));
        if(np == NULL || (np->name = strdup(name)) == NULL)
        {
            return NULL;
        }
        hashval = hash(name); 
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    }
    else
    {
        /* Already there */
        free((void *) np->defn);   /* free previous definitions */
    }
   
    if((np->defn = strdup(defn)) == NULL)
    {
        return NULL;
    }

    return np;
}

int main(void)
{
    struct nlist* retval;
    retval = install("a", "Argonne");
    retval = install("n", "National");
    retval = install("l", "Lab");
    return 0;
}
