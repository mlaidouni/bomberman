#include "list.h"

#include <stdio.h>
#include <stdlib.h>

list *init_list() {
  list *l = malloc(sizeof(list));
  l->in = NULL;
  l->out = NULL;
  return l;
}

void free_list_elem(list_elem *e) {
  if (e != NULL) {
    //free(e);
  }
}

void free_list_elem_rec(list_elem *e) {
  if (e != NULL) {
    free_list_elem_rec(e->next);
    free(e->curr);
    free(e);
  }
}

void free_list(list *l) {
  free_list_elem_rec(l->in);
  free(l);
}

int length(list *list) {
  int len = 0;
  list_elem *curr_elem = list->in;
  while (curr_elem != NULL) {
    len++;
    curr_elem = curr_elem->next;
  }
  return len;
}

int add_head(list *list, void *b) {
  list_elem *new_list_elem = malloc(sizeof(list_elem));
  new_list_elem->curr = b;
  new_list_elem->prev = NULL;
  if (list->in == NULL) {
    if (list->out != NULL) {
      fprintf(stderr,
              "list.c: si la tete est NULL alors la queue doit aussi l'etre");
      exit(100);
    }
    new_list_elem->next = NULL;
    list->out = new_list_elem;
  } else {
    new_list_elem->next = list->in;
    list->in->prev = new_list_elem;
  }
  list->in = new_list_elem;
  return 0;
}

int remove_head(list *list) {
  if (list->in != NULL) {
    if (list->in->next != NULL) {
      list_elem *next = list->in->next;
      next->prev = NULL;
      free(list->in);
      list->in = next;
      return 0;
    }
    return 1;
  }
  return 2;
}

int remove_tail(list *list) {
  if (list->out != NULL) {
    if (list->out->prev != NULL) {
      list_elem *prev = list->out->prev;
      prev->next = NULL;
      free(list->out);
      list->out = prev;
      return 0;
    }
    free(list->out);
    list->in = NULL;
    list->out = NULL;
    return 1;
  }
  return 2;
}

/**
 * Supprimer un element particulier de la liste
 * @param list La liste
 * @param b L'element en question
 * @return 0
 * @return -1 si la fonction echoue
 */
int remove_elem(list *list, void *b) {
  if (list == NULL) {
    fprintf(stderr, "list.c: retire un element d'un list NULL");
    exit(101);
  }
  list_elem *curr_elem = list->in;
  if (list->in == NULL) {
    return -1;
  }
  
  do {
    if (curr_elem->curr == b) {
      if (curr_elem->prev != NULL) {
        curr_elem->prev->next = curr_elem->next;
      }
      if (curr_elem->next != NULL) {
        curr_elem->next->prev = curr_elem->prev;
      }
      free_list_elem(curr_elem);
    }
  } while ((curr_elem = curr_elem->next) != NULL);
  return -1;
}

void *pop_tail(list *list) {
  list_elem *tl = list->out;
  if (tl == NULL) {
    return NULL;
  }
  remove_tail(list);
  return tl->curr;
}
