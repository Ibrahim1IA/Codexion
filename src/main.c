/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:26:54 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/22 16:46:52 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_table	table;

	if (parse_args(&table.params, argc, argv) == 0)
		return (1);
	if (!init_simulation(&table, table.params))
		return (1);
	clean_exit(&table);
	return (0);
}
