#include "lib.h"
#include "ctest.h"

int	main(void)
{
	char		*argv[] = {"prog", "5", "800", "200", "200", "7"};
	t_config	config;

	config = (t_config){0};
	ASSERT_TRUE(parse_arguments(6, argv, &config));
	ASSERT_EQ(config.meals_required, 7);
	ASSERT_EQ(config.philo_count, 5);
	return (0);
}
