#include "codexion.h"

static void get_timeout_timespec(struct timespec *ts, long long ms_from_now)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    ts->tv_sec = tv.tv_sec + (ms_from_now / 1000);
    ts->tv_nsec = (tv.tv_usec * 1000) + ((ms_from_now % 1000) * 1000000);
    if (ts->tv_nsec >= 1000000000)
    {
        ts->tv_sec += 1;
        ts->tv_nsec -= 1000000000; 
    }
}

int heap_init(t_min_heap *heap, int capacity)
{
    heap->data = malloc(sizeof(t_request) * capacity);
    if (!heap->data)
    {
		fprintf(stderr, "ERROR: Failed to initialize dongle heap.\n");
		return (0);
	}
    heap->size = 0;
    heap->capacity = capacity;
    return (1);
    
}

int init_dongles(t_table *table)
{
    int i;

    table->dongles = malloc(sizeof(t_dongle) * table->params.num_coders);
    if (!table->dongles)
        return (0);
    
    i = 0;
    while(i < table->params.num_coders)
    {
        table->dongles[i].id = i + 1;
        table->dongles[i].is_available = 1;
        table->dongles[i].cooldown_until = 0;
        pthread_mutex_init(&table->dongles[i].mutex, NULL);
        pthread_cond_init(&table->dongles[i].cond, NULL);
        if (!heap_init(&table->dongles[i].waiting_queue, table->params.num_coders))
        {
            fprintf(stderr, "ERROR: Failed to initialize dongle waiting queue.\n");
            destroy_dongles(table);
            return (0);
        }

        i++;
    }
    return (1);
}



void destroy_dongles(t_table *table)
{
    int i;

    if (!table || !table->dongles)
        return ;
    i = 0;
    while (i < table->params.num_coders)
    {
        // 1. Destruction du mutex propre au dongle
        pthread_mutex_destroy(&table->dongles[i].mutex);

        // 2. Destruction de la variable de condition
        pthread_cond_destroy(&table->dongles[i].cond);

        if (table->dongles[i].waiting_queue.data)
        {
            free(table->dongles[i].waiting_queue.data);
            table->dongles[i].waiting_queue.data = NULL;
        }
        i++;
    }

    // 4. Libération du tableau principal des dongles
    free(table->dongles);
    table->dongles = NULL;
}

bool take_dongle(t_coder *coder, t_dongle *dongle)
{
    t_request request;
    long long now;
    long long cooldown_remaining;
    struct timespec ts;
    
    // 1. Verrouiller le mutex du dongle
    pthread_mutex_lock(&dongle->mutex);
    
    // 2. Préparer la requête pour la file de priorité
    request.coder_id = coder->id;
    request.request_time = get_current_time(coder->table);
    request.deadline = coder->last_compile_start + coder->table->params.time_to_burnout;

    // Insérer la demande dans le Min-Heap du dongle
    min_heap_push(&dongle->waiting_queue, &request, coder->table->params.scheduler_type);
    while (1)
    {
        pthread_mutex_lock(&coder->table->end_mutex);
        if (coder->table->simulation_ended)
        {
            pthread_mutex_unlock(&coder->table->end_mutex);
            pthread_mutex_unlock(&dongle->mutex);
            return (false);
        }
        pthread_mutex_unlock(&coder->table->end_mutex);

        now = get_current_time(coder->table);

        // Vérifier si le coder courant est en tête du Min-Heap
        if (dongle->waiting_queue.size > 0 && dongle->waiting_queue.data[0].coder_id == coder->id)
        {
            // Cas A : Le dongle est libre et son cooldown est terminé
            if (dongle->is_available && now >= dongle->cooldown_until)
            {
                // Extraire la demande validée du tas
                min_heap_pop(&dongle->waiting_queue, coder->table->params.scheduler_type);
                dongle->is_available = 0;

                // Affichage sérialisé du log sous le format requis : "<timestamp> X has taken a dongle"
                print_log(coder, "has taken a dongle");
                pthread_mutex_unlock(&dongle->mutex);
                return (true);
            }
        // Cas B : Le dongle est libre mais encore en période de cooldown
        else if (dongle->is_available && now < dongle->cooldown_until)
        {
            cooldown_remaining = dongle->cooldown_until - now;
            get_timeout_timespec(&ts, cooldown_remaining);
            pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
        }

        // Cas C : Le dongle est actuellement occupé par un autre coder
        else
        {
            pthread_cond_wait(&dongle->cond, &dongle->mutex);
        }
        }
        else
        {
            // Le coder n'est pas encore le plus prioritaire dans la file
            pthread_cond_wait(&dongle->cond, &dongle->mutex);
        }
    }
}


static   void release_one(t_dongle *dongle, long long now, long long cooldown_time)
{
    pthread_mutex_lock(&dongle->mutex);
    dongle->is_available = 1;
    dongle->cooldown_until = now + cooldown_time;
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->mutex);
}

void release_dongles(t_coder *coder)
{
    t_dongle *dongles[2];
    long long now;


    dongles[0] = coder->left_dongle;
    dongles[1] = coder->right_dongle;
    now = get_current_time(coder->table);
    if (dongles[0] == dongles[1])
        release_one(dongles[0], now, coder->table->params.dongle_cooldown);
    else
    {
        for (int i = 0; i < 2; i++)
        {
            release_one(dongles[i], now, coder->table->params.dongle_cooldown);
        }
    }
}

bool acquire_both_dongles(t_coder *coder)
{
    t_dongle *first;
    t_dongle *second;

    // 1 seul coder : un seul dongle, il ne peut pas compiler -> il attend le burnout
    if (coder->left_dongle->id == coder->right_dongle->id)
    {
        if (take_dongle(coder, coder->left_dongle))
        {
            while (!should_stop_now(coder->table))
                usleep(500);
            release_one(coder->left_dongle, get_current_time(coder->table),
                coder->table->params.dongle_cooldown);
        }
        return false;
    }

    if (coder->left_dongle->id < coder->right_dongle->id)
    {
        first = coder->left_dongle;
        second = coder->right_dongle;
    }
    else{
        second = coder->left_dongle;
        first = coder->right_dongle;
    }
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
