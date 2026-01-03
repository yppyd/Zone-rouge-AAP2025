#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define MAX_READ_LINE 100
#define TAILLE_G 5

typedef int t_bool;    // Booléen
typedef int t_vertex;  // Sommet de graphe

// Maillon de liste chaînée
typedef struct node {
  t_vertex val;          // Valeur (sommet)
  struct node * p_next;  // Pointeur vers le maillon suivant
} t_node;
// Liste chaînée (de sommets)
typedef t_node * t_list;
// Pile (de sommets)
typedef t_list t_stack;


//////// Graphes ////////

//////// Piles ////////
void stack_show(t_stack * ps);
t_stack * stack_new();
int stack_is_empty(t_stack * ps);
void stack_push(t_vertex e, t_stack * ps);
t_vertex stack_pop(t_stack * ps);
t_vertex stack_top(t_stack * ps);

//////// Listes chaînées ////////
t_list list_new();
t_list list_add_head(t_vertex e, t_list l);
int list_is_empty(t_list l);
void list_show(t_list l);
t_vertex list_get_first_val(t_list l);
t_list list_remove_head(t_list l);
t_list list_free(t_list l);
t_node * list_cursor_new(t_list l);
int list_cursor_at_end(t_node * lc);
int list_cursor_get_val(t_node * lc);
t_node * list_cursor_next(t_node * lc);


// TODO : type représentant un graphe // askip partie de Youri
//Rq : espace juste alloué, ini pas faite (tt est vide)

//format liste d'adjacence
typedef struct {
  int size; // Taille
  t_list * l; // Contenu
} t_graph_l;

t_graph_l* creer_graphe_liste(int taille) {
  int i;
  t_graph_l* g = malloc(sizeof(t_graph_l));  //alloue esp de taille sizeof()
  assert(g != NULL);    //si malloc, assert pour vérif si esp est vrm dispo
  g->size = taille;    //N_sommets colonnes créées
  t_list* l = malloc(sizeof(t_list));
  assert(l!=NULL);
  for(i = 0; i < taille; i++) { //pour chaque colonne
    t_list new = list_new(); 
    l[i] = new;
  }
  g->l = l;
  return g;
}

//format matrice d'adjacence
typedef struct {
  int size; // Taille
  t_bool ** m; // Contenu
} t_graph_m;

t_graph_m* creer_graphe_matrice(int taille) {
  int i;
  t_graph_m* g = malloc(sizeof(t_graph_m));
  assert(g != NULL);
  g->size = taille;
  g->m = malloc(g->size * sizeof(*(g->m)));
  assert(g->m != NULL);
  for (i=0;i<g->size;i++) {
    g->m[i] = malloc(g->size * sizeof(*(g->m[i])));
    assert(g->m[i] != NULL);
  }
  return g;
}

// TODO : prototypes //
void recherche_iter(t_graph_l g, t_vertex x, t_vertex y); //Rq _m si format matrice 

//////// main ////////
int main() {
  printf("Hello world!\n");
  
  return 0;
}


//////// Piles ////////

// Utilisation : t_stack * ps = stack_new();
t_stack * stack_new() {
  t_stack * ps = malloc(sizeof(*ps));
  assert(ps != NULL);
  *ps = list_new();    //nouvelle pile vide
  return ps;
}

// Affiche tout le contenu d'une pile (pour le débogage seulement)
void stack_show(t_stack * ps) {
  list_show(*ps);
}

// Retourne 1 si la pile *ps est vide, 0 sinon
int stack_is_empty(t_stack * ps) {
  return list_is_empty(*ps);
}

// Empile la valeur e en haut de la pile *ps
void stack_push(t_vertex e, t_stack * ps) {
  *ps = list_add_head(e, *ps);
}

// Dépile et retourne la valeur en haut de la pile *ps
t_vertex stack_pop(t_stack * ps) {
  t_vertex res = stack_top(ps);
  *ps = list_remove_head(*ps);
  return res;
}

// Retourne la valeur en haut de la pile *ps sans la dépiler
t_vertex stack_top(t_stack * ps) {
  assert(!list_is_empty(*ps));
  return list_get_first_val(*ps);
}



