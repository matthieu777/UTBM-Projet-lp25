#include "sync.h"
#include <dirent.h>
#include <string.h>
#include "processes.h"
#include "utility.h"
#include "messages.h"
#include "file-properties.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/msg.h>

#include <stdio.h>
#include <stdlib.h>


#include <utime.h>
#include <errno.h>
//a supp : 

#include <openssl/evp.h>
#include <assert.h>
#include "defines.h"

#include "configuration.h"
//gcc -o sync sync.c -lssl -lcrypto



/*!
 * @brief synchronize is the main function for synchronization
 * It will build the lists (source and destination), then make a third list with differences, and apply differences to the destination
 * It must adapt to the parallel or not operation of the program.
 * @param the_config is a pointer to the configuration
 * @param p_context is a pointer to the processes context
 */

// Cette fonction synchronise les fichiers entre une source et une destination.
void synchronize(configuration_t *the_config, process_context_t *p_context) {
    // Initialisation des listes de fichiers source et destination
    files_list_t src_list;
    src_list.head = src_list.tail = NULL;

    files_list_t dest_list;
    dest_list.head = dest_list.tail = NULL;

    // Construire les listes de fichiers source et destination
    make_files_list(&src_list, the_config->source);
    make_files_list(&dest_list, the_config->destination);

    // Comparaison et synchronisation des fichiers
    files_list_entry_t *src_entry = src_list.head;

    while (src_entry != NULL) {
        // Recherche de l'entrée correspondante dans la liste de destination
        files_list_entry_t *dest_entry = dest_list.head;
        while (dest_entry != NULL && strcmp(dest_entry->path_and_name, src_entry->path_and_name) != 0) {
            dest_entry = dest_entry->next;
        }

        // Copier le fichier source s'il n'existe pas dans la destination
        if (dest_entry == NULL) {
            copy_entry_to_destination(src_entry, the_config);
        } else {
            // Vérifier les différences et mettre à jour si nécessaire
            if (mismatch(src_entry, dest_entry, the_config->uses_md5)) {
                copy_entry_to_destination(src_entry, the_config);
            }
        }

        src_entry = src_entry->next;
    }

    // Nettoyage - Libérer la mémoire utilisée pour les listes de fichiers
    files_list_entry_t *src_temp = src_list.head;
    while (src_temp != NULL) {
        files_list_entry_t *next = src_temp->next;
        free(src_temp);
        src_temp = next;
    }

    files_list_entry_t *dest_temp = dest_list.head;
    while (dest_temp != NULL) {
        files_list_entry_t *next = dest_temp->next;
        free(dest_temp);
        dest_temp = next;
    }
}

/*!
 * @brief mismatch tests if two files with the same name (one in source, one in destination) are equal
 * @param lhd a files list entry from the source
 * @param rhd a files list entry from the destination
 * @has_md5 a value to enable or disable MD5 sum check
 * @return true if both files are not equal, false else
 */
bool mismatch(files_list_entry_t *lhd, files_list_entry_t *rhd, bool has_md5) {
    
    if (lhd->entry_type != rhd->entry_type) {           //test si de meme type
        return true;
    }

  
    if (lhd->size != rhd->size) {           //test si meme taille
        return true;
    }

   
    if (lhd->mtime.tv_nsec != rhd->mtime.tv_nsec) {             //test si la date de modification est la meme
        return true;
    }

    
    if (has_md5) {          //si le parm md5 est true
        if (memcmp(lhd->md5sum, rhd->md5sum, sizeof(lhd->md5sum)) != 0) {                   //compare les md5
            return true;
        }
    }

    return false;           //sinon il sont egale on retourne flase
}

/*!
 * @brief make_files_list buils a files list in no parallel mode
 * @param list is a pointer to the list that will be built
 * @param target_path is the path whose files to list
 */
void make_files_list(files_list_t *list, char *target_path) {

    make_list(list, target_path);          //construie la liste

    files_list_entry_t *current = list->head;
    while (current != NULL) {
        if (get_file_stats(current) == -1) {                //ajout des stats
            printf("erreur dans l'obtention des stats");
        }
        current = current->next;
    }
}

