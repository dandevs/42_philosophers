#include "lib.h"
#include "ctest.h"

int	main(void)
{
	char		*argv[] = {"prog", "5", "800", "200", "200"};
	t_config	config;

	config = (t_config){0};
	ASSERT_TRUE(parse_arguments(5, argv, &config));
	ASSERT_EQ(config.philo_count, 5);
	ASSERT_EQ(config.time_to_die_ms, 800ul);
	ASSERT_EQ(config.time_to_eat_ms, 200ul);
	ASSERT_EQ(config.time_to_sleep_ms, 200ul);
	ASSERT_EQ(config.meals_required, -1);
	return (0);
}
