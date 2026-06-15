#include "lib.h"
#include <stdio.h>

int	parse_ulong(char *str, unsigned long *value);

int	main(void)
{
	unsigned long	value;
	int				ret;

	ret = parse_ulong("1", &value);
	if (!ret || value != 1)
	{
		printf("parse_ulong(\"1\") expected ret 1/value 1, got %d/%lu",
			ret, value);
		return (1);
	}
	ret = parse_ulong("42", &value);
	if (!ret || value != 42)
	{
		printf("parse_ulong(\"42\") expected ret 1/value 42, got %d/%lu",
			ret, value);
		return (1);
	}
	ret = parse_ulong("1000000", &value);
	if (!ret || value != 1000000)
	{
		printf("parse_ulong(\"1000000\") expected value 1000000, got %lu",
			value);
		return (1);
	}
	return (0);
}