/*!
 * @brief make_files_lists_parallel makes both (src and dest) files list with parallel processing
 * @param src_list is a pointer to the source list to build
 * @param dst_list is a pointer to the destination list to build
 * @param the_config is a pointer to the program configuration
 * @param msg_queue is the id of the MQ used for communication
 */
void make_files_lists_parallel(files_list_t *src_list, files_list_t *dst_list, configuration_t *the_config, int msg_queue) {

}

/*!
 * @brief copy_entry_to_destination copies a file from the source to the destination
 * It keeps access modes and mtime (@see utimensat)
 * Pay attention to the path so that the prefixes are not repeated from the source to the destination
 * Use sendfile to copy the file, mkdir to create the directory
 */
void copy_entry_to_destination(files_list_entry_t *source_entry, configuration_t *the_config) {
    // Création du chemin de destination en utilisant le répertoire source et destination
    char destination_path[PATH_SIZE];
    strncpy(destination_path, source_entry->path_and_name, sizeof(destination_path));
    strncpy(destination_path, the_config->destination, strlen(the_config->destination));
    strncpy(destination_path + strlen(the_config->destination), source_entry->path_and_name + strlen(the_config->source), sizeof(destination_path) - strlen(the_config->destination));

    // Vérification s'il s'agit d'un dossier, création dans la destination si nécessaire
    if (source_entry->entry_type == DOSSIER) {
        if (mkdir(destination_path, source_entry->mode) == -1 && errno != EEXIST) {
            printf("Erreur lors de la création du répertoire");
            return;
        }
    } else { // Si c'est un fichier
        // Ouverture du fichier source en lecture
        int source_fd = open(source_entry->path_and_name, O_RDONLY);
        if (source_fd == -1) {
            printf("Erreur lors de l'ouverture du fichier source");
            return;
        }

        // Ouverture ou création du fichier destination en écriture
        int destination_fd = open(destination_path, O_WRONLY | O_CREAT | O_TRUNC, source_entry->mode);
        if (destination_fd == -1) {
            printf("Erreur lors de l'ouverture du fichier destination");
            close(source_fd);
            return;
        }

        // Copie du contenu du fichier source vers le fichier destination
        off_t offset = 0;
        ssize_t bytes_copied = sendfile(destination_fd, source_fd, &offset, source_entry->size);
        if (bytes_copied == -1) {
            printf("Erreur lors de la copie du fichier");
        }

        // Fermeture des descripteurs de fichiers
        close(source_fd);
        close(destination_fd);

        // Définition du temps de modification du fichier destination pour correspondre à celui du fichier source
        struct utimbuf times;
        times.actime = source_entry->mtime.tv_sec;
        times.modtime = source_entry->mtime.tv_sec;
        utime(destination_path, &times);
    }
}


/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, char *target) {
    DIR *dir = open_dir(target);            //ouvre le repertoire donné 

    if (dir == NULL) {          //si on ne parveient pas a ouvir le repertoir
        printf("erreur");  
    }

    struct dirent *entry;           //
    while ((entry = get_next_entry(dir)) != NULL) { //boucle tant que qu'il y'a des fichier dans le repertoire
        
        char full_path[PATH_SIZE];      //tableau pour stocker le chemin complet
        concat_path(full_path, target, entry->d_name);  //assemble le chemin target avec l'entry actuelle 


        if (directory_exists(full_path)) {      //si c'est un repertoire on recuse dedans pour traiter sont contenu
            make_list(list, full_path);
        } else {
            files_list_entry_t *new_entry = (files_list_entry_t *)malloc(sizeof(files_list_entry_t));       //aloue de la memoire dans la list chainer 
            if (new_entry != NULL) {                                                                        //si l'allocution a marché
                snprintf(new_entry->path_and_name, sizeof(new_entry->path_and_name), "%s", full_path);          //copie de full_path dans path_and name 
                
                new_entry->next = NULL;                             //initialise next à NULL
                new_entry->prev = list->tail;                       //initialise prev au pointeur de la queue de la list

                
                if (list->tail != NULL) {               //si la liste n'est pas vide 
                    list->tail->next = new_entry;          //queue de du dernier element de la liste pointe vers le nouveau noeud 
                } else {                                //si elle est vide 
                    list->head = new_entry;             //la tete de la liste pointe sur le nouvelle elem
                }
                list->tail = new_entry;                 //le pointeur de la queue de la liste pointe sur le nouvelle elem 
            }
        }
    }
    closedir(dir);          //fermeture du repertoire
}






