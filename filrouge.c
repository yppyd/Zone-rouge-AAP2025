#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_READ_LINE 100

typedef int t_bool;    // Booléen
typedef int t_vertex;  // Sommet de graphe

typedef struct node {
  t_vertex val;
  struct node * p_next;
} t_node;

typedef t_node * t_list;
typedef t_list t_stack;

// --- DÉFINITION DU GRAPHE (Listes d'adjacence) --- 
typedef struct {
  int size;   // Nombre de sommets
  t_list * l; // Tableau de listes d'adjacence (tableau de têtes de listes)
} t_graph;

//////// Prototypes Graphes ////////
t_graph * graph_new(int size);
void graph_free(t_graph * g);
void graph_add_edge(t_graph * g, t_vertex src, t_vertex dest);
t_graph * graph_read_from_file(FILE * fp);
void graph_write_dot(t_graph * g, FILE * fp);

//////// Prototypes Piles & Listes (Déjà dans votre fichier) ////////
void stack_show(t_stack * ps);
t_stack * stack_new();
int stack_is_empty(t_stack * ps);
void stack_push(t_vertex e, t_stack * ps);
t_vertex stack_pop(t_stack * ps);
t_vertex stack_top(t_stack * ps);

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

//////// MAIN ////////
int main(int argc, char *argv[]) {
    // Gestion des arguments selon le PDF Page 7 
    char *input_filename = NULL;
    char *output_filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_filename = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_filename = argv[i + 1];
            i++;
        } else {
            printf("Argument non reconnu ou incomplet: %s\n", argv[i]);
            printf("Usage: %s -i <fichier_entree> -o <fichier_sortie>\n", argv[0]);
            return 1;
        }
    }

    // 1. Lecture (Entrée standard si pas de fichier -i)
    FILE *in_stream = stdin;
    if (input_filename != NULL) {
        in_stream = fopen(input_filename, "r");
        if (in_stream == NULL) {
            perror("Erreur ouverture fichier entree");
            return 1;
        }
    }

    t_graph * g = graph_read_from_file(in_stream);
    
    if (input_filename != NULL) fclose(in_stream);
    
    if (g == NULL) {
        fprintf(stderr, "Erreur lors de la lecture du graphe.\n");
        return 1;
    }

    // 2. Écriture (Sortie standard si pas de fichier -o)
    FILE *out_stream = stdout;
    if (output_filename != NULL) {
        out_stream = fopen(output_filename, "w");
        if (out_stream == NULL) {
            perror("Erreur ouverture fichier sortie");
            graph_free(g);
            return 1;
        }
    }

    graph_write_dot(g, out_stream);

    if (output_filename != NULL) fclose(out_stream);

    // Nettoyage
    graph_free(g);

    return 0;
}

//////// GRAPHES ////////

// Crée un graphe de taille donnée (allocation dynamique)
t_graph * graph_new(int size) {
    t_graph * g = malloc(sizeof(t_graph));
    assert(g != NULL);
    g->size = size;
    
    // Allocation du tableau de listes
    g->l = malloc(size * sizeof(t_list));
    assert(g->l != NULL);

    // Initialisation de chaque liste à vide (NULL)
    for (int i = 0; i < size; i++) {
        g->l[i] = list_new();
    }
    return g;
}

// Libère la mémoire du graphe
void graph_free(t_graph * g) {
    if (g == NULL) return;
    for (int i = 0; i < g->size; i++) {
        list_free(g->l[i]); // Utilise la fonction fournie pour libérer les listes
    }
    free(g->l);
    free(g);
}

// Ajoute un arc orienté de src vers dest
// Note: Pour les listes d'adjacence, on ajoute dest dans la liste de src
void graph_add_edge(t_graph * g, t_vertex src, t_vertex dest) {
    assert(src >= 0 && src < g->size);
    assert(dest >= 0 && dest < g->size);
    
    // On ajoute en tête de liste (O(1))
    g->l[src] = list_add_head(dest, g->l[src]);
}

