#include <pthread.h>

int	set_int(pthread_mutex_t *mutex, int *ptr, int new_value)
{
	pthread_mutex_lock(mutex);
	*ptr = new_value;
	pthread_mutex_unlock(mutex);
	return new_value;
}

int get_int(pthread_mutex_t *mutex, int *ptr)
{
	int result;

	pthread_mutex_lock(mutex);
	result = *ptr;
	pthread_mutex_unlock(mutex);
	return (result);
}

unsigned long set_ulong(pthread_mutex_t *mutex, unsigned long *ptr, unsigned long new_value)
{
	pthread_mutex_lock(mutex);
	*ptr = new_value;
	pthread_mutex_unlock(mutex);
	return new_value;
}

unsigned long get_ulong(pthread_mutex_t *mutex, unsigned long *ptr)
{
	unsigned long result;

	pthread_mutex_lock(mutex);
	result = *ptr;
	pthread_mutex_unlock(mutex);
	return (result);
}