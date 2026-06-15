#include "mutex_utils.h"
#include <stdio.h>

int	main(void)
{
	pthread_mutex_t	mutex;
	unsigned long	value;
	unsigned long	set_ret;

	pthread_mutex_init(&mutex, NULL);
	value = 0;
	set_ret = m_set_ulong(&value, 999999ul, &mutex);
	if (set_ret != 999999ul)
	{
		printf("m_set_ulong expected to return 999999, got %lu", set_ret);
		return (1);
	}
	if (m_get_ulong(&value, &mutex) != 999999ul)
	{
		printf("m_get_ulong expected 999999, got %lu",
			m_get_ulong(&value, &mutex));
		return (1);
	}
	pthread_mutex_destroy(&mutex);
	return (0);
}
