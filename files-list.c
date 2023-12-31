#include <sys/stat.h>
#include "files-list.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>


/*
//pour le test de md5 (a supp) : 
#include <openssl/evp.h>
int compute_file_md5(files_list_entry_t *entry);
*/


/*!
 * @brief clear_files_list clears a files list
 * @param list is a pointer to the list to be cleared
 * This function is provided, you don't need to implement nor modify it
 */
void clear_files_list(files_list_t *list) {
    while (list->head) {
        files_list_entry_t *tmp = list->head;
        list->head = tmp->next;
        free(tmp);
    }
}

/*!
 *  @brief add_file_entry adds a new file to the files list.
 *  It adds the file in an ordered manner (strcmp) and fills its properties
 *  by calling stat on the file.
 *  Il the file already exists, it does nothing and returns 0
 *  @param list the list to add the file entry into
 *  @param file_path the full path (from the root of the considered tree) of the file
 *  @return 0 if success, -1 else (out of memory)
 */
files_list_entry_t *add_file_entry(files_list_t *list, char *file_path) {

    if (!list || !file_path) {
        printf("Invalide parameter\n");
        return NULL;
    }


    files_list_entry_t *new_entry = (files_list_entry_t *)malloc(sizeof(files_list_entry_t)); //allocation de la memoire

    if (!new_entry) {               //si out of memory
        printf("out of memory\n");
        return NULL;                    // NULL et pas -1 car doit retourner un pointeur
    }

    

    strcpy(new_entry->path_and_name, file_path);            //recuperation path and name
    

    files_list_entry_t *current = list->head;


    while (current != NULL && strcmp(file_path, current->path_and_name) > 0) {      //parcours la liste tant que le fichier est inferieur alphabétiquement 
        current = current->next;
    }



    if (current != NULL) {                          //insertion dans la liste
        new_entry->next = current;
        new_entry->prev = current->prev;
        if (current->prev != NULL) {
            current->prev->next = new_entry;
        } else {
            list->head = new_entry;
        }
        current->prev = new_entry;
    } else {                                                                    
        int result = add_entry_to_tail(list, new_entry);        // ajout en fin de liste
        if (result == -1) {
            printf("erreur dans l'ajout");
            free(new_entry);
            return NULL;
        }
    }

    return new_entry;


}


/*!
 * @brief add_entry_to_tail adds an entry directly to the tail of the list
 * It supposes that the entries are provided already ordered, e.g. when a lister process sends its list's
 * elements to the main process.
 * @param list is a pointer to the list to which to add the element
 * @param entry is a pointer to the entry to add. The list becomes owner of the entry.
 * @return 0 in case of success, -1 else
 */
int add_entry_to_tail(files_list_t *list, files_list_entry_t *entry) {
    if (!list || !entry) {                  // si il manque un paramètre
        printf("erreur dans le paramètrage\n");
        return -1;
    }

    
    if (!list->head) {              //dans le cas ou la liste est null on meme le fichier au debut
        list->head = entry;
        list->tail = entry;
    } else {                        // sinon on le mets à la fin
        list->tail->next = entry;
        entry->prev = list->tail;
        list->tail = entry;
    }

    return 0;

}

/*!
 *  @brief find_entry_by_name looks up for a file in a list
 *  The function uses the ordering of the entries to interrupt its search
 *  @param list the list to look into
 *  @param file_path the full path of the file to look for
 *  @param start_of_src the position of the name of the file in the source directory (removing the source path)
 *  @param start_of_dest the position of the name of the file in the destination dir (removing the dest path)
 *  @return a pointer to the element found, NULL if none were found.
 */
files_list_entry_t *find_entry_by_name(files_list_t *list, char *file_path, size_t start_of_src, size_t start_of_dest) {

    if (list == NULL || file_path == NULL) {            //test si la liste est vide
        return NULL;
    }


    files_list_entry_t* current = list->head;           //on se positionne sur la tete
    while (current != NULL) {                           //parcours de la liste 
        size_t len_in_list = strlen(current->path_and_name + start_of_src);
        size_t len_searched = strlen(file_path + start_of_dest);
        if (len_in_list > len_searched) {
            return NULL;
        }
        if (strcmp(current->path_and_name + start_of_src, file_path + start_of_dest) == 0) {
            return current;
        }
        current = current->next;                    //on passe au suivant
    }
    return NULL;                                    //sinon NULL
}

