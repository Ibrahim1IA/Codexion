#include "codexion.h"

int should_stop_now(t_table *table)
{
    int result;

    pthread_mutex_lock(&table->end_mutex);
    result = table->simulation_ended;
    pthread_mutex_unlock(&table->end_mutex);
    return (result);
}

void	ft_swap(t_coder **a, t_coder **b)
{
	t_coder	*temp;

	temp = *a;
	*a = *b;
	*b = temp;
}