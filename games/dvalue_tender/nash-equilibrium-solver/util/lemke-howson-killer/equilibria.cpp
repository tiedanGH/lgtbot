/*
  Equilibria library. 

  This library contains the actual data structures used to implement equilibria
  and lists of equilibria (used in the all_lemke algorithm).

  Equilibria are linked lists of items containing the index of the strategy and the probability
  of using that. Lists of equilibria are implemented with linked lists of lexicographically
  sorted equilibria. This way it's easy to find out if we discovered an equilibrium we already
  found before.

*/

#include "equilibria.h"
#include "stdio.h"
#include "stdlib.h"

/*
  Adds a new strategy (with the related probability) in the equilibrium, and returns
  the updated linked list. The equilibrium itself is the head of this linked list.
*/

equilibrium* add_strategy(equilibrium *old, int label, double prob) {
  equilibrium *i;
 
  equilibrium *neweq = (equilibrium*)malloc(sizeof(equilibrium));
  neweq->label = label;
  neweq->prob = prob;
  neweq->next = 0;
  
  //In this case, we need to create a new equilibrium, so we simply return item we just created.
  
  if ( old == 0 )
    return neweq;

  //The strategies are sorted, so this checks if we need to put the new element in the head of the list.
  
  if ( old->label > label ) {
    neweq->next = old;
    return neweq;
  }

  for( i = old; ; i = i->next) {
    
    //Checks if we reached the end of the list
    if(i->next == 0) {
      i->next = neweq;
      return old;
    }

    //In this case, we have to insert the item in this position
    if( i->next->label > label ) {
      neweq->next = i->next;
      i->next = neweq;
      return old;
    }
  }
}

void print_equilibrium(equilibrium *eq, FILE *f) {
  fprintf(f,"\nStrategy\tProbability\n");

  while(eq!=0) {
    fprintf(f,"%d\t\t%.7lf\n",eq->label,eq->prob);
    eq = eq->next;
  }

  fprintf(f,"\n");
  return;
}

/*
  This debug function prints the equilibrium found in the style of GAMBIT. This
  is useful for testing and debugging purposes.
*/

void print_equilibrium_gambit(equilibrium *eq, int dim1, int dim2, FILE *f) {
  int i;
  fprintf(f,"NE");

  for(i = 1; i <= (dim1+dim2); i++) {
    if( eq!=0 && eq->label == i) {
      fprintf(f,",%-.8lf",eq->prob);
      eq = eq->next;
    }
    else
      fprintf(f,",0");
  }
  fprintf(f,"\n");
}

/*
  Lexicographical comparison function to compare two equilibria.
*/

int lex_comp(equilibrium* x, equilibrium* y) {
  
  if( x == 0 && y == 0 )
    return 0;
  if( x!=0 && y == 0 )
    return 1;
  if( x == 0 && y != 0)
    return -1;

  if( x->label < y->label )
    return -1;
  else if ( x->label > y->label )
    return 1;
  else
    return lex_comp(x->next,y->next);
}

/*
  Checks whether the equilibrium we found is artificial or not.
*/

int is_artificial(equilibrium* x) {
  return ( x == 0 );
}

/*
  Return the equilibrium support size
*/

int eq_size(equilibrium* x) {
  int size = 0;

  while(x!=0) {
    size++;
    x = x->next;
  }

  return size;
}


void free_equilibrium(equilibrium* eq) {
  if( !eq )
    return;

  free_equilibrium(eq->next);
  free(eq);
}

/*
  Adds a new equilibrium in the list, and takes care of keeping it sorted in lexicographical
  order. This is done simply inserting the element in the correct position, checking if we
  need to put it in the head or in the end of the list. In addiction, this function checks if the
  equilibrium is already in the list: in this case, it returns the same list, and sets found = 1.
  This is needed in order to stop the recursive implementation of all_lemke.
*/

eqlist* search_add_equilibrium(eqlist* list, equilibrium *eq, int* found) {
  
  eqlist* i;
  
  eqlist* newlist = (eqlist*)malloc(sizeof(eqlist));
  newlist->next = 0;
  newlist->eq = eq;
  
  if ( list == 0 ) {
    *found = 0;
    return newlist;
  }
  
  if ( lex_comp(list->eq,eq) > 0 ) {
    *found = 0;
    newlist->next = list;
    return newlist;
  }
  else if ( lex_comp(list->eq,eq) == 0 ) {
    *found = 1;
    free(newlist);
    return list;
  } 
  
  for( i = list; ; i = i->next) {
    
    if(i->next == 0) {
      *found = 0;
      i->next = newlist;
      return list;
    }
    
    if ( lex_comp(i->next->eq,eq) == 0 ) {
      *found = 1;
      free(newlist);
      return list;
    }
    
    if( lex_comp(i->next->eq,eq) > 0) {
      *found = 0;
      newlist->next = i->next;
      i->next = newlist;
      return list;
    }
  }

}

void print_eqlist(eqlist *list, FILE *f) {
  int k = 0;
  char suff[][4] = { "st", "nd", "rd", "th"};

  for( ; list != 0; list = list->next ) {
    ++k;
    fprintf(f,"%d%s equilibrium:\n",k,suff[k>3?3:k-1]);
    print_equilibrium(list->eq,f);
  }

}


/*
  Again, this is for testing and debugging purposes: it prints a list of equilibria
  following the style of GAMBIT.
*/

void print_eqlist_gambit(eqlist *list, int dim1, int dim2, FILE *f) {
  
  for( ; list != 0; list = list->next ) {
    print_equilibrium_gambit(list->eq,dim1,dim2,f);
  }
  
}

void free_eqlist(eqlist* lista) {
  if( !lista )
    return;
  
  free_eqlist(lista->next);
  free_equilibrium(lista->eq);
  free(lista);
}
