#include <stdio.h>

typedef int t_bool; // Booléen

typedef int t_vertex; // Sommet de graphe

// Maillon de liste chaînée
typedef struct node {
t_vertex val; // Valeur (sommet)
struct node * p_next; // Pointeur vers le maillon suivant
} t_node;

// Liste chaînée (de sommets)
typedef t_node * t_list;

//Liste d'adjacence
typedef struct {
int size; // Taille
t_list * m; // Contenu
} t_graph;



void enum_cfc_kosaraju(t_graph g);
void Kosaraju_1(t_graph g);
int Kosaraju_1_recur(t_graph g, int x, t_bool * marking, int * order, int step)



int main() {
	int i;
	int nb_nodes;
	// Allocation de la structure t_graph
	t_graph * pg = malloc(sizeof(*pg));
	assert(pg != NULL);
	pg->size = nb_nodes; // Où nb_nodes est le nombre de sommets
	// Allocation du tableau de têtes de listes d'adjacence
	pg->m = malloc(pg->size * sizeof(t_list));
	assert(pg->m != NULL);
	
	
	
	
	
	return 0;
}


void enum_cfc_kosaraju(t_graph g){
	int n = g.size;
	bool marking[n];
	int order[n];
//	1) Effectuer un parcours en profondeur sur g dans un ordre arbitraire et conserver l’ordre suffixe de parcours dans le tableau order
	Kosaraju_1(g);
//	2) h ← Calculer le graphe inverse de g
	h = Kosaraju_2(g, order);
//	ffectuer un parcours en profondeur sur g’ dans l’ordre inverse de celui donné par orde et afficher les sommets des sous-graphes parcourus depuis chaque sommet initial
	Kosaraju_1(h);
}


void Kosaraju_1(t_graph g){
	int x;		//Un sommet de g
	int step;	//Entier permettant de compter l’ordre suffixe
//	Initialiser marking à FAUX
	int i;
	for (i = 0; i < n; i++){
		marking[i] = 0;
	}
	
	step = 0;
	
	for (int x = 0; x < g->n; x++) {
    		if (!marking[x]) {
        		Kosaraju_1_recur(g, x, marking, order, &step);
    		}
	}
}


int Kosaraju_1_recur(t_graph g, int x, t_bool * marking, int * order, int step){
	if(marking[x] == 0){
		marking[x] = 1;
		for (int y ){
			
		}
		order[x] = step;
		step++
	}
	return step;
}


