/*
 * AAP - Projet Fil Rouge 2025-2026
 * Version complète et unifiée (Matrice d'adjacence)
 * Compilation : gcc filrouge.c -o filrouge
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_READ_LINE 100
#define VRAI 1
#define FAUX 0

typedef int t_bool;    // Booléen
typedef int t_vertex;  // Sommet de graphe

// --- LISTES CHAÎNÉES & PILES (Code fourni) ---
typedef struct node {
    t_vertex val;
    struct node * p_next;
} t_node;

typedef t_node * t_list;
typedef t_list t_stack;

// --- DÉFINITION DU GRAPHE (Matrice d'adjacence) ---
typedef struct {
    int size;      // Nombre de sommets
    t_bool ** m;   // Matrice d'adjacence (tableau de tableaux de booléens)
} t_graph;

// --- PROTOTYPES ---

// Gestion du Graphe
t_graph * graph_new(int size);
void graph_free(t_graph * g);
void graph_add_edge(t_graph * g, t_vertex src, t_vertex dest);
t_graph * graph_read_from_file(FILE * fp);
void graph_write_dot(t_graph * g, FILE * fp);
t_graph * graph_transpose(t_graph * g); // Pour Kosaraju

// Gestion des Piles & Listes
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

// Partie 2 : Recherche de chemin
void recherche_iter(t_graph *g, t_vertex x, t_vertex y);
t_bool Recherche_recur_f(t_graph *g, t_vertex x, t_vertex y, t_bool *marking, t_stack *stack);
t_bool Recherche_recur(t_graph *g, t_vertex x, t_vertex y, t_stack *stack);

// Partie 3 : Composantes Fortement Connexes (Kosaraju)
void enum_cfc_kosaraju(t_graph *g);
void Kosaraju_1(t_graph *g, int *order);
int Kosaraju_1_recur(t_graph *g, int x, t_bool *marking, int *order, int step);
void Kosaraju_2(t_graph *g, int *order);
t_bool Kosaraju_2_recur(t_graph *g, int x, t_bool *marking);


// ======================================================================
//                                MAIN
// ======================================================================

int main(int argc, char *argv[]) {
    char *input_filename = NULL;
    char *output_filename = NULL;
    int start_node = -1;
    int goal_node = -1;
    t_bool mode_cfc = FAUX;
    t_bool mode_path = FAUX;

    // 1. Analyse des arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_filename = argv[i + 1];
            i++;
        } 
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_filename = argv[i + 1];
            i++;
        } 
        else if (strcmp(argv[i], "-start") == 0 && i + 1 < argc) {
            start_node = atoi(argv[i + 1]);
            mode_path = VRAI;
            i++;
        } 
        else if (strcmp(argv[i], "-goal") == 0 && i + 1 < argc) {
            goal_node = atoi(argv[i + 1]);
            mode_path = VRAI;
            i++;
        } 
        else if (strcmp(argv[i], "-cfc") == 0) {
            mode_cfc = VRAI;
        }
        else {
            printf("Argument inconnu : %s\n", argv[i]);
            printf("Usage :\n");
            printf("  1. Conversion DOT : %s -i <in.txt> [-o <out.dot>]\n", argv[0]);
            printf("  2. Chemin         : %s -i <in.txt> -start <id> -goal <id>\n", argv[0]);
            printf("  3. CFC (Kosaraju) : %s -i <in.txt> -cfc\n", argv[0]);
            return 1;
        }
    }

    // 2. Lecture du graphe
    FILE *in_stream = stdin;
    if (input_filename != NULL) {
        in_stream = fopen(input_filename, "r");
        if (in_stream == NULL) { perror("Erreur ouverture entree"); return 1; }
    }

    t_graph * g = graph_read_from_file(in_stream);
    if (input_filename != NULL) fclose(in_stream);
    
    if (g == NULL) { fprintf(stderr, "Echec lecture graphe (fichier vide ou invalide).\n"); return 1; }

    // 3. Exécution selon le mode

    // MODE KOSARAJU (CFC)
    if (mode_cfc) {
        printf("--- Recherche des Composantes Fortement Connexes (Kosaraju) ---\n");
        // Redirection sortie si -o est précisé
        if (output_filename != NULL) freopen(output_filename, "w", stdout);
        
        enum_cfc_kosaraju(g);
    }
    // MODE CHEMIN
    else if (mode_path) {
        if (start_node == -1 || goal_node == -1) {
            fprintf(stderr, "Erreur : Specifiez -start ET -goal pour chercher un chemin.\n");
        } else if (start_node < 0 || start_node >= g->size || goal_node < 0 || goal_node >= g->size) {
            fprintf(stderr, "Erreur : Sommets hors limites (0 a %d).\n", g->size - 1);
        } else {
            printf("--- Recherche de chemin (%d -> %d) ---\n", start_node, goal_node);
            if (output_filename != NULL) freopen(output_filename, "w", stdout);
            
            recherche_iter(g, start_node, goal_node);
        }
    }
    // MODE PAR DÉFAUT : Conversion DOT
    else {
        FILE *out_stream = stdout;
        if (output_filename != NULL) {
            out_stream = fopen(output_filename, "w");
            if (out_stream == NULL) { perror("Erreur ouverture sortie"); return 1; }
        }
        graph_write_dot(g, out_stream);
        if (output_filename != NULL) fclose(out_stream);
    }

    // Nettoyage
    graph_free(g);
    return 0;
}


// ======================================================================
//                       IMPLÉMENTATION GRAPHES
// ======================================================================

t_graph * graph_new(int size) {
    t_graph * g = malloc(sizeof(t_graph));
    assert(g != NULL);
    g->size = size;

    // Allocation des pointeurs de lignes
    g->m = malloc(g->size * sizeof(t_bool *));
    assert(g->m != NULL);

    // Allocation et initialisation des lignes
    for (int i = 0; i < g->size; i++) {
        g->m[i] = malloc(g->size * sizeof(t_bool));
        assert(g->m[i] != NULL);
        for (int j = 0; j < g->size; j++) {
            g->m[i][j] = FAUX;
        }
    }
    return g;
}

void graph_free(t_graph * g) {
    if (g == NULL) return;
    for (int i = 0; i < g->size; i++) {
        free(g->m[i]);
    }
    free(g->m);
    free(g);
}

void graph_add_edge(t_graph * g, t_vertex src, t_vertex dest) {
    if (src >= 0 && src < g->size && dest >= 0 && dest < g->size) {
        g->m[src][dest] = VRAI;
    }
}

t_graph * graph_read_from_file(FILE * fp) {
    int size, src, dest;
    if (fscanf(fp, "%d", &size) != 1) return NULL;

    t_graph * g = graph_new(size);
    while (fscanf(fp, "%d %d", &src, &dest) == 2) {
        graph_add_edge(g, src, dest);
    }
    return g;
}

void graph_write_dot(t_graph * g, FILE * fp) {
    fprintf(fp, "digraph G {\n");
    for (int i = 0; i < g->size; i++) {
        for (int j = 0; j < g->size; j++) {
            if (g->m[i][j]) {
                fprintf(fp, "  %d -> %d;\n", i, j);
            }
        }
    }
    fprintf(fp, "}\n");
}

t_graph * graph_transpose(t_graph * g) {
    t_graph * gt = graph_new(g->size);
    for (int i = 0; i < g->size; i++) {
        for (int j = 0; j < g->size; j++) {
            if (g->m[i][j]) {
                gt->m[j][i] = VRAI; // Inversion du sens
            }
        }
    }
    return gt;
}


// ======================================================================
//                       IMPLÉMENTATION PILES & LISTES
// ======================================================================

t_stack * stack_new() {
    t_stack * ps = malloc(sizeof(*ps));
    assert(ps != NULL);
    *ps = list_new();
    return ps;
}

void stack_show(t_stack * ps) { list_show(*ps); }

int stack_is_empty(t_stack * ps) { return list_is_empty(*ps); }

void stack_push(t_vertex e, t_stack * ps) { *ps = list_add_head(e, *ps); }

t_vertex stack_pop(t_stack * ps) {
    t_vertex res = stack_top(ps);
    *ps = list_remove_head(*ps);
    return res;
}

t_vertex stack_top(t_stack * ps) {
    assert(!list_is_empty(*ps));
    return list_get_first_val(*ps);
}

t_list list_new() { return NULL; }

t_list list_add_head(t_vertex e, t_list l) {
    t_node * n = malloc(sizeof(t_node));
    assert(n != NULL);
    n->val = e;
    n->p_next = l;
    return n;
}

int list_is_empty(t_list l) { return l == NULL; }

void list_show(t_list l) {
    t_node * n = l;
    while (n != NULL) {
        printf("%d -> ", n->val);
        n = n->p_next;
    }
    printf("NULL\n");
}

t_vertex list_get_first_val(t_list l) {
    assert(l != NULL);
    return l->val;
}

t_list list_remove_head(t_list l) {
    assert(l != NULL);
    t_list l2 = l->p_next;
    free(l);
    return l2;
}

t_list list_free(t_list l) {
    t_node * n = l;
    while (n != NULL) {
        t_node * next = n->p_next;
        free(n);
        n = next;
    }
    return NULL;
}


// ======================================================================
//                       PARTIE 2 : RECHERCHE CHEMIN
// ======================================================================

// Version Itérative
void recherche_iter(t_graph *g, t_vertex x, t_vertex y) {
    t_stack *stack_traversal = stack_new();
    t_stack *stack_path = stack_new();
    t_stack *stack_path_final = stack_new();
    
    // Allocation sûre (Tas)
    t_bool *marking = calloc(g->size, sizeof(t_bool));
    assert(marking != NULL);

    stack_push(x, stack_traversal);
    t_bool found = FAUX;

    while (!stack_is_empty(stack_traversal)) {
        x = stack_pop(stack_traversal);
        
        if (x == y) {
            stack_push(x, stack_path);
            found = VRAI;
            break;
        } else if (x == -1) {
            // Marqueur de retour arrière : on dépile le chemin car ce n'était pas la bonne voie
            if(!stack_is_empty(stack_path)) stack_pop(stack_path);
        } else {
            if (marking[x] == FAUX) {
                marking[x] = VRAI;
                stack_push(x, stack_path);
                stack_push(-1, stack_traversal); // On place un marqueur pour le backtrack
                
                // Empiler les successeurs
                for (int w = 0; w < g->size; w++) {
                    if (g->m[x][w]) {
                        stack_push(w, stack_traversal);
                    }
                }
            }
        }
    }

    if (found) {
        // Inversion de la pile pour affichage dans l'ordre
        while (!stack_is_empty(stack_path)) {
            stack_push(stack_pop(stack_path), stack_path_final);
        }
        while (!stack_is_empty(stack_path_final)) {
            printf("%d ", stack_pop(stack_path_final));
        }
        printf("\n");
    } else {
        printf("Pas de chemin trouve.\n");
    }

    free(marking);
    free(stack_traversal); free(stack_path); free(stack_path_final);
}

// Version Récursive (Fonction Wrapper)
t_bool Recherche_recur(t_graph *g, t_vertex x, t_vertex y, t_stack *stack) {
    t_bool *marking = calloc(g->size, sizeof(t_bool));
    *stack = list_new();
    
    t_bool res = Recherche_recur_f(g, x, y, marking, stack);
    
    free(marking);
    return res;
}

// Version Récursive (Coeur de l'algo)
t_bool Recherche_recur_f(t_graph *g, t_vertex x, t_vertex y, t_bool *marking, t_stack *stack) {
    if (x == y) {
        stack_push(x, stack);
        return VRAI;
    }
    
    if (marking[x] == FAUX) {
        marking[x] = VRAI;
        for (int w = 0; w < g->size; w++) {
            if (g->m[x][w] == VRAI) { // Si voisin
                if (Recherche_recur_f(g, w, y, marking, stack)) {
                    stack_push(x, stack);
                    return VRAI;
                }
            }
        }
    }
    return FAUX;
}


// ======================================================================
//                       PARTIE 3 : KOSARAJU (CFC)
// ======================================================================

void enum_cfc_kosaraju(t_graph *g){
    int *order = malloc(g->size * sizeof(int));
    assert(order != NULL);

    // Etape 1 : Calcul de l'ordre suffixe
    Kosaraju_1(g, order);
    
    // Etape 2 : Transposée
    t_graph * h = graph_transpose(g);
    
    // Etape 3 : Parcours sur transposée
    Kosaraju_2(h, order);
    
    // Nettoyage
    graph_free(h);
    free(order);
}

void Kosaraju_1(t_graph *g, int *order){
    t_bool *marking = calloc(g->size, sizeof(t_bool));
    int step = 0;
    
    // On lance le DFS sur tous les nœuds non visités
    for (int x = 0; x < g->size; x++) {
         if (marking[x] == FAUX) {
            step = Kosaraju_1_recur(g, x, marking, order, step);
         }
    }
    free(marking);
}

int Kosaraju_1_recur(t_graph *g, int x, t_bool * marking, int * order, int step){
    if(marking[x] == FAUX){
        marking[x] = VRAI;
        for (int y = 0; y < g->size; y++) {
            if (g->m[x][y]) {
                step = Kosaraju_1_recur(g, y, marking, order, step);
            }
        }
        order[x] = step; // Enregistrement de la date de fin (ordre suffixe)
        step++;
    }
    return step;
}

void Kosaraju_2(t_graph *g, int *order) {
    t_bool *marking = calloc(g->size, sizeof(t_bool));
    int *inv_order = malloc(g->size * sizeof(int));
    int nb_scc = 0;

    // On inverse le tableau order pour avoir l'ordre décroissant de fin
    for (int x = 0; x < g->size; x++) {
        inv_order[(g->size - 1) - order[x]] = x;
    }

    printf("Liste des Composantes Fortement Connexes :\n");
    for (int i = 0; i < g->size; i++) {
        int x = inv_order[i]; 
        // Si le nœud n'est pas encore dans une CFC, on lance un DFS
        if (Kosaraju_2_recur(g, x, marking)) {
            nb_scc++;
            printf("\n");
        }
    }
    printf("\nTotal : %d composantes trouvees.\n", nb_scc);

    free(marking);
    free(inv_order);
}

t_bool Kosaraju_2_recur(t_graph *g, int x, t_bool *marking) {
    if (marking[x] == VRAI) return FAUX;

    marking[x] = VRAI;
    printf("%d ", x); // Affichage du membre de la CFC

    for (int y = 0; y < g->size; y++) {
        if (g->m[x][y]) {
            Kosaraju_2_recur(g, y, marking);
        }
    }
    return VRAI;
}