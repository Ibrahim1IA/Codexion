/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:25:36 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/22 13:25:48 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
static void setup_coder_struct(t_table *table, int i)
{
    t_coder *coder;
    coder = &table->coders[i];
    coder->id = i + 1;
    coder->compiles_done = 0;
    coder->is_full = 0;
    coder->last_compile_start = 0;
    coder->left_dongle = &table->dongles[i];
    if (i == table->params.num_coders - 1)
        coder->right_dongle = &table->dongles[0];
    else
        coder->right_dongle = &table->dongles[(i + 1) % table->params.num_coders];
    pthread_mutex_init(&table->coders[i].coder_mutex, NULL);
    table->coders[i].table = table;
}

int init_coder(t_table *table)
{
    int i;

    table->coders = malloc(sizeof(t_coder) * table->params.num_coders);
    if (!table->coders)
        return (0);
    
    for (i = 0; i < table->params.num_coders; i++)
        setup_coder_struct(table, i);

    i = 0;
    while (i < table->params.num_coders)
    {
        if (pthread_create(&table->coders[i].thread_id, NULL, coder_routine, &table->coders[i]) != 0)
        {
            fprintf(stderr, "ERROR: Failed to create coder thread %d.\n", i + 1);
            clean_exit(table);
            return (0);
        }
        i++;
    }
    return (1);
}