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
    table->simulation_ended = 0;
    return (1);
}

static void clean_exit(t_table *table)
{
    pthread_mutex_destroy(&table->end_mutex);
    pthread_mutex_destroy(&table->log_mutex);
    destroy_dongles(table);
    free(table->coders);
}
int init_simulation(t_table *table,  t_params params)
{
    table->params = params;
    table->start_time = get_time_in_ms();
    if (!init_simulation_mutexes(table))
        return (0);

    if (!init_dongles(table))
    {
        // Clean up mutexes if dongle initialization fails
        pthread_mutex_destroy(&table->log_mutex);
        pthread_mutex_destroy(&table->end_mutex);
        return (0);
    }
    
    if (!init_coder(table))
    {
        destroy_dongles(table);
        pthread_mutex_destroy(&table->log_mutex);
        pthread_mutex_destroy(&table->end_mutex);
        return (0);
    }

    if (pthread_create(&table->monitor_thread, NULL, monitor_routine, (void *)table) != 0)
    {
        table->simulation_ended = 1;
        clean_exit(table);
        return (0);
    }
    return (1);
}