/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 16:54:24 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/25 19:42:46 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include "codexion.h"

static int	free_dongles(t_table *table, int i)
{
	fprintf(stderr, "ERROR: Failed to initialize dongle %d.\n", i);
	while (--i >= 0)
		free(table->dongles[i].waiting_queue.data);
	free(table->dongles);
	table->dongles = NULL;
	pthread_cond_destroy(&table->dongle_cond);
	pthread_mutex_destroy(&table->dongle_mutex);
	return (0);
}

static int	init_dongle_sync(t_table *table)
{
	if (pthread_mutex_init(&table->dongle_mutex, NULL) != 0)
		return (0);
	if (pthread_cond_init(&table->dongle_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&table->dongle_mutex);
		return (0);
	}
	return (1);
}

int	init_dongles(t_table *table)
{
	int	i;

	if (!init_dongle_sync(table))
		return (0);
	table->dongles = malloc(sizeof(t_dongle) * table->params.num_coders);
	if (!table->dongles)
		return (free_dongles(table, 0));
	i = 0;
	while (i < table->params.num_coders)
	{
		table->dongles[i].id = i + 1;
		table->dongles[i].is_available = 1;
		table->dongles[i].cooldown_until = 0;
		if (!heap_init(&table->dongles[i].waiting_queue,
				table->params.num_coders))
			return (free_dongles(table, i));
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
		free(table->dongles[i].waiting_queue.data);
		i++;
	}
	free(table->dongles);
	table->dongles = NULL;
	pthread_cond_destroy(&table->dongle_cond);
	pthread_mutex_destroy(&table->dongle_mutex);
}

void	release_dongles(t_coder *coder)
{
	t_table		*t;
	long long	now;

	t = coder->table;
	pthread_mutex_lock(&t->dongle_mutex);
	now = get_current_time(t);
	coder->left_dongle->is_available = 1;
	coder->left_dongle->cooldown_until = now + t->params.dongle_cooldown;
	coder->right_dongle->is_available = 1;
	coder->right_dongle->cooldown_until = now + t->params.dongle_cooldown;
	pthread_cond_broadcast(&t->dongle_cond);
	pthread_mutex_unlock(&t->dongle_mutex);
}