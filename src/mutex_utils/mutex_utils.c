#include <pthread.h>

int	m_set_int(int *ptr, int new_value, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	*ptr = new_value;
	pthread_mutex_unlock(mutex);
	return new_value;
}

int m_get_int(int *ptr, pthread_mutex_t *mutex)
{
	int result;

	pthread_mutex_lock(mutex);
	result = *ptr;
	pthread_mutex_unlock(mutex);
	return (result);
}

unsigned long m_set_ulong(unsigned long *ptr, unsigned long new_value, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	*ptr = new_value;
	pthread_mutex_unlock(mutex);
	return new_value;
}

unsigned long m_get_ulong(unsigned long *ptr, pthread_mutex_t *mutex)
{
	unsigned long result;

	pthread_mutex_lock(mutex);
	result = *ptr;
	pthread_mutex_unlock(mutex);
	return (result);
}