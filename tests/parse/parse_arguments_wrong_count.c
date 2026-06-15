#include "lib.h"
#include <stdio.h>

int	main(void)
{
	char		*argv[] = {"prog", "5", "800", "200", "200", "7", "x"};
	t_config	config;

	config = (t_config){0};
	if (parse_arguments(4, argv, &config))
	{
		printf("parse_arguments(4 args) expected 0, got 1");
		return (1);
	}
	if (parse_arguments(7, argv, &config))
	{
		printf("parse_arguments(7 args) expected 0, got 1");
		return (1);
	}
	return (0);
}
