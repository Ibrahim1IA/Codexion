/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:27:12 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/22 12:39:34 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	precise_sleep(long long duration, t_table *table)
{
	long long	start_time;

	start_time = get_current_time(table);
	while (!should_stop_now(table))
	{
		if (get_current_time(table) - start_time >= duration)
			break ;
		usleep(500);
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
	if (coder->id % 2 == 0)
		usleep(1000);
	while (!should_stop_now(coder->table))
	{
		if (!acquire_both_dongles(coder))
			break ;
		coder_compile(coder);
		release_dongles(coder);
		if (should_stop_now(coder->table))
			break ;
		coder_debug(coder);
		if (should_stop_now(coder->table))
			break ;
		coder_refactor(coder);
	}
	return (NULL);
}
