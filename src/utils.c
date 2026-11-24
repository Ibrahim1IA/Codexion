/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:27:17 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/24 09:27:12 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_in_ms(void)
{
	struct timeval	tv;
	long long		milliseconds;

	gettimeofday(&tv, NULL);
	milliseconds = (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000);
	return (milliseconds);
}

long long	get_current_time(t_table *table)
{
	return (get_time_in_ms() - table->start_time);
}

void	get_timeout_timespec(struct timespec *ts, long long ms_from_now)
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

int	should_stop_now(t_table *table)
{
	int	result;

	pthread_mutex_lock(&table->end_mutex);
	result = table->simulation_ended;
	pthread_mutex_unlock(&table->end_mutex);
	return (result);
}

void	ft_swap(t_request *a, t_request *b)
{
	t_request	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}
