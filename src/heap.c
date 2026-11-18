#include "codexion.h"

int min_heap_push(t_min_heap *heap, t_request *request, t_scheduler sch_type)
{
    if (heap->size >= heap->capacity)
    {
        fprintf(stderr, "ERROR: Heap is full. Cannot push new request.\n");
        return (1);
    }
    heap->data[heap->size] = *request;
    heap->size++;
    sift_up(heap, heap->size - 1, sch_type);
    return (0);
}