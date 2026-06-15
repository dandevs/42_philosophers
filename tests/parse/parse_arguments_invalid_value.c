#include "lib.h"
#include <stdio.h>

int	main(void)
{
	char		*argv[] = {"prog", "5", "abc", "200", "200"};
	t_config	config;

	config = (t_config){0};
	if (parse_arguments(5, argv, &config))
	{
		printf("parse_arguments with non-numeric arg expected 0, got 1");
		return (1);
	}
	return (0);
}
