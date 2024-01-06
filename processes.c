#include "processes.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include "messages.h"
#include "file-properties.h"
#include "sync.h"
#include <string.h>
#include <errno.h>

#include <sys/wait.h>
/*!
 * @brief prepare prepares (only when parallel is enabled) the processes used for the synchronization.
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the program processes context
 * @return 0 if all went good, -1 else
 */
int prepare(configuration_t *the_config, process_context_t *p_context) {
   if (the_config == NULL || !the_config->is_parallel) {
        return 0; 
    } else {
        pid_t pid = fork();
        if (pid == -1) {
            return -1; 
        } else if (pid == 0) {
            
           
            return 0;
        } else {
            p_context->source_lister_pid = pid;
            
        }

    return 0;
    }
}


/*!
 * @brief make_process creates a process and returns its PID to the parent
 * @param p_context is a pointer to the processes context
 * @param func is the function executed by the new process
 * @param parameters is a pointer to the parameters of func
 * @return the PID of the child process (it never returns in the child process)
 */
int make_process(process_context_t *p_context, process_loop_t func, void *parameters) {
     pid_t child_pid = fork(); // creation du processus enfant

     if (child_pid == -1){
        printf("errreur lors de la creation du processus");
        exit(EXIT_FAILURE);
     } else if ( child_pid == 0){
        func(parameters); // appel de la fonction pour le processus enfant 
        exit(EXIT_SUCCESS); // le processus enfant ce fini
     } else {
        p_context->processes_count++; //on ajout 1 au nombre de processus
        return child_pid; //donne le pid du processus enfant au parent 
     }


}

/*!
 * @brief lister_process_loop is the lister process function (@see make_process)
 * @param parameters is a pointer to its parameters, to be cast to a lister_configuration_t
 */
void lister_process_loop(void *parameters) {
    lister_configuration_t *config = (lister_configuration_t *)parameters; //cast des paramètre vers lister_configuration_t 


}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */
void analyzer_process_loop(void *parameters) {
    analyzer_configuration_t *config = (analyzer_configuration_t *)parameters; //cast des paramètre vers lister_configuration_t  analyzer_configuration_t 
}

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting to the confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    // Do nothing if not parallel
     if (!the_config->is_parallel) {
        return;
    }
    // Send terminate

     for (int i = 0; i < p_context->processes_count; ++i) { //boucle pour récuprer chaque pid
        int pid = p_context->destination_analyzers_pids[i]; //recupere le pid stocker
        if (pid > 0) { //si c'est un pid valide
            send_terminate_command(p_context->message_queue_id, pid); //appel de la fonction pour send terminate
        }
    }

    // Wait for responses

    for (int i = 0; i < p_context->processes_count; ++i) { //boucle pour récuprer chaque pid
        int pid = p_context->destination_analyzers_pids[i];//recupere le pid stocker
        if (pid > 0) {  //si c'est un pid valide
            int status; //pour recuperer le statut du processus fini
            waitpid(pid, &status, 0);//attente que le pid ce fini
        }
    }

    // Free allocated memory
    free(p_context->source_analyzers_pids);
     // Free the MQ
    msgctl(p_context->message_queue_id, IPC_RMID,NULL);
}
