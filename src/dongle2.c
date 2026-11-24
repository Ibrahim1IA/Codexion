/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 09:02:37 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/24 19:30:44 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	take_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	push_request(coder, dongle);
	while (!should_stop_now(coder->table))
	{
		if (try_acquire(coder, dongle))
		{
			pthread_mutex_unlock(&dongle->mutex);
			return (true);
		}
	}
	pthread_mutex_unlock(&dongle->mutex);
	return (false);
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

static void	assign_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		*first = coder->left_dongle;
		*second = coder->right_dongle;
	}
	else
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
	(void)first;
	(void)second;
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
	assign_dongles(coder, &first, &second);
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
