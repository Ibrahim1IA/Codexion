/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 16:54:24 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/25 20:12:56 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	free_dongles(t_table *table, int i)
{
	fprintf(stderr, "ERROR: Failed to initialize dongle %d.\n", i);
	while (--i >= 0)
		free(table->dongles[i].waiting_queue.data);
	free(table->dongles);
	table->dongles = NULL;
	pthread_mutex_destroy(&table->dongle_mutex);
	return (0);
}

int	init_dongles(t_table *table)
{
	int	i;

	if (pthread_mutex_init(&table->dongle_mutex, NULL) != 0)
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
	pthread_mutex_destroy(&table->dongle_mutex);
}

void	release_dongles(t_coder *coder)
{
	t_dongle	*d[2];
	long long	now;
	int			id;
	int			i;

	d[0] = coder->left_dongle;
	d[1] = coder->right_dongle;
	pthread_mutex_lock(&coder->table->dongle_mutex);
	now = get_current_time(coder->table);
	i = -1;
	while (++i < 2)
	{
		d[i]->is_available = 1;
		d[i]->cooldown_until = now + coder->table->params.dongle_cooldown;
		if (d[i]->waiting_queue.size > 0)
		{
			id = d[i]->waiting_queue.data[0].coder_id;
			pthread_cond_signal(&coder->table->coders[id - 1].cond);
		}
	}
	pthread_mutex_unlock(&coder->table->dongle_mutex);
}