/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:26:59 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/22 14:35:55 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_all_full(t_table *table)
{
	int	i;
	int	full_count;

	if (table->params.compiles_required <= 0)
		return (0);
	full_count = 0;
	i = 0;
	while (i < table->params.num_coders)
	{
		pthread_mutex_lock(&table->coders[i].coder_mutex);
		if (table->coders[i].is_full)
			full_count++;
		pthread_mutex_unlock(&table->coders[i].coder_mutex);
		i++;
	}
	return (full_count == table->params.num_coders);
}

void	stop_simulation(t_table *table)
{
	int	i;

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

static void	burnout_check(t_table *table)
{
	int			i;
	long long	now;
	long long	last_compile;

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
			return ;
		}
		i++;
	}
}

void	*monitor_routine(void *arg)
{
	t_table	*table;

	table = (t_table *)arg;
	while (!should_stop_now(table))
	{
		burnout_check(table);
		if (check_all_full(table))
		{
			stop_simulation(table);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
