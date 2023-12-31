#include <sys/stat.h>
#include "file-properties.h"
#include <dirent.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "defines.h"
#include <fcntl.h>
#include <stdio.h>
#include "utility.h"

#include "configuration.h"





/*!
 * @brief get_file_stats gets all of the required information for a file (inc. directories)
 * @param the files list entry
 * You must get:
 * - for files:
 *   - mode (permissions)
 *   - mtime (in nanoseconds)
 *   - size
 *   - entry type (FICHIER)
 *   - MD5 sum
 * - for directories:
 *   - mode
 *   - entry type (DOSSIER)
 * @return -1 in case of error, 0 else
 */
int get_file_stats(files_list_entry_t *entry) {
    
    
    struct stat stats;

    const char* path = entry->path_and_name;              //pat_and_name

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



/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
*/
int compute_file_md5(files_list_entry_t *entry) {

    if (!entry || entry->entry_type != FICHIER) { // verifie si entry est null ou si ce n'est pas un fichier
        return -1;
    }
    
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

 

/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    if (!path_to_dir) {
        return false; 
    }
    DIR *dir = opendir(path_to_dir);

    if (dir != NULL) {
        closedir(dir);      //on le ferme 
        return true;        //il existe
    } else {
        return false;       //sinon il existe pas
    }
}

/*!
 * @brief is_directory_writable tests if a directory is writable
 * @param path_to_dir the path to the directory to test
 * @return true if dir is writable, false else
 * Hint: try to open a file in write mode in the target directory.
*/
bool is_directory_writable(char *path_to_dir) {
    if (!path_to_dir) {
        return false; 
    }

    if (access(path_to_dir, W_OK) != -1) {          //test si on a l'access à l'ecriture avec unistd
        return true;                                
    } else {
        return false;
    }    
}

 






//test a supp : 


/*
int main() {


//test de la fonction is 3 et 4 :        
    char chemin[] = "test";

    if (directory_exists(chemin)) {
        printf("le dossier existe\n");
    } else {
        printf("le dossier n'existe pas\n");
    };
     if (is_directory_writable(chemin)) {
        printf("le dossier est écrivable\n");
    } else {
        printf("le dossier n'est pas écrivable\n");
    };



    return 0;
}
*/