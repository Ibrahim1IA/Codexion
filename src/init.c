#include "codexion.h"

void init_simulation(t_table *table,  t_params params)
{
    table->params = params;
    table->start_time = get_current_time();
    table->simulation_ended = 0;
    pthread_mutex_init(&table->end_mutex, NULL);
    pthread_mutex_init(&table->log_mutex, NULL);
    init_dongles(table);
    init_coders(table);
}