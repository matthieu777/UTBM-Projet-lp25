#include "defines.h"
#include <string.h>
#include <stdio.h>

/*!
 * @brief concat_path concatenates suffix to prefix into result
 * It checks if prefix ends by / and adds this token if necessary
 * It also checks that result will fit into PATH_SIZE length
 * @param result the result of the concatenation
 * @param prefix the first part of the resulting path
 * @param suffix the second part of the resulting path
 * @return a pointer to the resulting path, NULL when concatenation failed
 */
char *concat_path(char *result, char *prefix, char *suffix) {

    if (result == NULL || prefix == NULL || suffix == NULL) {  // si les pramètres ne sont pas NULL
        return NULL;
    }

    int prefix_len = strlen(prefix);            //calcul longeur des 2 chaines
    int suffix_len = strlen(suffix);

    
    if (prefix_len + suffix_len + 1 > PATH_SIZE) {          //verification que la taille ne depasse pas Path_size
        return NULL;                                        // sinon echec
    }

    strcpy(result, prefix);     //copie du prefix dans le resultat


    if (result[prefix_len - 1] != '/') {        //ajout de '/' si pas deja present
        strcat(result, "/");
    }

    
    strcat(result, suffix);         //ajout du suffix

    return result;
}



/*
// test a sup : 



int main() {

    char result[PATH_SIZE];
    char prefix[] = "/test/test2/";
    char suffix[] = "test.txt";

    char *concatenated_path = concat_path(result, prefix, suffix);

    if (concatenated_path != NULL) {
        printf("l'asemblage donne : %s\n", concatenated_path);
    } else {
        printf("echec \n");
    }

    return 0;
}

*/