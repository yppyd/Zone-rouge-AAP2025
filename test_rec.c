#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_READ_LINE 100
#define VRAI 1
#define FAUX 0

typedef int t_bool;    // Booléen
typedef int t_vertex;  // Sommet de graphe

// --- LISTES CHAÎNÉES & PILES ---
typedef struct node 
{
    t_vertex val;
    struct node * p_next;
} t_node;

typedef t_node * t_list;
typedef t_list t_stack;

// Listes d'adjacence
typedef struct 
{
    int size;      // Nombre de sommets
    t_list * l;    // Tableau de listes d'adjacence (tableau de têtes de listes)
} t_graph;

// --- PROTOTYPES ---
//Fonction de base 
int string_to_int(char *str);
// Gestion du Graphe
t_graph * graph_new(int size);
void graph_free(t_graph * g);
void graph_add_edge(t_graph * g, t_vertex src, t_vertex dest);
t_graph * graph_read_from_file(FILE * fp);
void graph_write_dot(t_graph * g, FILE * fp);
t_graph * graph_transpose(t_graph * g);

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

// Recherche de chemin
void recherche_iter(t_graph *g, t_vertex x, t_vertex y, FILE *out);
t_bool Recherche_recur_f(t_graph *g, t_vertex x, t_vertex y, t_bool *marking, t_stack *stack);
void Recherche_recur(t_graph *g, t_vertex x, t_vertex y, FILE *out);

// Kosaraju
void enum_cfc_kosaraju(t_graph *g, FILE *out);
void Kosaraju_1(t_graph *g, int *order);
int Kosaraju_1_recur(t_graph *g, int x, t_bool *marking, int *order, int step);
void Kosaraju_2(t_graph *g, int *order, FILE *out);
t_bool Kosaraju_2_recur(t_graph *g, int x, t_bool *marking, FILE *out);


// ======================================================================
//                                MAIN
// ======================================================================

int main(int argc, char *argv[]) { //création d'arguments pour les divers modes de fonctionnement du code 
     char *input_filename  = NULL;
    char *output_filename = NULL;
    int start_node        = -1;
    int goal_node         = -1;
    t_bool mode_cfc       = FAUX;
    t_bool mode_path      = FAUX;
    t_bool mode_rec       = FAUX
    // Lecture des arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_filename = argv[i + 1]; 
            i++;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_filename = argv[i + 1]; 
            i++;
        } else if (strcmp(argv[i], "-start") == 0 && i + 1 < argc) {
            start_node = string_to_int(argv[i + 1]); 
            mode_path = VRAI; 
            i++;
        } else if (strcmp(argv[i], "-goal") == 0 && i + 1 < argc) {
            goal_node = string_to_int(argv[i + 1]); 
            mode_path = VRAI; 
            i++;
        } else if (strcmp(argv[i], "-cfc") == 0) {
            mode_cfc = VRAI;
        } else if (strcmp(argv[i], "-r") == 0) {
            mode_rec = VRAI;
        } else {
            printf("Usage incorrect.\n"); return 1;
        }
    }

    // Lecture du fichier graphe
    FILE *in_stream = stdin;
    if (input_filename != NULL) { //choix du fichier + verif du fichier
        in_stream = fopen(input_filename, "r");
        if (in_stream == NULL) { perror("Erreur ouverture entree"); return 1; }
    }

    t_graph * g = graph_read_from_file(in_stream); //utilisation de la fonction de conversion en graphe du fichier 
    if (input_filename != NULL) fclose(in_stream);

    if (g == NULL) { 
        fprintf(stderr, "Echec lecture graphe.\n"); 
        return 1; 
    }

    FILE *out_stream = stdout; //gestion du fichier de sortie à peu près pareil qu'entrée  

    if (output_filename != NULL) {
        out_stream = fopen(output_filename, "w");
        if (out_stream == NULL) {
            perror("Erreur ouverture fichier sortie");
            graph_free(g);
            return 1;
        }
    }

    // --- choix du mode selon l'arg ---
    if (mode_cfc) {
        if(output_filename != NULL) printf("Calcul des CFC en cours...\n");
        enum_cfc_kosaraju(g, out_stream);
    } 
    else if (mode_path) {
        if (start_node == -1 || goal_node == -1) {
            fprintf(stderr, "Erreur : Arguments -start et -goal requis.\n");
        } else if (start_node < 0 || start_node >= g->size || goal_node < 0 || goal_node >= g->size) {
            fprintf(stderr, "Erreur : Sommets invalides.\n");
        } else {
            if(output_filename != NULL) printf("Recherche de chemin en cours...\n")
            {
                if(mode_rec){
                Recherche_recur(g, start_node, goal_node, out_stream);
                }
                else{
                    recherche_iter(g, start_node, goal_node, out_stream);
                }
            }
            
        }
    } 
    else {
        if(output_filename != NULL) printf("Generation du fichier DOT...\n");
        graph_write_dot(g, out_stream);
    }

    // Annonce du fichier écrit 
    if (out_stream != stdout) {
        printf("Resultat ecrit dans %s\n", output_filename);
        fclose(out_stream);
    }

    graph_free(g);
    return 0;
}


