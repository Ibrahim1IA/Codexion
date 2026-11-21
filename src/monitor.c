#include "codexion.h"

static int check_all_full(t_table *table)
{
    int i;
    int full_count;

    if (table->params.compiles_required <= 0)
        return (0);

    full_count = 0;
    for (i = 0; i < table->params.num_coders; i++) {
        pthread_mutex_lock(&table->coders[i].coder_mutex);
        if (table->coders[i].is_full)
            full_count++;
        pthread_mutex_unlock(&table->coders[i].coder_mutex);
    }
    return (full_count == table->params.num_coders);
}

static void stop_simulation(t_table *table)
{
    int i;

    pthread_mutex_lock(&table->end_mutex);
    table->simulation_ended = 1;
    pthread_mutex_unlock(&table->end_mutex);
    i = 0;
    while (i < table->params.num_coders)
    {
        pthread_mutex_lock(&table->dongles[i].mutex);
        pthread_cond_broadcast(&table->dongles[i].cond);
        pthread_mutex_unlock(&table->dongles[i].mutex);
        i++;
    }
}

void *monitor_routine(void *arg)
{
    t_table *table;
    int i;
    long long now;
    long long last_compile;

    table = (t_table *)arg;
    while (!should_stop_now(table))
    {
        i = 0;
        while (i < table->params.num_coders)
        {
            pthread_mutex_lock(&table->coders[i].coder_mutex);
            last_compile = table->coders[i].last_compile_start;
            pthread_mutex_unlock(&table->coders[i].coder_mutex);
            now = get_current_time(table);
            if (now - last_compile > table->params.time_to_burnout)
            {
                stop_simulation(table);
                print_log(&table->coders[i], "burned out");
                return (NULL);
            }
            i++;
        }
        if (check_all_full(table))
        {
            stop_simulation(table);
            return (NULL);
        }
        // Pause courte de 1 ms pour respecter la tolérance strict de 10 ms sans surcharger le CPU
        usleep(1000); // Sleep to reduce CPU usage
    }
    return (NULL);
}