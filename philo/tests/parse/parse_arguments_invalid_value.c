#include "lib.h"
#include "ctest.h"

int	main(void)
{
	char		*argv[] = {"prog", "5", "abc", "200", "200"};
	t_config	config;

	config = (t_config){0};
	ASSERT_FALSE(parse_arguments(5, argv, &config));
	return (0);
}