// ======================================================================
//                        IMPLÉMENTATION GRAPHES
// ======================================================================

t_graph * graph_new(int size) {
    t_graph * g = malloc(sizeof(t_graph));
    assert(g != NULL);
    g->size = size;

    // Allocation du tableau de listes
    g->l = malloc(g->size * sizeof(t_list));
    assert(g->l != NULL);

    // Initialisation de chaque liste
    for (int i = 0; i < g->size; i++) {
        g->l[i] = list_new();
    }
    return g;
}

void graph_free(t_graph * g) {
    if (g == NULL) return;
    // Libération de chaque liste chaînée
    for (int i = 0; i < g->size; i++) {
        list_free(g->l[i]);
    }
    free(g->l);
    free(g);
}

void graph_add_edge(t_graph * g, t_vertex src, t_vertex dest) {
    if (src >= 0 && src < g->size && dest >= 0 && dest < g->size) {
        // Ajout en tête de liste : complexité O(1)
        g->l[src] = list_add_head(dest, g->l[src]);
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
        // Parcours de la liste des voisins [cite: 59, 72]
        t_node * curr = g->l[i];
        while (curr != NULL) {
            fprintf(fp, "  %d -> %d;\n", i, curr->val);
            curr = curr->p_next;
        }
    }
    fprintf(fp, "}\n");
}

t_graph * graph_transpose(t_graph * g) {
    t_graph * gt = graph_new(g->size);
    for (int i = 0; i < g->size; i++) {
        t_node * curr = g->l[i];
        while (curr != NULL) {
            // Arc i -> curr->val devient curr->val -> i
            graph_add_edge(gt, curr->val, i);
            curr = curr->p_next;
        }
    }
    return gt;
}
// ======================================================================
//                        fonction de base 
// ======================================================================
int string_to_int(char *str) {
    int res = 0;
    for (int i = 0; str[i] != '\0'; i++) 
    {
        if (str[i] >= '0' && str[i] <= '9') {
            int chiffre = str[i] - '0';
            res = res * 10 + chiffre;
        }
    }
    return res;
}


// ======================================================================
//                        IMPLÉMENTATION PILES & LISTES
// ======================================================================

