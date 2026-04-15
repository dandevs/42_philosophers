#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 5))
	{
		printf("table_create returned 0\n");
		return (1);
	}
	table_destroy(&table);
	return (0);
}
