#include "mutex_utils.h"
#include <stdio.h>

int	main(void)
{
	pthread_mutex_t	mutex;
	int				value;

	pthread_mutex_init(&mutex, NULL);
	value = 0;
	m_set_int(&value, 1, &mutex);
	m_set_int(&value, 2, &mutex);
	m_set_int(&value, 3, &mutex);
	if (m_get_int(&value, &mutex) != 3)
	{
		printf("after 3 sets, m_get_int expected 3, got %d",
			m_get_int(&value, &mutex));
		return (1);
	}
	pthread_mutex_destroy(&mutex);
	return (0);
}
