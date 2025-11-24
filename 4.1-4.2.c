
 #include "sat_q1.h"
#include "sat_q1.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 10
typedef int32_t lit;
typedef int16_t var;


typedef struct Variable {
    int8_t assign; // -1 unassigned, 0 false, 1 true
    WatchNode* pos;
    WatchNode* nega;
} Variable;

typedef struct Clause {
    size_t size;
    size_t wid1;   // indice1 literale surveille
    size_t wid2;   // indice2 litterale surveille
    lit lits[];    
} Clause;

typedef struct Formula {
    size_t nvars;
    size_t nclauses;
    size_t capacity;

    Clause** clauses;
    Variable* vars;

    var* assigned;
    size_t nassigned;
} Formula;

typedef struct WatchNode {
    Clause* clause;
    WatchNode* next;
    size_t lit_idx;
} WatchNode;


/**
 * Fonction pour allouer et intialiser une Clause.
 * @param size le nombre de littÃ©raux dans la clause
 * @param lits le tableau des littÃ©raux qui seront copiÃ©s dans la clause
 * @return un pointeur var la clause crÃ©Ã©e
 */
Clause* clause_new(size_t size, lit lits[static size]) {

    // Allocation d'un bloc unique pour Clause + lits[]
    Clause* c = malloc(sizeof(Clause) + size * sizeof(lit));
    if (!c) return NULL;

    c->size = size;

    // Choix automatique des deux premiers littéraux
    if (size == 1) {
        c->wid1 = 0;
        c->wid2 = 0;
    } else {
        c->wid1 = 0;
        c->wid2 = 1;
    }

      //Put lits in clause c
    for (size_t i = 0; i < size; i++)
        c->lits[i] = lits[i];

    return c;
}

/**
 * Fonction pour allouer et intialiser une Formule.
 * Une formule aura une capacitÃ© de 10 clause et si elle devient pleine alors la capacitÃ© sera augmentÃ©e par incrÃ©ment de 10
 * @param nvars le nombre de variables utilisÃ©es dans la formule
 * @return un pointeur var la formule crÃ©Ã©e
 */

Formula* formula_new(size_t nvars){
    //Create the pointer for formula and if not return NULL
    Formula* f = malloc(sizeof(Formula));
    if (!f) return NULL;
     //Init of needed params
    f->nvars = nvars;
    f->nclauses = 0;
    f->capacity = CAPACITY;

    //Create clauses of formula in size of capacity(10 by question) and if not return NULL
    f->clauses = malloc(f->capacity * sizeof(Clause*));
    if (!f->clauses) { free(f); return NULL; }

    
    f->vars = malloc((nvars + 1) * sizeof(Variable));

    if (!f->vars) {
        free(f->clauses);
        free(f);
        return NULL;
    }

    // Initialisation des variables
    for (size_t i = 0; i <= nvars; i++) {
        f->vars[i].assign = -1;
        f->vars[i].pos = NULL;
        f->vars[i].nega = NULL;
    }

    //Create assigned ta of formula in size of nb of variables and if not return NULL
    f->assigned = (var*)malloc(nvars * sizeof(var));
    if (!f->assigned) {
        free(f->vars);
        free(f->clauses);
        free(f);
        return NULL;
    }

    f->nassigned = (size_t)0;
    return f;
}

/**
 * Libère la mémoire allouée pour une formule et ses clauses.
 * @param f le pointeur vers la formule à libérer
 */
void formula_free(Formula* f){
    if(!f){ printf(("ERROR: F est vide "));return;}

    //For make free the memory of clauses
    for (size_t i = 0; i < f->nclauses; i++) {
        Clause* c = f->clauses[i];
        if(c){
            free(c);
        }
    }

    free(f->clauses);
    free(f->assigned);
    free(f->vars);             
    free(f);
}


/**
 * Ajoute une clause à la formule.
 * Si la capacité de la formule est atteinte, elle est augmentée.
 * @param f le pointeur vers la formule
 * @param c le pointeur vers la clause à ajouter
 */

