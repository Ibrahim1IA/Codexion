/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 08:48:21 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/24 08:59:45 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	free_all(t_table *table)
{
	int	i;

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

void	clean_exit(t_table *table)
{
	pthread_join(table->monitor_thread, NULL);
	join_coders(table, table->params.num_coders);
	free_all(table);
}

bool	is_higher_priority(t_request *a, t_request *b, t_scheduler sch_type)
{
	if (sch_type == SCH_EDF)
	{
		if (a->deadline < b->deadline)
			return (true);
		else if (a->deadline == b->deadline)
			return (a->request_time < b->request_time);
		else
			return (false);
	}
	else
	{
		if (a->request_time < b->request_time)
			return (true);
		else if (a->request_time == b->request_time)
			return (a->deadline < b->deadline);
		else
			return (false);
	}
}

void	print_log(t_coder *coder, const char *message)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->table->log_mutex);
	pthread_mutex_lock(&coder->table->end_mutex);
	if (!coder->table->simulation_ended || strcmp(message, "burned out") == 0)
	{
		timestamp = get_current_time(coder->table);
		printf("%lld %d %s\n", timestamp, coder->id, message);
	}
	pthread_mutex_unlock(&coder->table->end_mutex);
	pthread_mutex_unlock(&coder->table->log_mutex);
}
