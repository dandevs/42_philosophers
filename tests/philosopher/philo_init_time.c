#include "lib.h"
#include "table/table.h"
#include "philosopher/utils.h"
#include "ctest.h"

int	main(void)
{
	t_table			table;
	t_config		config;
	unsigned long	before;
	unsigned long	after;
	int				i;

	config = (t_config){0};
	config.philo_count = 3;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	before = get_time_ms();
	philo_init_time(&table);
	after = get_time_ms();
	ASSERT_GE(table.start_time, before);
	ASSERT_LE(table.start_time, after);
	i = 0;
	while (i < config.philo_count)
	{
		ASSERT_EQ(table.philosophers[i].time_began_eating, table.start_time);
		ASSERT_EQ(table.philosophers[i].start_time, table.start_time);
		i++;
	}
	table_free(&table);
	return (0);
}