void formula_add_clause(Formula* f, Clause* c){
    //Check if formula f and clause c are not NULL
    if(!c){ printf("ERROR: C est vide "); return;}
    if(!f){
        printf("ERROR: formula NULL\n");
        return;
    }
    
    //Check if we need to increase capacity
    if(f->nclauses >= f->capacity){
        f->capacity += CAPACITY;
        f->clauses = realloc(f->clauses, f->capacity * sizeof(Clause*));
    }

     //Add clause c to formula f
    f->clauses[f->nclauses] = c;
    f->nclauses += 1;
    

    return;
}


/**
 * Ajoute une clause à la formule.
 * Si la capacité de la formule est atteinte, elle est augmentée.
 * @param f le pointeur vers la formule
 * @param c le pointeur vers la clause à ajouter
 */
void formula_add_clause(Formula* f, Clause* c){
    //Check if formula f and clause c are not NULL
    if(!c){ printf("ERROR: C est vide "); return;}
    if(!f){
        f = formula_new(0);
        f->clauses[0] = c;
        f->nclauses = 1;
        return;
    }

    //Check if we need to increase capacity
    if(f->nclauses >= f->capacity){
        f->capacity += CAPACITY;
        f->clauses = realloc(f->clauses, f->capacity * sizeof(Clause*));
    }

    //Add clause c to formula f
    f->clauses[f->nclauses] = c;
    f->nclauses += 1;

    return;
};


/**
 * Évalue un littéral dans le contexte de la formule.
 * @param f le pointeur vers la formule
 * @param l le littéral à évaluer
 * @return 1 si le littéral est vrai, 0 si faux, -1 si non assigné
 */
int8_t lit_eval(Formula* f, lit l){
    //Check if f is NULL
    if(!f){printf("ERROR: Formula nulle");return;}

    //Check is anything is assigned
    if(f->nassigned == 0){return -1;}

    //Check if l is in f and if yes return the value of literal
    for (size_t i = 0; i < f->nclauses; i++) {
        Clause* clause = f->clauses[i];
        for (size_t j = 0; j < clause->size; j++) {
            if (clause->lits[j] == l) {
                var v = abs(l);
                int8_t assign = f->vars[v].assign;
                if (assign == -1) return -1;
                if ((l > 0 && assign == 1) || (l < 0 && assign == 0)) return 1;
                else return 0;
            }
        }
    }

    //If not founded return -1
    return -1;
};


/**
 * Évalue une clause.
 * @param f le pointeur vers la formule
 * @param c le pointeur vers la clause à évaluer
 * @return 1 si la clause est vraie, 0 si fausse, -1 si indéterminée (non vraie et contient un littéral non assigné)
 */
int8_t clause_eval(Formula* f, Clause* c){
    //TODO: Verify if we should put -1 in case of error

    //Check if c is NULL
    if(!c){printf("ERROR: Clause nulle");return -1;}

    //Check if f is NULL
    if(!f){printf("ERROR: Formula nulle");return -1;}

    //Check each literal in clause c
    bool has_unassigned = false;
    for(size_t i = 0 ; i < c->size ; i++){
        int8_t eval = lit_eval(f, c->lits[i]);
        if(eval == 1){
        return 1; //Clause is true
        } else if(eval == -1){
            has_unassigned = true; //There is an unassigned literal
        }
    }
    if(has_unassigned){
        return -1; //Clause is not true and has unassigned literal
    } else {
        return 0; //Clause is false
    }
};


/**
 * Évalue si la formule est satisfaite par l'assignation courante.
 * @param f le pointeur vers la formule
 * @return true si la formule est vraie, false sinon (fausse ou incomplète)
 */
bool formula_eval(Formula* f){
    //Check if f is NULL
    if(!f){printf("ERROR: Formula nulle");return false;}

    //Check each clause in formula f
    for(size_t i = 0 ; i < f->nclauses ; i++){
        int8_t eval = clause_eval(f, f->clauses[i]);
        if(eval == 0){
            return false; //Formula is false
        } else if(eval == -1){
            return false; //Formula is not true
        }
    }
    return true; //Formula is true
};


/**
 * Assigne une valeur booléenne à une variable.
 * Met à jour le tableau des variables assignées.
 * @param f le pointeur vers la formule
 * @param v l'index de la variable à assigner
 * @param value la valeur à assigner (true ou false)
 * @return true si l'assignation a réussi ou est cohérente avec l'existante, false si conflit
 */
