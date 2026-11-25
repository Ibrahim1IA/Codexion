/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 17:57:00 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/25 20:12:00 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef enum e_scheduler
{
	SCH_FIFO,
	SCH_EDF
}					t_scheduler;

typedef struct s_params
{
	int				num_coders;
	int				compiles_required;
	long long		time_to_burnout;
	long long		time_to_compile;
	long long		time_to_debug;
	long long		time_to_refactor;
	long long		dongle_cooldown;
	t_scheduler		scheduler_type;
}					t_params;

// Élément individuel d'attente dans le Min-Heap
typedef struct s_request
{
	int				coder_id;
	long long		request_time;
	long long		deadline;
}					t_request;

typedef struct s_min_heap
{
	t_request		*data;
	int				size;
	int				capacity;
}					t_min_heap;

typedef struct s_dongle
{
	int				id;
	int				is_available;
	long long		cooldown_until;
	t_min_heap		waiting_queue;
}					t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread_id;
	long long		last_compile_start;
	int				compiles_done;
	int				is_full;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	pthread_mutex_t	coder_mutex;
	pthread_cond_t	cond;
	struct s_table	*table;
}					t_coder;

typedef struct s_table
{
	t_params		params;
	long long		start_time;
	int				simulation_ended;
	pthread_mutex_t dongle_mutex;
	pthread_mutex_t	end_mutex;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	start_mutex;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_t		monitor_thread;
}					t_table;

int					init_dongles(t_table *table);
bool				acquire_both_dongles(t_coder *coder);
void				release_dongles(t_coder *coder);
void				destroy_dongles(t_table *table);

int					parse_args(t_params *params, int ac, char **av);

void				*monitor_routine(void *arg);

void				*coder_routine(void *arg);
int					init_coder(t_table *table);

int					min_heap_push(t_min_heap *heap, t_request *request,
						t_scheduler sch_type);
int					min_heap_pop(t_min_heap *heap, t_scheduler sch_type);
int					heap_init(t_min_heap *heap, int capacity);

int					init_simulation(t_table *table, t_params params);
void				clean_exit(t_table *table);
void				join_coders(t_table *table, int count);
void				free_all(t_table *table);
void				stop_simulation(t_table *table);

// Utils

long long			get_current_time(t_table *table);
void				print_log(t_coder *coder, const char *message);
int					should_stop_now(t_table *table);
void				ft_swap(t_request *a, t_request *b);
bool				is_higher_priority(t_request *a, t_request *b,
						t_scheduler sch_type);
long				get_time_in_ms(void);
void				get_timeout_timespec(struct timespec *ts,
						long long ms_from_now);
#endif