/*! 
 * @brief open_dir opens a dir
 * @param path is the path to the dir
 * @return a pointer to a dir, NULL if it cannot be opened
 */
DIR *open_dir(char *path) {
    DIR *dir = opendir(path);                   //ouveture du dir 
    if (dir == NULL) {                      //si l'ouverture n'a pas marché
        return NULL;
    }
    return dir;                       
}

/*!
 * @brief get_next_entry returns the next entry in an already opened dir
 * @param dir is a pointer to the dir (as a result of opendir, @see open_dir)
 * @return a struct dirent pointer to the next relevant entry, NULL if none found (use it to stop iterating)
 * Relevant entries are all regular files and dir, except . and ..
 */
struct dirent *get_next_entry(DIR *dir) {
    
    struct dirent *entry; //stock l'entré actuel 

    while ((entry = readdir(dir)) != NULL) {   //si on peut lire

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {  //si c'est . ou .. on ignore 
            continue;
        }


        return entry;
    }

    
    return NULL;

}








//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//test à supp : 

//pour test make list: 
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



int get_file_stats(files_list_entry_t *entry) {
    
    
    struct stat stats;

    char* path = entry->path_and_name;              //pat_and_name

    if (lstat(path, &stats) == -1) {            //veerification que stat marche sur le path 
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




/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    DIR *dir = opendir(path_to_dir);

    if (dir != NULL) {
        closedir(dir);      //on le ferme 
        return true;        //il existe
    } else {
        return false;       //sinon il existe pas
    }
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

    // (compilation avec gcc -o file-properties file-properties.c -lssl -lcrypto)
}










int main() {


     /*
    char path[] = "test/test";
    DIR *directory = open_dir(path);


   
    // test de opendir

    if (directory != NULL) {
        printf("ouverture à marché\n");
    } else {
        printf("ouverture à echoué\n");
    }
    */

   /*
   //test de get_next_entry
    if (directory != NULL) {
        struct dirent *entry;
        while ((entry = get_next_entry(directory)) != NULL) {
            printf("trouvé : %s\n", entry->d_name);
        }
    } else {
        printf("rien trouver\n");
    }

    return 0;
    */




/*
   //test de make_list 
   files_list_t my_list;
    my_list.head = NULL;
    my_list.tail = NULL;

   
    char target_directory[] = "test";

   
    make_list(&my_list, target_directory); //construie la liste

    
    files_list_entry_t *current_entry = my_list.head; 
    while (current_entry != NULL) {                             // parcours de la liste
        printf("chemin: %s\n", current_entry->path_and_name);
        current_entry = current_entry->next;
    }


    return 0;
*/






/*

    //test de make_files_list 
   files_list_t my_list;
    my_list.head = NULL;
    my_list.tail = NULL;

   
    char target_directory[] = "test";

   
    make_files_list(&my_list, target_directory); //construie la liste

    
    files_list_entry_t *current_entry = my_list.head; 
    while (current_entry != NULL) {                             // parcours de la liste
        printf("chemin: %s\n", current_entry->path_and_name);
        printf("%lu \n", current_entry->size);
        current_entry = current_entry->next;
    }


    return 0;
*/


/*

//test de miss match 

    files_list_t source_list;
    files_list_t dest_list;

    char source[] = "test";
    char dest[] = "testbis";


    make_files_list(&source_list, source);
    make_files_list(&dest_list, dest);

    files_list_entry_t *source_entry = source_list.head;
    files_list_entry_t *dest_entry = dest_list.head;


    bool result = mismatch(source_entry, dest_entry, true);

    if (result) {
        printf("pas égaux.\n");
    } else {
        printf(" égaux.\n");
    }



    return 0;
*/




}
