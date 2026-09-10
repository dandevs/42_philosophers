#include "lib.h"
#include "ctest.h"

int	main(void)
{
	char		*argv[] = {"prog", "5", "800", "200", "200", "7", "x"};
	t_config	config;

	config = (t_config){0};
	ASSERT_FALSE(parse_arguments(4, argv, &config));
	ASSERT_FALSE(parse_arguments(7, argv, &config));
	return (0);
}
