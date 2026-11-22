#include "codexion.h"

static int init_simulation_mutexes(t_table *table)
{
    if (pthread_mutex_init(&table->end_mutex, NULL) != 0)
        return (0);
    if (pthread_mutex_init(&table->log_mutex, NULL) != 0)
    {
        fprintf(stderr, "ERROR: Failed to initialize log mutex.\n");
        pthread_mutex_destroy(&table->end_mutex);
        return (0);
    }
    if (pthread_mutex_init(&table->start_mutex, NULL) != 0)
    {
        fprintf(stderr, "ERROR: Failed to initialize start mutex.\n");
        pthread_mutex_destroy(&table->end_mutex);
        pthread_mutex_destroy(&table->log_mutex);
        return (0);
    }
    table->simulation_ended = 0;
    return (1);
}
void join_coders(t_table *table, int count)
{
    int i;

    i = 0;
    while (i < count)
    {
        pthread_join(table->coders[i].thread_id, NULL);
        i++;
    }
}

void free_all(t_table *table)
{
    int i;

    if (table->coders)
    {
        i = 0;
        while (i < table->params.num_coders)
        {
            pthread_mutex_destroy(&table->coders[i].coder_mutex);
            i++;
        }
        free(table->coders);
        table->coders = NULL;
    }
    destroy_dongles(table);
    pthread_mutex_destroy(&table->log_mutex);
    pthread_mutex_destroy(&table->end_mutex);
    pthread_mutex_destroy(&table->start_mutex);
}

void clean_exit(t_table *table)
{
    pthread_join(table->monitor_thread, NULL);
    join_coders(table, table->params.num_coders);
    free_all(table);
}

int init_simulation(t_table *table,  t_params params)
{
    table->params = params;
    if (!init_simulation_mutexes(table))
        return (0);

    if (!init_dongles(table))
    {
        pthread_mutex_destroy(&table->log_mutex);
        pthread_mutex_destroy(&table->end_mutex);
        pthread_mutex_destroy(&table->start_mutex);
        return (0);
    }
    
    if (!init_coder(table))
        return (0);
    

    if (pthread_create(&table->monitor_thread, NULL, monitor_routine, (void *)table) != 0)
    {
        fprintf(stderr, "ERROR: Failed to create monitor thread.\n");
        stop_simulation(table);
        join_coders(table, table->params.num_coders);
        free_all(table);
        return (0);
    }
    return (1);
}