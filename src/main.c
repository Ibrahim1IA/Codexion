/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:26:54 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/22 13:27:56 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int main(int argc, char **argv)
{
    t_table table;

    if (parse_args(&table.params, argc, argv) == 0)
        return (1);
    // Initialisation et lancement de la simulation
    if (!init_simulation(&table, table.params))
        return (1);

    // Attente et nettoyage de la mémoire après l'arrêt de la simulation
    clean_exit(&table);

    return (0);
}

