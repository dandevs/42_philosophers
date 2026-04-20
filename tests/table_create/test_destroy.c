#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 5))
		return (fprintf(stderr, "table_create returned 0\n"), 1);
	table_free(&table);
	return (0);
}
