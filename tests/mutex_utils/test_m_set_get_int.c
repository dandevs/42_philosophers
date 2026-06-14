#include "mutex_utils.h"
#include <stdio.h>

int	main(void)
{
	pthread_mutex_t	mutex;
	int				value;
	int				set_ret;

	pthread_mutex_init(&mutex, NULL);
	value = 0;
	set_ret = m_set_int(&value, 42, &mutex);
	if (set_ret != 42)
	{
		printf("m_set_int expected to return 42, got %d", set_ret);
		return (1);
	}
	if (m_get_int(&value, &mutex) != 42)
	{
		printf("m_get_int expected 42, got %d", m_get_int(&value, &mutex));
		return (1);
	}
	pthread_mutex_destroy(&mutex);
	return (0);
}
