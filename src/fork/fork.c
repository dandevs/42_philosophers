#include "fork.h"

int	fork_init(t_fork *fork)
{
	fork->available = 1;
	return (pthread_mutex_init(&fork->mutex, NULL) == 0);
}