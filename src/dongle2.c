/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 09:02:37 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/25 21:26:15 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	push_request(t_coder *c)
{
	t_request	request;

	request.coder_id = c->id;
	request.request_time = get_current_time(c->table);
	request.deadline = c->last_compile_start + c->table->params.time_to_burnout;
	min_heap_push(&c->left_dongle->waiting_queue, &request,
		c->table->params.scheduler_type);
	min_heap_push(&c->right_dongle->waiting_queue, &request,
		c->table->params.scheduler_type);
}

static bool	is_ready(t_coder *c, t_dongle *d)
{
	return (d->is_available && d->waiting_queue.size > 0
		&& d->waiting_queue.data[0].coder_id == c->id);
}

static void	take_both(t_coder *c)
{
	min_heap_pop(&c->left_dongle->waiting_queue,
		c->table->params.scheduler_type);
	min_heap_pop(&c->right_dongle->waiting_queue,
		c->table->params.scheduler_type);
	c->left_dongle->is_available = 0;
	c->right_dongle->is_available = 0;
	pthread_mutex_lock(&c->coder_mutex);
	c->last_compile_start = get_current_time(c->table);
	pthread_mutex_unlock(&c->coder_mutex);
	print_compile_start(c);
}

static bool	try_acquire_both(t_coder *c)
{
	long long		wait;
	struct timespec	ts;

	if (!is_ready(c, c->left_dongle) || !is_ready(c, c->right_dongle))
	{
		pthread_cond_wait(&c->cond, &c->table->dongle_mutex);
		return (false);
	}
	wait = c->left_dongle->cooldown_until;
	if (c->right_dongle->cooldown_until > wait)
		wait = c->right_dongle->cooldown_until;
	wait -= get_current_time(c->table);
	if (wait > 0)
	{
		get_timeout_timespec(&ts, wait);
		pthread_cond_timedwait(&c->cond,
			&c->table->dongle_mutex, &ts);
		return (false);
	}
	take_both(c);
	return (true);
}

bool	acquire_both_dongles(t_coder *c)
{
	if (c->left_dongle == c->right_dongle)
	{
		print_log(c, "has taken a dongle");
		while (!should_stop_now(c->table))
			usleep(500);
		return (false);
	}
	pthread_mutex_lock(&c->table->dongle_mutex);
	push_request(c);
	while (!should_stop_now(c->table))
	{
		if (try_acquire_both(c))
		{
			pthread_mutex_unlock(&c->table->dongle_mutex);
			return (true);
		}
	}
	pthread_mutex_unlock(&c->table->dongle_mutex);
	return (false);
}