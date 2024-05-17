#ifndef LIST_H
#define LIST_H

typedef struct list_elem {
    void *curr;
    struct list_elem *next;
    struct list_elem *prev;
} list_elem;

typedef struct list {
    list_elem *in;
    list_elem *out;
} list;

list *init_list();
void free_list(list *list);

int add_head(list *list, void *b);
int remove_tail(list *list);
int remove_elem(list *list, void *b);
void *pop_list(list *list);

#endif