// Lecture selon le Format 1 (Numéros)
t_graph * graph_read_from_file(FILE * fp) {
    int size;
    int src, dest;
    
    // Lecture du nombre de sommets (première ligne)
    // On ignore les lignes vides si besoin
    if (fscanf(fp, "%d", &size) != 1) {
        return NULL; 
    }

    t_graph * g = graph_new(size);

    // Lecture des arcs (paires d'entiers)
    while (fscanf(fp, "%d %d", &src, &dest) == 2) {
        if (src >= 0 && src < size && dest >= 0 && dest < size) {
            graph_add_edge(g, src, dest);
        } else {
            fprintf(stderr, "Avertissement: Arc %d -> %d invalide (hors bornes)\n", src, dest);
        }
    }
    return g;
}

// Écriture au format DOT
void graph_write_dot(t_graph * g, FILE * fp) {
    fprintf(fp, "digraph mon_graphe {\n");
    
    // Parcours de tous les sommets
    for (int i = 0; i < g->size; i++) {
        // Parcours de la liste d'adjacence du sommet i avec le curseur fourni
        t_node * lc = list_cursor_new(g->l[i]);
        
        // Si le sommet n'a pas d'arc, on pourrait l'afficher seul (optionnel en DOT standard mais propre)
        // fprintf(fp, "  %d;\n", i); 

        while (!list_cursor_at_end(lc)) {
            t_vertex voisin = list_cursor_get_val(lc);
            // Écriture de l'arc : i -> voisin
            fprintf(fp, "  %d -> %d;\n", i, voisin);
            
            lc = list_cursor_next(lc);
        }
    }
    fprintf(fp, "}\n");
}

//////// Piles ////////

// Crée une nouvelle pile vide
// Utilisation : t_stack * ps = stack_new();
t_stack * stack_new() {
  t_stack * ps = malloc(sizeof(*ps));
  assert(ps != NULL);
  *ps = list_new();
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

// Crée une nouvelle liste chaînée
// Utilisation : t_list l = list_new();
t_list list_new() {
  return NULL;
}

// Ajoute un maillon avec la valeur e au début de la liste l
t_list list_add_head(t_vertex e, t_list l) {
  t_node * n = malloc(sizeof(t_node));
  assert(n != NULL);
  n->val = e;
  n->p_next = l;
  return n;
}

// Retourne 1 si la liste l est vide, 0 sinon
int list_is_empty(t_list l) {
  return l == NULL;
}

// Affiche la liste l
void list_show(t_list l) {
  t_node * n = l;
  int i = 0;
  while (n != NULL) {
    printf("Maillon %d, valeur = %d\n", i++, n->val);
    n = n->p_next;
  }
}

// Retourne la valeur du premier maillon de la liste l
t_vertex list_get_first_val(t_list l) {
  assert(l != NULL);
  return l->val;
}

// Retire le premier maillon de la liste l et retourne la nouvelle tête
// Utilisation : l = list_remove_head(l);
t_list list_remove_head(t_list l) {
  t_list l2;
  assert(l != NULL);
  l2 = l->p_next;
  free(l);
  return l2;
}

// Libère la liste l de la mémoire (maillon par maillon)
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
t_node * list_cursor_new(t_list l) {
  return l;
}

// Retourne 1 si la fin de la liste a été atteinte
// Remarque : la fin d'une liste (NULL) est la même chose qu'une liste vide !
int list_cursor_at_end(t_node * lc) {
  return list_is_empty(lc);
}

// Retourne la valeur du maillon actuellement pointé par lc
// Remarque : ça revient en fait à prendre la valeur du premier maillon de lc !
int list_cursor_get_val(t_node * lc) {
  return list_get_first_val(lc);
}

// Avance le curseur vers le maillon suivant
// Utilisation : lc = list_cursor_next(lc);
t_node * list_cursor_next(t_node * lc) {
  assert(lc != NULL);
  return lc->p_next;
}

