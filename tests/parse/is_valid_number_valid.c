#include "lib.h"
#include "ctest.h"

int	is_valid_number(char *str);

int	main(void)
{
	char	*valid[] = {"42", "+42", " 42", "42 ", "  +42  ", "0001", "0"};
	int		i;

	i = 0;
	while (i < (int)(sizeof(valid) / sizeof(valid[0])))
	{
		ASSERT_TRUE(is_valid_number(valid[i]));
		i++;
	}
	return (0);
}
