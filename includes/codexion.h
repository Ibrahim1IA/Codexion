#ifndef CODEXION_H
# define CODEXION_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
# include <stdbool.h>
#include <unistd.h>

typedef enum e_scheduler
{
    SCH_FIFO,
    SCH_EDF
} t_scheduler;

typedef struct s_params{
    int num_coders;
    int compiles_required;
    long long time_to_burnout;
    long long time_to_compile;
    long long time_to_debug;
    long long time_to_refactor;
    long long dongle_cooldown;
    t_scheduler scheduler_type;
} t_params;
// Élément individuel d'attente dans le Min-Heap
typedef struct s_request
{
    int coder_id;
    long long request_time;
    long long deadline;
} t_request;

// File de priorité sous forme de Tas Binaire (Min-Heap)
typedef struct s_min_heap
{
    t_request *data;
    int size;
    int capacity;
} t_min_heap;


// Structure représentant un Dongle USB
typedef struct s_dongle{
    int             id;
    int             is_available;     // 1 si libre, 0 si occupé ou en cooldown
    long long       cooldown_until;   // Timestamp d'expiration du cooldown
    pthread_mutex_t mutex;            // Mutex protégeant l'état du dongle et de sa queue
    pthread_cond_t  cond;             // Signal d'attente pour la libération
    t_min_heap      waiting_queue;    // File de priorité d'accès
}t_dongle;

// Structure d'un Coder (Thread)
typedef struct s_coder{
    int             id;               // Numéro de 1 à number_of_coders
    pthread_t       thread_id;        // Thread POSIX
    long long       last_compile_start;// Horodatage du début de la dernière compilation
    int             compiles_done;    // Nombre de compilations effectuées
    int             is_full;          // 1 si compiles_required est atteint
    t_dongle        *left_dongle;     // Pointeur vers le dongle gauche
    t_dongle        *right_dongle;    // Pointeur vers le dongle droit
    pthread_mutex_t coder_mutex;      // Protège les lectures/écritures des variables du coder
    struct s_table  *table;           // Référence vers la structure globale    
} t_coder;

// Context global de la simulation (Remplace les variables globales interdites
typedef struct s_table {
    t_params        params;
    long long       start_time;       // Horodatage du début de la simulation
    int             simulation_ended; // Flag d'arrêt (burnout ou compilations terminées)
    pthread_mutex_t end_mutex;        // Protège le flag d'arrêt de la simulation
    pthread_mutex_t log_mutex;        // Sérialisation des messages d'affichage
    t_dongle        *dongles;         // Tableau de dongles (taille = num_coders)
    t_coder         *coders;          // Tableau de coders (taille = num_coders)
    pthread_t       monitor_thread;   // Thread moniteur
}   t_table;


int min_heap_push(t_min_heap *heap, t_request *request, t_scheduler sch_type);

int should_stop_now(t_table *table);
#endif