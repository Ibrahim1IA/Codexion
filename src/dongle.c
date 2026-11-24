/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 16:54:24 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/24 17:09:29 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	free_dongles(t_table *table, int i, int step)
{
	fprintf(stderr, "ERROR: Failed to initialize dongle waiting queue.\n");
	if (step >= 1)
		pthread_mutex_destroy(&table->dongles[i].mutex);
	if (step >= 2)
		pthread_cond_destroy(&table->dongles[i].cond);
	while (--i >= 0)
	{
		pthread_mutex_destroy(&table->dongles[i].mutex);
		pthread_cond_destroy(&table->dongles[i].cond);
		free(table->dongles[i].waiting_queue.data);
	}
	free(table->dongles);
	table->dongles = NULL;
	return (0);
}

int	init_dongles(t_table *table)
{
	int	i;

	table->dongles = malloc(sizeof(t_dongle) * table->params.num_coders);
	if (!table->dongles)
		return (0);
	i = 0;
	while (i < table->params.num_coders)
	{
		table->dongles[i].id = i + 1;
		table->dongles[i].is_available = 1;
		table->dongles[i].cooldown_until = 0;
		if (pthread_mutex_init(&table->dongles[i].mutex, NULL) != 0)
			free_dongles(table, i, 0);
		if (pthread_cond_init(&table->dongles[i].cond, NULL) != 0)
			free_dongles(table, i, 1);
		if (!heap_init(&table->dongles[i].waiting_queue,
				table->params.num_coders))
			return (free_dongles(table, i, 2));
		i++;
	}
	return (1);
}

void	destroy_dongles(t_table *table)
{
	int	i;

	if (!table || !table->dongles)
		return ;
	i = 0;
	while (i < table->params.num_coders)
	{
		pthread_mutex_destroy(&table->dongles[i].mutex);
		pthread_cond_destroy(&table->dongles[i].cond);
		if (table->dongles[i].waiting_queue.data)
		{
			free(table->dongles[i].waiting_queue.data);
			table->dongles[i].waiting_queue.data = NULL;
		}
		i++;
	}
	free(table->dongles);
	table->dongles = NULL;
}

void	push_request(t_coder *coder, t_dongle *dongle)
{
	t_request	request;

	request.coder_id = coder->id;
	request.request_time = get_current_time(coder->table);
	request.deadline = coder->last_compile_start
		+ coder->table->params.time_to_burnout;
	min_heap_push(&dongle->waiting_queue, &request,
		coder->table->params.scheduler_type);
}

bool	try_acquire(t_coder *coder, t_dongle *dongle)
{
	long long		now;
	struct timespec	ts;

	now = get_current_time(coder->table);
	if (!(dongle->waiting_queue.size > 0
			&& dongle->waiting_queue.data[0].coder_id == coder->id)
		|| !dongle->is_available)
	{
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
		return (false);
	}
	if (now < dongle->cooldown_until)
	{
		get_timeout_timespec(&ts, dongle->cooldown_until - now);
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		return (false);
	}
	min_heap_pop(&dongle->waiting_queue, coder->table->params.scheduler_type);
	dongle->is_available = 0;
	print_log(coder, "has taken a dongle");
	return (true);
}