bool assign(Formula* f, var v, bool value){

    //Check if variable v is already assigned
    if(f->vars[v].assign != -1){
        if((f->vars[v].assign == 1 && value) || (f->vars[v].assign == 0 && !value)){
            return true;
        } else {
            return false;
        }
    }

    //Assign value to variable v
    f->vars[v].assign = value ? 1 : 0;

    //Update assigned array
    f->assigned[f->nassigned] = v;
    f->nassigned += 1;

    return true;
};

bool formula_naive_solve(Formula* f);








/**
 * Met à jour les listes de littéraux surveillés après l'assignation d'une variable.
 * @param f le pointeur vers la formule
 * @param v la variable qui vient d'être assignée
 */
void update_watched(Formula* f, var v){
    if(!f) { printf("Erreur: Formula nulle-\n"); 
        
        return ; 
    }

    // On parcourt d'abord la liste des watchers positifs de la variable v
    WatchNode* node = f->vars[v].pos;
    while(node){
        Clause* c = node->clause;
        size_t idx = node->lit_idx;

        // Vérifier si le littéral surveillé est vrai maintenant
        lit l = c->lits[idx];
        int8_t val = lit_eval(f, l);

        bool replaced = false;

        if(val != 1){ // si le littéral n'est pas vrai, on cherche un remplacement
            for(size_t i = 0; i < c->size; i++){
                if(i != c->wid1 && i != c->wid2){ // ignorer les watchers actuels 
                    int8_t eval = lit_eval(f, c->lits[i]);
                    if(eval != 0){ // littéral non faux
                        if(c->wid1 == idx) c->wid1 = i;
                        else c->wid2 = i;
                        replaced = true;
                    }
                }
            }

            // Si aucun remplacement trouvé
            if(!replaced){
                int8_t clause_val = clause_eval(f, c);
                if(clause_val == 0){
                    // Conflit détecté
                    printf("Conflict detected in clause\n");
                }
                // Sinon unité ou indéterminée, traitement possible ailleurs
            }
        }

        node = node->next;
    }

    // Maintenant, parcourir la liste des watchers négatifs de la variable v
    node = f->vars[v].nega;
    while(node){
        Clause* c = node->clause;
        size_t idx = node->lit_idx;

        lit l = c->lits[idx];
        int8_t val = lit_eval(f, l);

        bool replaced = false;

        if(val != 1){ // si le littéral n'est pas vrai, on cherche un remplacement
            for(size_t i = 0; i < c->size; i++){
                if(i != c->wid1 && i != c->wid2){
                    int8_t eval = lit_eval(f, c->lits[i]);
                    if(eval != 0){
                        if(c->wid1 == idx) c->wid1 = i;
                        else c->wid2 = i;
                        replaced = true;
                    }
                }
            }

            if(!replaced){
                int8_t clause_val = clause_eval(f, c);
                if(clause_val == 0){
                    printf("Conflict detected in clause\n");
                }
            }
        }

        node = node->next;
    }
}
#include <stdbool.h>
#include <stddef.h>

