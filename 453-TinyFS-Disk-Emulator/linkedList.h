struct node {
   void *data;
   struct node *next;
};

struct linkedList {
   node *head;
   node *tail;
   int count;
};

int init(LinkedList **l);
int deallocate(LinkedList **l);

int removeAllEntries(LinkedList *l);
void *getDataFromIdx(LinkedList *l, int idx);
int insert(LinkedList *l, void *data);
int getIndexOfData(LinkedList *l, int data);
int isEmpty(LinkedList *l);
void *removeEntry(LinkedList *l, int idx);
int size(LinkedList *l);