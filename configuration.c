#include "configuration.h"
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef enum {DATE_SIZE_ONLY, NO_PARALLEL, DRY_RUN} long_opt_values; //JE RAJOUTE DRY-RUN

/*!
 * @brief function display_help displays a brief manual for the program usage
 * @param my_name is the name of the binary file
 * This function is provided with its code, you don't have to implement nor modify it.
 */
void display_help(char *my_name) {
    printf("%s [options] source_dir destination_dir\n", my_name);
    printf("Options: \t-n <processes count>\tnumber of processes for file calculations\n");
    printf("         \t-h display help (this text)\n");
    printf("         \t--date_size_only disables MD5 calculation for files\n");
    printf("         \t--no-parallel disables parallel computing (cancels values of option -n)\n");
     printf("        \t--dry-run pour exécution de test (juste lister les opérations à faire, ne pas faire les copies réellement)\n"); //ajout de dry run ici
}



/*!
 * @brief init_configuration initializes the configuration with default values
 * @param the_config is a pointer to the configuration to be initialized
 */
void init_configuration(configuration_t *the_config) {
    the_config->source[0] = '\0';
    the_config->destination[0] = '\0';  //on initialiser la source et la destination a une chaine vide de base

    the_config->processes_count = 1;   //on initialise à 1 processus 
    the_config->is_parallel = true;   // de base on calcul en parallèle 
    the_config->uses_md5 = true;       //de base on annalyse le md5

    the_config->verbose = false;
    the_config->dry_run = false;
}

/*!
 * @brief set_configuration updates a configuration based on options and parameters passed to the program CLI
 * @param the_config is a pointer to the configuration to update
 * @param argc is the number of arguments to be processed
 * @param argv is an array of strings with the program parameters
 * @return -1 if configuration cannot succeed, 0 when ok
 */
int set_configuration(configuration_t *the_config, int argc, char *argv[]) {
    int opt = 0;


    static struct option long_options[] = {
        {.name="date-size-only", .has_arg=0, .flag=0, .val= DATE_SIZE_ONLY},
        {.name="no-parallel", .has_arg=0, .flag=0, .val= NO_PARALLEL},
        {.name="dry-run", .has_arg=0, .flag=0, .val= DRY_RUN},
        {0, 0, 0, 0}
    };

    
    while ((opt = getopt_long(argc, argv, "n:vh", long_options, NULL)) != -1){ 
        switch (opt) {
            case 'n':
                the_config->processes_count = atoi(optarg);
                break;
            case 'v':
                the_config->verbose = true;
                break;
            case DATE_SIZE_ONLY:
                the_config->uses_md5 = false;
                break;
            case NO_PARALLEL:
                the_config->is_parallel = false;
                break;
            case DRY_RUN:
                the_config->dry_run = true;
                break;
            case 'h':
                display_help(argv[0]);
                return -1;
            case '?':
                display_help(argv[0]);
                return -1;                  // toute les autres donné sont pas ok 
                break;
            default:
                display_help(argv[0]);
                return -1;
        }
    }

    if (optind + 2 != argc) {
        display_help(argv[0]);              //verification qu'on à une source et un destination
        return -1;
    }
    
    strncpy(the_config->source, argv[optind], sizeof(the_config->source) - 1);          //copie du paramètre passer dans source (taille de -1 pour laisser une place pour \0)
    the_config->source[sizeof(the_config->source) - 1] = '\0';                      //ajout caraqtères de fin de chaine de caractères 

    strncpy(the_config->destination, argv[optind + 1], sizeof(the_config->destination) - 1);        //copie du paramètre passer dans destination
    the_config->destination[sizeof(the_config->destination) - 1] = '\0';


    return 0;

}














// A supp en-dessous c'est juste pour test: 


void print_configuration(const configuration_t *the_config) {
    printf("Config:\n");
    printf("source: %s\n", the_config->source);
    printf("dest: %s\n", the_config->destination);
    printf("nb proces: %u\n", the_config->processes_count);
    printf("parallel : %s\n", the_config->is_parallel ? "vrai" : "faux");
    printf("MD5 : %s\n", the_config->uses_md5 ? "vrai" : "faux");
    printf("verbosse : %s\n", the_config->verbose ? "vrai" : "faux");

}

int main(int argc, char *argv[]){
    printf("test");

    configuration_t my_config;

    init_configuration(&my_config);

    if (set_configuration(&my_config, argc, argv) == -1) {
        return 1;
    }
    print_configuration(&my_config);
    return 0;
    
}
