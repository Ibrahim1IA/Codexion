/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 12:26:42 by iissoufo          #+#    #+#             */
/*   Updated: 2026/11/23 16:26:35 by iissoufo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	sift_up(t_min_heap *heap, int index, t_scheduler sch_type)
{
	int	parent_index;

	while (index > 0)
	{
		parent_index = (index - 1) / 2;
		if (is_higher_priority(&heap->data[index], &heap->data[parent_index],
				sch_type))
		{
			ft_swap(&heap->data[index], &heap->data[parent_index]);
			index = parent_index;
		}
		else
			break ;
	}
}

int	min_heap_push(t_min_heap *heap, t_request *request, t_scheduler sch_type)
{
	if (heap->size >= heap->capacity)
	{
		fprintf(stderr, "ERROR: Heap is full. Cannot push new request.\n");
		return (0);
	}
	heap->data[heap->size] = *request;
	heap->size++;
	sift_up(heap, heap->size - 1, sch_type);
	return (1);
}

static int	get_smallest_child_index(t_min_heap *heap, int index,
		t_scheduler sch_type)
{
	int	left_child_index;
	int	right_child_index;
	int	smallest_index;

	left_child_index = 2 * index + 1;
	right_child_index = 2 * index + 2;
	smallest_index = index;
	if (left_child_index < heap->size
		&& is_higher_priority(&heap->data[left_child_index],
			&heap->data[smallest_index], sch_type))
	{
		smallest_index = left_child_index;
	}
	if (right_child_index < heap->size
		&& is_higher_priority(&heap->data[right_child_index],
			&heap->data[smallest_index], sch_type))
	{
		smallest_index = right_child_index;
	}
	return (smallest_index);
}

static void	sift_down(t_min_heap *heap, int index, t_scheduler sch_type)
{
	int	smallest_index;

	while (1)
	{
		smallest_index = get_smallest_child_index(heap, index, sch_type);
		if (smallest_index == index)
			break ;
		ft_swap(&heap->data[index], &heap->data[smallest_index]);
		index = smallest_index;
	}
}

int	min_heap_pop(t_min_heap *heap, t_scheduler sch_type)
{
	t_request	first_request;
	t_request	last_request;

	if (heap->size <= 0)
	{
		fprintf(stderr, "ERROR: Heap is empty. Cannot pop request.\n");
		return (1);
	}
	(void)first_request;
	first_request = heap->data[0];
	last_request = heap->data[heap->size - 1];
	heap->data[0] = last_request;
	heap->size--;
	sift_down(heap, 0, sch_type);
	return (0);
}
