#include "table.h"
#include "lib.h"
#include <stdlib.h>

void	table_free(t_table *table)
{
	if (!table)
		return ;

	free(table->forks);
	free(table->philosophers);
}