/*!
 * @brief display_files_list displays a files list
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list(files_list_t *list) {
    if (!list)
        return;
    
    for (files_list_entry_t *cursor=list->head; cursor!=NULL; cursor=cursor->next) {
        printf("%s\n", cursor->path_and_name);
    }
}

/*!
 * @brief display_files_list_reversed displays a files list from the end to the beginning
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list_reversed(files_list_t *list) {
    if (!list)
        return;
    
    for (files_list_entry_t *cursor=list->tail; cursor!=NULL; cursor=cursor->prev) {
        printf("%s\n", cursor->path_and_name);
    }
}













// a sup juste pour test :



// test de la liste : 
 /*
int main() {
    //creer list vide :  
    files_list_t list;
    list.head = NULL;
    list.tail = NULL;

    // add fichier
    char test[] = "test";
    char test1[] = "test/atest.txt";
    char test2[] = "test/test.txt";
    char test3[] = "test/test2";
    char test4[] = "test/test2/btest3";
    files_list_entry_t *file1 = add_file_entry(&list,test);
    files_list_entry_t *file2 = add_file_entry(&list,test1);
    files_list_entry_t *file3 = add_file_entry(&list,test2);
    files_list_entry_t *file4 = add_file_entry(&list,test3);
    files_list_entry_t *file5 = add_file_entry(&list,test4);
    

    // affichage 
    printf("\n \nListe :\n");
    display_files_list(&list);

    
    return 0;
}
*/



//------------------------------------------------------------------------------------------------

/*


// test des propreties et md5: 

int get_file_stats(files_list_entry_t *entry) {
    
    
    struct stat stats;

    char* path = entry->path_and_name;              //pat_and_name

    if (lstat(path, &stats) == -1) {            //verification que stat marche sur le path 
        return -1;
    }

    if (S_ISDIR(stats.st_mode)) {                   // si c'est un dossier 


        entry->entry_type = DOSSIER;                //entry_type
        
        entry->mode = stats.st_mode;                //mode



    } else if (S_ISREG(stats.st_mode)) {               //sinon si c'est un fichier

        entry->mtime.tv_nsec = stats.st_mtime;              //m_time en nano 

        entry->size = stats.st_size;                    //size

        if (compute_file_md5(entry) == -1) {
            return -1;
        }

        entry->entry_type = FICHIER;                   //entry_type

        entry->mode = stats.st_mode;                //mode

    } else {                                        //sinon probleme
        return -1;
    }

    return 0;
    
    
}



int compute_file_md5(files_list_entry_t *entry) {
    
    if (entry->entry_type != FICHIER || !entry) {          //si ce n'est pas un fichier ou la liste est
        return -1;
    }

    FILE *file = fopen(entry->path_and_name, "rb"); // ouverture du fichier
    if (!file) {        //si l'ouverture n'a pas marché
        return -1;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();   //creer le contexte
    const EVP_MD *md = EVP_md5();           // utilisation de l'algorithme MD5 de evp.h

    if ((!mdctx || !md)||(1 != EVP_DigestInit_ex(mdctx, md, NULL))) {       //vérifie que la creation du contexte et l'initialisation de l'algo
        EVP_MD_CTX_free(mdctx);             //on libere la memoire
        fclose(file);                       //on ferme le fichier
        return -1;
    }

    unsigned char buffer[1024];             //pour lire le fichier 
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {         //lis le fichier par morceaux
        if (1 != EVP_DigestUpdate(mdctx, buffer, bytes)) {      //met à jour le contexte avec les donné lues
            EVP_MD_CTX_free(mdctx);                     //on libere la memoire
            fclose(file);                   //on ferme le fichier
            return -1;
        }
    }

    unsigned int md_len;                //pour stocker la longeur de la somme md5
    if (1 != EVP_DigestFinal_ex(mdctx, entry->md5sum, &md_len)) {           //calcul du md5 et stockage
        EVP_MD_CTX_free(mdctx);             //on libere la memoire
        fclose(file);               //on ferme le fichier
        return -1;
    }

    EVP_MD_CTX_free(mdctx);         //on libere la memoire
    fclose(file);               //on ferme le fichier

    return 0;

}

// test d'ajout de donner : 
int main() {
    
    files_list_t list;          //creer list
    list.head = NULL;
    list.tail = NULL;

    
    char test_file[] = "test/test.txt";                 //ajt de fichier
    files_list_entry_t *file_entry = add_file_entry(&list, test_file);

    // affichage avant d'ajt les donner 
    printf("avant:\n");
    printf("path_and_name: %s\n", file_entry->path_and_name);
    printf("mtime: %ld nanoseconds\n", file_entry->mtime.tv_nsec);
    printf("size: %lu octets\n", file_entry->size);
    printf("mode: %o\n", file_entry->mode);
    printf("entry_type: %s\n", (file_entry->entry_type == FICHIER) ? "FICHIER" : "DOSSIER");

    
    get_file_stats(file_entry);     // ajt des donné

    //affichage apres l'ajt
    printf("\n apres:\n");
    printf("path_and_name: %s\n", file_entry->path_and_name);
    printf("mtime: %ld nanoseconds\n", file_entry->mtime.tv_nsec);
    printf("size: %lu octets\n", file_entry->size);
    printf("mode: %o\n", file_entry->mode);
    printf("entry_type: %s\n", (file_entry->entry_type == FICHIER) ? "FICHIER" : "DOSSIER");

     if (file_entry->entry_type == FICHIER) {
        printf("MD5 sum: ");
        for (int i = 0; i < 16; ++i) {
            printf("%02x", file_entry->md5sum[i]);
        }
        printf("\n");
    }

    clear_files_list(&list);

    return 0;
}


*/