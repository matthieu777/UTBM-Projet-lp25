#include "files-list.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>
#include <time.h>

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

    /*
    
    MANQUE LE TEST SI LE FICHIER EXISTE DEJA AVEC la fonction FIND ENTRY BY NAME
    
    */
    

    files_list_entry_t *new_entry = (files_list_entry_t *)malloc(sizeof(files_list_entry_t)); //allocation de la memoir

    if (!new_entry) {               //si out of memory
        printf("out of memory\n");
        return NULL;                    // NULL et pas -1 car doit retourner un pointeur
    }

    
    struct stat file_stat;                      //utilisation de stat pour recuperer les "poperties du fichier"


    if (stat(file_path, &file_stat) == -1) {    //test que stat marche bien 
        printf("stat n'a pas marché");
        free(new_entry);
        return NULL;
    }

    
    strcpy(new_entry->path_and_name, file_path);            //recuperation path and name
    
    new_entry->mtime = file_stat.st_mtim;           // mtime

    new_entry->size = file_stat.st_size;            //size


    /*
    
    MANQUE LE MD5
    
    */



    if (S_ISREG(file_stat.st_mode)) {                   //type 
        new_entry->entry_type = FICHIER;
    } else if (S_ISDIR(file_stat.st_mode)) {
        new_entry->entry_type = DOSSIER;
    } else {
        printf("erreur pas fichier et pas dossier");
    }


    new_entry->mode = file_stat.st_mode;            //mode
    

    new_entry->next = NULL;          
    new_entry->prev = NULL;
    

    
    int result = add_entry_to_tail(list, new_entry);        // ajout du fichier dans la list

    if (result == -1) {         //si l'ajout n'a pas marché
        printf("erreur lors de l'ajout\n");
        free(new_entry);
        return NULL;
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


int main() {
    //creer list vide :  
    files_list_t list;
    list.head = NULL;
    list.tail = NULL;

    // add fichier
    char test[] = "test.txt";
    char test2[] = "test2.txt";
    files_list_entry_t *file1 = add_file_entry(&list,test);
    files_list_entry_t *file2 = add_file_entry(&list,test2);
    

    // affichage 
    printf("Liste après l'ajout de fichiers :\n");
    display_files_list(&list);
    // affichage des propriété
    printf("path_and_name: %s\n", file1->path_and_name);
    printf("mtime: %ld seconds, %ld nanoseconds\n", file1->mtime.tv_sec, file1->mtime.tv_nsec);
    printf("size: %lu bytes\n", file1->size);
    switch (file1->entry_type) {
        case FICHIER:
            printf("type : Fichier\n");
        break;
        case DOSSIER:
            printf("type : Dossier\n");
        break;
        }
    printf("mode: %o\n", file1->mode);


    return 0;
}