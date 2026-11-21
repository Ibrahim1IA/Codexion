#include "codexion.h"

long get_time_in_ms(void)
{
    struct timeval  tv;
    long long       milliseconds;

    gettimeofday(&tv, NULL);
    milliseconds = (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000);
    return (milliseconds);
}

/*
** Renvoie le temps écoulé en ms depuis le début de la simulation
*/
long long	get_current_time(t_table *table)
{
	return (get_time_in_ms() - table->start_time);
}

int should_stop_now(t_table *table)
{
    int result;

    pthread_mutex_lock(&table->end_mutex);
    result = table->simulation_ended;
    pthread_mutex_unlock(&table->end_mutex);
    return (result);
}

void	ft_swap(t_request *a, t_request *b)
{
	t_request	*temp;

	temp = a;
	a = b;
	b = temp;
}

bool is_higher_priority(t_request *a, t_request *b, t_scheduler sch_type)
{
    if (sch_type == SCH_EDF)
    {
        if (a->deadline < b->deadline)
            return true;
        else if (a->deadline == b->deadline)
            return a->request_time < b->request_time;
        else
            return false;
    }
    else // SCH_FIFO
    {
        if (a->request_time < b->request_time)
            return true;
        else if (a->request_time == b->request_time)
            return a->deadline < b->deadline;
        else
            return false;
    }
}
void print_log(t_coder *coder, const char *message)
{
    long long timestamp;

    pthread_mutex_lock(&coder->table->log_mutex);
    pthread_mutex_lock(&coder->table->end_mutex);
    if (!coder->table->simulation_ended ||
        strcmp(message, "burned out") == 0)
    {
        timestamp = get_current_time(coder->table);
        printf("%lld %d %s\n", timestamp, coder->id, message);
    }
    
    pthread_mutex_unlock(&coder->table->log_mutex);
}