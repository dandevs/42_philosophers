#include "lib.h"
#include <stdio.h>

int	parse_ulong(char *str, unsigned long *value);

int	main(void)
{
	unsigned long	value;
	int				ret;

	value = 999;
	ret = parse_ulong("0", &value);
	if (ret)
	{
		printf("parse_ulong(\"0\") expected 0, got 1 (value %lu)", value);
		return (1);
	}
	return (0);
}
