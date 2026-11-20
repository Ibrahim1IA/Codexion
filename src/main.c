#include "codexion.h"
void clean_exit(t_table *table)
{
    int i;

    // 1. Attente de la fin du thread moniteur
    pthread_join(table->monitor_thread, NULL);

    // 2. Attente de la fin de chaque thread coder
    i = 0;
    while (i < table->params.num_coders)
    {
        pthread_join(table->coders[i].thread_id, NULL);
        pthread_mutex_destroy(&table->coders[i].coder_mutex);
        i++;
    }

    // 3. Libération des dongles et de leurs tas binaires
    destroy_dongles(table);

    // 4. Destruction des mutexes globaux
    pthread_mutex_destroy(&table->log_mutex);
    pthread_mutex_destroy(&table->end_mutex);

    // 5. Libération des tableaux dynamiques
    if (table->coders)
        free(table->coders);
    if (table->params.num_coders > 0)
        free(table->dongles);
}

int main(int argc, char **argv)
{
    t_table table;

    if (parse_args(&table.params, argc, argv) == 0)
        return (1);
    // Initialisation et lancement de la simulation
    if (!init_simulation(&table, &table.params))
        return (1);

    // Attente et nettoyage de la mémoire après l'arrêt de la simulation
    clean_exit(&table);

    return (0);
}

