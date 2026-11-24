/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:25:36 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/24 17:58:32 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	setup_coder_struct(t_table *table, int i)
{
	t_coder	*coder;

	coder = &table->coders[i];
	coder->id = i + 1;
	coder->compiles_done = 0;
	coder->is_full = 0;
	coder->last_compile_start = 0;
	coder->left_dongle = &table->dongles[i];
	if (i == table->params.num_coders - 1)
		coder->right_dongle = &table->dongles[0];
	else
		coder->right_dongle = &table->dongles[(i + 1)
			% table->params.num_coders];
	table->coders[i].table = table;
	if (pthread_mutex_init(&table->coders[i].coder_mutex, NULL) != 0)
		return (0);
	return (1);
}

static int	instanciate_coders(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->params.num_coders)
	{
		if (pthread_create(&table->coders[i].thread_id, NULL, coder_routine,
				&table->coders[i]) != 0)
		{
			fprintf(stderr, "ERROR: Failed to create coder thread %d.\n", i
				+ 1);
			pthread_mutex_lock(&table->end_mutex);
			table->simulation_ended = 1;
			pthread_mutex_unlock(&table->end_mutex);
			pthread_mutex_unlock(&table->start_mutex);
			join_coders(table, i);
			free_all(table);
			return (0);
		}
		i++;
	}
	return (1);
}

static int	free_coders(t_table *table, int count)
{
	fprintf(stderr, "ERROR: Failed to initialize coder mutex %d.\n",
		count + 1);
	while (--count >= 0)
		pthread_mutex_destroy(&table->coders[count].coder_mutex);
	free(table->coders);
	table->coders = NULL;
	free_all(table);
	return (0);
}

int	init_coder(t_table *table)
{
	int	i;

	table->coders = malloc(sizeof(t_coder) * table->params.num_coders);
	if (!table->coders)
	{
		fprintf(stderr, "ERROR: Failed to allocate coders.\n");
		free_all(table);
		return (0);
	}
	i = 0;
	while (i < table->params.num_coders)
	{
		if (!setup_coder_struct(table, i))
			return (free_coders(table, i));
		i++;
	}
	pthread_mutex_lock(&table->start_mutex);
	if (!instanciate_coders(table))
		return (0);
	table->start_time = get_time_in_ms();
	pthread_mutex_unlock(&table->start_mutex);
	return (1);
}
