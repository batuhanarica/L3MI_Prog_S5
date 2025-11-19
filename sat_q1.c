// #include "sat_q1.h"
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
} Variable;

typedef struct Clause {
    size_t size;
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

/**
 * Fonction pour allouer et intialiser une Clause.
 * @param size le nombre de littÃ©raux dans la clause
 * @param lits le tableau des littÃ©raux qui seront copiÃ©s dans la clause
 * @return un pointeur var la clause crÃ©Ã©e
 */
Clause* clause_new(size_t size, lit lits[static size]){

    //Create pointer to clause and if not created return NULL
    Clause* c = malloc(sizeof(Clause));
    if (!c){return NULL;}

    //Create place for lits and if not return NULL
    c->size = size;
    c->lits = malloc(size*sizeof(lit));
    if(!c->lits){free(c);return NULL;};

    //Put lits in clause c
    for(size_t i = 0 ; i < size ; i++){
        c->lits[i] = lits[i];
    }
    return c;


};

/**
 * Fonction pour allouer et intialiser une Formule.
 * Une formule aura une capacitÃ© de 10 clause et si elle devient pleine alors la capacitÃ© sera augmentÃ©e par incrÃ©ment de 10
 * @param nvars le nombre de variables utilisÃ©es dans la formule
 * @return un pointeur var la formule crÃ©Ã©e
 */
Formula* formula_new(size_t nvars){

    //Create the pointer for formula and if not return NULL
    Formula* f = malloc(sizeof(Formula));
    if (!f){return NULL;}

    //Init of needed params
    f->nvars = nvars;
    f->nclauses = 0;
    f->capacity = CAPACITY;

    //Create clauses of formula in size of capacity(10 by question) and if not return NULL
    f->clauses = (Clause**)malloc(f->capacity * sizeof(Clause*));
    if(!f->clauses){free(f);return NULL;};

    //Create assigned ta of formula in size of nb of variables and if not return NULL
    f->assigned = (var*)malloc(nvars*sizeof(var));
    if(!f->assigned){free(f->clauses);free(f);return NULL;};

    f->nassigned = (size_t)0;

    return f;
};

/**
 * Libère la mémoire allouée pour une formule et ses clauses.
 * @param f le pointeur vers la formule à libérer
 */
void formula_free(Formula* f){
    if(!f){ printf(("ERROR: F est vide "));return;}

    //For make free the memory of clauses
    for(size_t i = 0 ; i < f->nclauses; i++){
        Clause* c = f->clauses[i];
        if(c){
            free(c);
        }
    }

    //Free other params
    free(f->clauses);
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
