/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:27:12 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/25 20:21:08 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	precise_sleep(long long duration, t_table *table)
{
	long long	end;
	long long	remaining;

	end = get_current_time(table) + duration;
	while (!should_stop_now(table))
	{
		remaining = end - get_current_time(table);
		if (remaining <= 0)
			break ;
		if (remaining > 10)
			usleep(10000);
		else
			usleep(remaining * 1000 / 2 + 100);
	}
}

static void	coder_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->coder_mutex);
	coder->last_compile_start = get_current_time(coder->table);
	pthread_mutex_unlock(&coder->coder_mutex);
	print_log(coder, "is compiling");
	precise_sleep(coder->table->params.time_to_compile, coder->table);
	pthread_mutex_lock(&coder->coder_mutex);
	coder->compiles_done++;
	if (coder->compiles_done >= coder->table->params.compiles_required)
		coder->is_full = 1;
	pthread_mutex_unlock(&coder->coder_mutex);
}

static void	coder_debug(t_coder *coder)
{
	print_log(coder, "is debugging");
	release_dongles(coder);
	precise_sleep(coder->table->params.time_to_debug, coder->table);
}

static void	coder_refactor(t_coder *coder)
{
	print_log(coder, "is refactoring");
	precise_sleep(coder->table->params.time_to_refactor, coder->table);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	pthread_mutex_lock(&coder->table->start_mutex);
	pthread_mutex_unlock(&coder->table->start_mutex);
	if (coder->id % 2 == 0)
		precise_sleep(coder->table->params.time_to_compile, coder->table);
	while (!should_stop_now(coder->table))
	{
		if (!acquire_both_dongles(coder))
			break ;
		coder_compile(coder);
		coder_debug(coder);
		if (should_stop_now(coder->table))
			break ;
		coder_refactor(coder);
	}
	return (NULL);
}