t_stack * stack_new() {
    t_stack * ps = malloc(sizeof(*ps));
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
t_list list_new() { 
    return NULL; 
}
t_list list_add_head(t_vertex e, t_list l) {
    t_node * n = malloc(sizeof(t_node));
    n->val = e; n->p_next = l;
    return n;
}
int list_is_empty(t_list l) { 
    return l == NULL; 
}
void list_show(t_list l) {
    t_node * n = l;
    while (n != NULL) { printf("%d -> ", n->val); n = n->p_next; }
    printf("NULL\n");
}
t_vertex list_get_first_val(t_list l) { assert(l != NULL); return l->val; }
t_list list_remove_head(t_list l) {
    t_list l2 = l->p_next; free(l); return l2;
}
t_list list_free(t_list l) {
    t_node * n = l;
    while (n != NULL) { t_node * next = n->p_next; free(n); n = next; }
    return NULL;
}


// ======================================================================
//                        PARTIE 2 : RECHERCHE CHEMIN
// ======================================================================

// Version Itérative 
void recherche_iter(t_graph *g, t_vertex x, t_vertex y, FILE *out) {
    t_stack *stack_traversal = stack_new();
    t_stack *stack_path = stack_new();
    t_stack *stack_path_final = stack_new();
    t_bool *marking = malloc(g->size * sizeof(t_bool));
    assert(marking != NULL);
    for (int i = 0; i < g->size; i++) marking[i] = FAUX;

    stack_push(x, stack_traversal);
    t_bool found = FAUX;

    while (!stack_is_empty(stack_traversal)) {
        x = stack_pop(stack_traversal);

        if (x == y) {
            stack_push(x, stack_path);
            found = VRAI;
            break;
        } else if (x == -1) {
            if(!stack_is_empty(stack_path)) stack_pop(stack_path);
        } else {
            if (marking[x] == FAUX) {
                marking[x] = VRAI;
                stack_push(x, stack_path);
                stack_push(-1, stack_traversal);

                // --- Adaptation Listes : Parcours des voisins ---
                t_node * curr = g->l[x];
                while (curr != NULL) {
                    stack_push(curr->val, stack_traversal);
                    curr = curr->p_next;
                }
            }
        }
    }

    if (found) {
        while (!stack_is_empty(stack_path)) stack_push(stack_pop(stack_path), stack_path_final);
        while (!stack_is_empty(stack_path_final)) {
            fprintf(out, "%d ", stack_pop(stack_path_final));
        }
        fprintf(out, "\n");
    } else {
        fprintf(out, "Pas de chemin trouve.\n");
    }

    free(marking);
    free(stack_traversal); free(stack_path); free(stack_path_final);
}

t_bool Recherche_recur_f(t_graph *g, t_vertex x, t_vertex y, t_bool *marking, t_stack *stack) 
{
    if (x == y) 
    {
        stack_push(x, stack);
        return VRAI;
    }
    if (marking[x] == FAUX) 
    {
        marking[x] = VRAI;
        // Parcours des voisins (Version Liste)
        t_node * curr = g->l[x];
        while (curr != NULL) 
        {
            if (Recherche_recur_f(g, curr->val, y, marking, stack)) 
            {
                stack_push(x, stack); // On empile x car il fait partie du chemin réussi
                return VRAI;
            }
            curr = curr->p_next;
        }
    }
    return FAUX;
}



// Fonction Principale recur

void Recherche_recur(t_graph *g, t_vertex x, t_vertex y, FILE *out) 
{
    t_bool *marking = malloc(g->size * sizeof(t_bool));
    assert(marking != NULL);
    for (int i = 0; i < g->size; i++) marking[i] = FAUX;
    t_stack stack = list_new();


    if (Recherche_recur_f(g, x, y, marking, &stack)) 
    {
        // Si trouvé : Affichage dans out 
        while (!stack_is_empty(&stack)) 
        {
            fprintf(out, "%d ", stack_pop(&stack));
        }
        fprintf(out, "\n");
    } 
    else 
    {
        // Sinon : Message d'erreur
        fprintf(out, "Pas de chemin trouve.\n");
    }
    free(marking);
}


// ======================================================================
//                        PARTIE 3 : KOSARAJU (CFC)
// ======================================================================

void enum_cfc_kosaraju(t_graph *g, FILE *out){
    // Allocation du graphe g
    int *order = malloc(g->size * sizeof(int));
    assert(order != NULL);
    
    /* Parcours en profondeur sur g dans un ordre arbitraire, 
    conservation de l’ordre des suffixes de parcours dans le tableau order */
    Kosaraju_1(g, order);
    
    /* Calcul du traphe inverse de g (transposition de la matrice d'adjacence) */
    t_graph * h = graph_transpose(g);
    
    /* Parcours en profondeur sur h dans l’ordre inverse de celui donné par order, 
    affichage des sommets des sous-graphes parcourus depuis chaque sommet initial */
    Kosaraju_2(h, order, out);

    // Libération des espaces alloués aux éléments h et order
    graph_free(h);
    free(order);
}

void Kosaraju_1(t_graph *g, int *order){
    //  Allocation et initialisation d'un tableau de booléens permettant de marquer les sommets déjà visités
    t_bool *marking = malloc(g->size * sizeof(t_bool));
    assert(marking != NULL);
    for (int i = 0; i < g->size; i++) marking[i] = FAUX;
    
    // Entier permettant de compter l’ordre suffixe
    int step = 0;
    
    // Parcours des sommets du graphe
    for (int x = 0; x < g->size; x++) {
         if (marking[x] == FAUX) {
            step = Kosaraju_1_recur(g, x, marking, order, step);
         }
    }
    free(marking);
}

int Kosaraju_1_recur(t_graph *g, int x, t_bool * marking, int * order, int step){
    // Si le sommet est déjà marqué, pas besoin de modifier la valeur de step
    if(marking[x] == FAUX){
        marking[x] = VRAI;

        // Parcours des successeurs de x
        t_node * curr = g->l[x];
        while (curr != NULL) {
            step = Kosaraju_1_recur(g, curr->val, marking, order, step);
            curr = curr->p_next;
        }

        order[x] = step;
        step++;
    }
    return step;
}

void Kosaraju_2(t_graph *g, int *order, FILE *out) {
    //  Allocation et initialisation d'un tableau de booléens permettant de marquer les sommets déjà visités
    t_bool *marking = malloc(g->size * sizeof(t_bool));
    assert(marking != NULL);
    for (int i = 0; i < g->size; i++) marking[i] = FAUX;

    // Allocation, initialisation et remplissage d'un tableau d’entiers permettant de stocker l’ordre inverse des suffixe
    int *inv_order = malloc(g->size * sizeof(int));
    for (int x = 0; x < g->size; x++) inv_order[(g->size - 1) - order[x]] = x;
    
    // Comptage des cfc
    int nb_scc = 0;
    fprintf(out, "Liste des Composantes Fortement Connexes :\n");
    for (int i = 0; i < g->size; i++) {
        int x = inv_order[i]; 
        if (Kosaraju_2_recur(g, x, marking, out)) {
            nb_scc++;
            fprintf(out, "\n");
        }
    }
    fprintf(out, "\nTotal : %d composantes trouvees.\n", nb_scc);
    
    // Libération des espaces alloués aux éléments marking et inv_order
    free(marking);
    free(inv_order);
}

t_bool Kosaraju_2_recur(t_graph *g, int x, t_bool *marking, FILE *out) {
    // Si le sommet est déjà marqué, pas besoin d'appliquer la fonction
    if (marking[x] == VRAI) return FAUX;

    marking[x] = VRAI;
    fprintf(out, "%d ", x);

    // Parcours des successeurs de x
    t_node * curr = g->l[x];
    while (curr != NULL) {
        Kosaraju_2_recur(g, curr->val, marking, out);
        curr = curr->p_next;
    }

    return VRAI;
}