//////// Listes chaînées ////////

// Utilisation : t_list l = list_new();
t_list list_new() {
  return NULL;   //nouvelle liste chaînée
}

// Ajoute un maillon avec la valeur e au début de la liste l
t_list list_add_head(t_vertex e, t_list l) {
  t_node * n = malloc(sizeof(t_node));
  assert(n != NULL);
  n->val = e;
  n->p_next = l;
  return n;
}

// Retourne 1 si l est vide, 0 sinon
int list_is_empty(t_list l) {
  return l == NULL;
}

// Affiche l
void list_show(t_list l) {
  t_node * n = l;
  int i = 0;
  while (n != NULL) {
    printf("Maillon %d, valeur = %d\n", i++, n->val);
    n = n->p_next;
  }
}

// Retourne valeur du 1er maillon de l
t_vertex list_get_first_val(t_list l) {
  assert(l != NULL);
  return l->val;
}

// Retire 1er maillon de l et retourne la nouvelle tête
// Utilisation : l = list_remove_head(l);
t_list list_remove_head(t_list l) {
  t_list l2;
  assert(l != NULL);
  l2 = l->p_next;
  free(l);
  return l2;
}

// Libère l de la mémoire (maillon par maillon)
// Utilisation : l = list_free(l);
t_list list_free(t_list l) {
  t_node * n = l, * nNext;
  while (n != NULL) {
    nNext = n->p_next;
    free(n);
    n = nNext;
  }
  return NULL;
}

// Retourne un curseur de liste pour parcourir l
// Utilisation : t_node * lc = list_cursor_new(l);
// Remarque : en fait, un curseur est aussi une liste chaînée !
t_node * list_cursor_new(t_list l) {
  return l;
}

// Retourne 1 si la fin de la liste a été atteinte
// Remarque : la fin d'une liste (NULL) est la même chose qu'une liste vide !
int list_cursor_at_end(t_node * lc) {
  return list_is_empty(lc);
}

// Remarque : ça revient en fait à prendre la valeur du premier maillon de lc !
int list_cursor_get_val(t_node * lc) {
  return list_get_first_val(lc);  // Retourne valeur maillon actuellement pointé par lc
}

// Utilisation : lc = list_cursor_next(lc);
t_node * list_cursor_next(t_node * lc) {
  assert(lc != NULL);
  return lc->p_next;    // Avance le curseur vers le maillon suivant
}


// TODO : fonctions
//Prog 2, itératif

void recherche_iter(t_graph_l g, t_vertex x, t_vertex y) { //Rq _m si format matrice
    t_vertex w;
    t_stack *stack_traversal = stack_new();
    t_stack *stack_path = stack_new();
    t_stack *stack_path_final = stack_new();
    t_bool marking[TAILLE_G];
    int i;
    for (i = 0; i < TAILLE_G; i++) {
      marking[i] = false;     //on ini les cases à false
    }
    stack_push(x, stack_traversal);
    while (!stack_is_empty(stack_traversal)) {
        x = stack_pop(stack_traversal);
        if (x == y) {
            stack_push(x, stack_path);
            break;
        } else if (x==-1) {
            stack_pop(stack_path);
        } else {
            if (marking[x] == false) {
                stack_push(x, stack_path);
                stack_push(-1, stack_traversal);
              //format liste
                t_node *p = g.l[0];
                while (p != NULL) {
                  w = p->val;
                  stack_push(w, stack_traversal);
                  p = p->p_next;
                }
              //format matrice
                //for (w = 0; w < g.size; w++) {
                  //if (g.m[x][w]) {
                  //  stack_push(w, stack_traversal);
                  //}
                //}
                }
            }
        }
    if (!stack_is_empty(stack_path)) {
        while (!stack_is_empty(stack_path)) {
            stack_push(stack_pop(stack_path), stack_path_final);
        }
        while (!stack_is_empty(stack_path_final)) {
            printf("%d ", stack_pop(stack_path_final));
        }
    }
  }