bool propagate(Formula* f, size_t from) {
    if (!f) return false; // sécurité

    // Parcourir la pile des assignations depuis l'indice 'from'
    for (size_t idx = from; idx < f->nassigned; idx++) {
        var v = f->assigned[idx];

        // Mettre à jour les littéraux positifs de v
        WatchNode* node = f->vars[v].pos;
        while (node) {
            Clause* c = node->clause;
            size_t lit_idx = node->lit_idx;

            int8_t val = lit_eval(f, c->lits[lit_idx]);

            if (val != 1) { // le littéral n'est pas vrai → on cherche à le remplacer
                bool replaced = false;
                for (size_t i = 0; i < c->size; i++) {
                    if (i != c->wid1 && i != c->wid2) {
                        int8_t eval = lit_eval(f, c->lits[i]);
                        if (eval != 0) { // non faux
                            if (c->wid1 == lit_idx) c->wid1 = i;
                            else c->wid2 = i;
                            replaced = true;
                            break;
                        }
                    }
                }
                if (!replaced) {
                    int8_t clause_val = clause_eval(f, c);
                    if (clause_val == 0) return false; // conflit détecté
                }
            }
            node = node->next;
        }

        // Mettre à jour les littéraux négatifs de v
        node = f->vars[v].nega;
        while (node) {
            Clause* c = node->clause;
            size_t lit_idx = node->lit_idx;

            int8_t val = lit_eval(f, c->lits[lit_idx]);

            if (val != 1) {
                bool replaced = false;
                for (size_t i = 0; i < c->size; i++) {
                    if (i != c->wid1 && i != c->wid2) {
                        int8_t eval = lit_eval(f, c->lits[i]);
                        if (eval != 0) {
                            if (c->wid1 == lit_idx) c->wid1 = i;
                            else c->wid2 = i;
                            replaced = true;
                            break;
                        }
                    }
                }
                if (!replaced) {
                    int8_t clause_val = clause_eval(f, c);
                    if (clause_val == 0) return false; // conflit détecté
                }
            }
            node = node->next;
        }
    }

    // Si on n'a détecté aucun conflit
    return true;
}
void backtrack(Formula* f, var v) {

    if (!f) return;

    int i = 0;
    int index;
    var varsup;
    Variable* variablesup;

    // Chercher v dans les variables déjà assignées
    while (i < f->nassigned && f->assigned[i] != v) {
        i = i + 1;
    }

    // Si la variable a été trouvée
    if (i < f->nassigned) {

        // Désassigner toutes les variables à partir de i
        for (index = i; index < f->nassigned; index++) {

            varsup = f->assigned[index];
            variablesup = &f->vars[varsup];
            variablesup->assign = -1;
        }

        // Réduire le compteur d’assignations
        f->nassigned = i;

    } else {

        // Sinon, message d'erreur
        printf("La variable n'a jamais ete assignee\n");
    }

    return;
}






/////////////////////
// Fonctions d'E/S
////////////////////

Formula* read_cnf(const char* fname)
{

    FILE* f = fopen(fname, "r");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    size_t nvars = 0, nclauses = 0;
    char line[4096]; // choix arbitraire d'un buffer de 4ko

    /* cerche la ligne qui commence par p et rÃ©cupÃ¨re le nb de variable et de clauses */
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == 'c')
            continue;
        if (line[0] == 'p') {
            if (sscanf(line, "p cnf %zu %zu", &nvars, &nclauses) == 2)
                break;
        }
    }
    if (nvars <= 0 || nclauses <= 0) {
        fprintf(stderr, "Invalid DIMACS header\n");
        exit(1);
    }

    Formula* res = formula_new(nvars);

    /* read clauses */
    size_t maxlen = nvars; // une clause a au plus nvar litteraux
    int32_t lits[maxlen];
    size_t cidx = 0;
    while (fgets(line, sizeof(line), f)) { // cidx < nclauses &&
        if (line[0] == 'c' || line[0] == 'p')
            continue;
        size_t lidx = 0;
        uint32_t lit = 1;
        char* pos = line;
        while (lidx < maxlen) {
            int n;
            if (sscanf(pos, " %d%n", &lit, &n) != 1)
                break;
            pos += n;
            if (lit == 0)
                break; // end of the clause
            lits[lidx++] = lit;
        }
        if (lidx >= maxlen) {
            fprintf(stderr, "Clause too long %zu (/%zu)\n", lidx, maxlen);
            exit(1);
        } else if (lidx == 0)
            continue; /* skip empty lines */
        cidx++;

        Clause* c = clause_new(lidx, lits);
        formula_add_clause(res, c);
    }
    if (cidx != nclauses) {
        fprintf(stderr, "Warning: read %zu clauses, header said %zu\n", cidx, nclauses);
    }
    fclose(f);
    return res;
}

int main(int argc, char* argv[argc])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.cnf\n", argv[0]);
        return 1;
    }
    Formula* f = read_cnf(argv[1]);
    // f->vars[3].assign = 0;
    // formula_print(f);

    printf("satisfiable=%c\n", formula_naive_solve(f) ? 'T' : 'F');

    formula_free(f);
    return 0;
}