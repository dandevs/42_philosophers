#include "lib.h"
#include <stdio.h>

int	parse_int(char *str, int *value);

int	main(void)
{
	int	value;
	int	ret;

	value = -1;
	ret = parse_int("1", &value);
	if (!ret || value != 1)
	{
		printf("parse_int(\"1\") expected ret 1/value 1, got %d/%d",
			ret, value);
		return (1);
	}
	ret = parse_int("42", &value);
	if (!ret || value != 42)
	{
		printf("parse_int(\"42\") expected ret 1/value 42, got %d/%d",
			ret, value);
		return (1);
	}
	ret = parse_int("1000", &value);
	if (!ret || value != 1000)
	{
		printf("parse_int(\"1000\") expected ret 1/value 1000, got %d/%d",
			ret, value);
		return (1);
	}
	return (0);
}
