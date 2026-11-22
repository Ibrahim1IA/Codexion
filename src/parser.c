/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:27:06 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/22 13:11:43 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid_integer(char *str)
{
	int		i;
	long	value;

	i = 0;
	value = 0;
	while (str[i])
	{
		value = value * 10 + (str[i] - '0');
		if (value > 2147483647)
		{
			fprintf(stderr,
				"ARGUMENT ERROR: A value exceeds "
				"the INT_MAX limits.\n");
			return (1);
		}
		i++;
	}
	return (0);
}

int	is_valid_numbers(int ac, char **av)
{
	int	i;
	int	j;

	i = 1;
	while (i < (ac - 1))
	{
		j = 0;
		while (av[i][j])
		{
			if ((av[i][j] >= '0') && (av[i][j] <= '9'))
				j++;
			else
				return (1);
		}
		if (is_valid_integer(av[i]) == 1)
			return (1);
		if (atoi(av[i]) == 0)
		{
			fprintf(stderr,
				"INVALID ARGUMENT : values must be greater than Zero\n");
			return (1);
		}
		i++;
	}
	return (0);
}

int	has_valid_scheduler(char *str)
{
	if ((strcmp(str, "fifo") == 0) || (strcmp(str, "edf") == 0))
		return (0);
	fprintf(stderr, "ARGUMENT ERROR: Invalid scheduler.\n");
	return (1);
}

int	is_args_valid(int ac, char **av)
{
	if (ac != 9)
    {
        fprintf(stderr, "ARGUMENT ERROR: Invalid number of arguments.\n");
        return (0);
    }
	if (is_valid_numbers(ac, av) == 1 || has_valid_scheduler(av[8]) == 1)
	{
		printf("ARGUMENT ERROR: Invalid arguments.\n");
		return (0);
	}
	return (1);
}

int	parse_args(t_params *params, int ac, char **av)
{
	if (!is_args_valid(ac, av))
		return (0);
	params->num_coders = atoi(av[1]);
	params->time_to_burnout = atoi(av[2]);
	params->time_to_compile = atoi(av[3]);
	params->time_to_debug = atoi(av[4]);
	params->time_to_refactor = atoi(av[5]);
	params->compiles_required = atoi(av[6]);
	params->dongle_cooldown = atoi(av[7]);
	if (strcmp(av[8], "fifo") == 0)
		params->scheduler_type = SCH_FIFO;
	else if (strcmp(av[8], "edf") == 0)
		params->scheduler_type = SCH_EDF;
	else
		return (0);
	return (1);
}
