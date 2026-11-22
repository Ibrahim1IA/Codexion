/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 16:54:24 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/22 19:02:59 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	get_timeout_timespec(struct timespec *ts, long long ms_from_now)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	ts->tv_sec = tv.tv_sec + (ms_from_now / 1000);
	ts->tv_nsec = (tv.tv_usec * 1000) + ((ms_from_now % 1000) * 1000000);
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec += 1;
		ts->tv_nsec -= 1000000000;
	}
}

int	heap_init(t_min_heap *heap, int capacity)
{
	heap->data = malloc(sizeof(t_request) * capacity);
	if (!heap->data)
	{
		fprintf(stderr, "ERROR: Failed to initialize dongle heap.\n");
		return (0);
	}
	heap->size = 0;
	heap->capacity = capacity;
	return (1);
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
		pthread_mutex_init(&table->dongles[i].mutex, NULL);
		pthread_cond_init(&table->dongles[i].cond, NULL);
		if (!heap_init(&table->dongles[i].waiting_queue,
				table->params.num_coders))
		{
			fprintf(stderr,
				"ERROR: Failed to initialize dongle waiting queue.\n");
			pthread_mutex_destroy(&table->dongles[i].mutex);
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

bool	take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request		request;
	long long		now;
	long long		cooldown_remaining;
	struct timespec	ts;

	pthread_mutex_lock(&dongle->mutex);
	request.coder_id = coder->id;
	request.request_time = get_current_time(coder->table);
	request.deadline = coder->last_compile_start
		+ coder->table->params.time_to_burnout;
	min_heap_push(&dongle->waiting_queue, &request,
		coder->table->params.scheduler_type);
	while (1)
	{
		pthread_mutex_lock(&coder->table->end_mutex);
		if (coder->table->simulation_ended)
		{
			pthread_mutex_unlock(&coder->table->end_mutex);
			pthread_mutex_unlock(&dongle->mutex);
			return (false);
		}
		pthread_mutex_unlock(&coder->table->end_mutex);
		now = get_current_time(coder->table);
		if (dongle->waiting_queue.size > 0
			&& dongle->waiting_queue.data[0].coder_id == coder->id)
		{
			if (dongle->is_available && now >= dongle->cooldown_until)
			{
				min_heap_pop(&dongle->waiting_queue,
					coder->table->params.scheduler_type);
				dongle->is_available = 0;
				print_log(coder, "has taken a dongle");
				pthread_mutex_unlock(&dongle->mutex);
				return (true);
			}
			else if (dongle->is_available && now < dongle->cooldown_until)
			{
				cooldown_remaining = dongle->cooldown_until - now;
				get_timeout_timespec(&ts, cooldown_remaining);
				pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
			}
			else
			{
				pthread_cond_wait(&dongle->cond, &dongle->mutex);
			}
		}
		else
		{
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
		}
	}
}

static void	release_one(t_dongle *dongle, long long now,
		long long cooldown_time)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_available = 1;
	dongle->cooldown_until = now + cooldown_time;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongles(t_coder *coder)
{
	t_dongle	*dongles[2];
	long long	now;
	int			i;

	dongles[0] = coder->left_dongle;
	dongles[1] = coder->right_dongle;
	now = get_current_time(coder->table);
	if (dongles[0] == dongles[1])
		release_one(dongles[0], now, coder->table->params.dongle_cooldown);
	else
	{
		i = 0;
		while (i < 2)
		{
			release_one(dongles[i], now, coder->table->params.dongle_cooldown);
			i++;
		}
	}
}

bool	acquire_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle->id == coder->right_dongle->id)
	{
		if (take_dongle(coder, coder->left_dongle))
		{
			while (!should_stop_now(coder->table))
				usleep(500);
			release_one(coder->left_dongle, get_current_time(coder->table),
				coder->table->params.dongle_cooldown);
		}
		return (false);
	}
	if (coder->id % 2 == 1)
    {
        first = coder->left_dongle;
        second = coder->right_dongle;
    }
    else
    {
        first = coder->right_dongle;
        second = coder->left_dongle;
    }
	if (!take_dongle(coder, first))
		return (false);
	if (!take_dongle(coder, second))
	{
		release_one(first, get_current_time(coder->table),
			coder->table->params.dongle_cooldown);
		return (false);
	}
	return (true);
}
