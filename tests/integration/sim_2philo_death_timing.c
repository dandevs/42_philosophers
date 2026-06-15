#include "lib.h"
#include "table/table.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define CAPTURE_PATH "/tmp/philo_2philo_death.txt"
#define TTD 310

int	main(void)
{
	t_table			table;
	t_config		config;
	int				saved;
	int				ret;
	FILE			*f;
	char			line[256];
	unsigned long	death_t;
	int				found;
	int				id;

	config = (t_config){0};
	config.philo_count = 2;
	config.meals_required = -1;
	config.time_to_die_ms = TTD;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 100;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	saved = dup(1);
	freopen(CAPTURE_PATH, "w", stdout);
	ret = table_main_routine(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	if (!ret)
	{
		printf("table_main_routine returned 0");
		return (1);
	}
	f = fopen(CAPTURE_PATH, "r");
	if (!f)
	{
		printf("could not open capture file");
		return (1);
	}
	death_t = 0;
	found = 0;
	id = 0;
	while (fgets(line, sizeof(line), f))
	{
		if (strstr(line, " died"))
		{
			sscanf(line, "%lu %d", &death_t, &id);
			found = 1;
		}
	}
	fclose(f);
	unlink(CAPTURE_PATH);
	if (!found)
	{
		printf("no death message captured");
		return (1);
	}
	if (death_t < (unsigned long)TTD)
	{
		printf("death at %lums before time_to_die %d", death_t, TTD);
		return (1);
	}
	if (death_t > (unsigned long)TTD + 10)
	{
		printf("death delayed to %lums (more than 10ms after TTD %d)",
			death_t, TTD);
		return (1);
	}
	table_free(&table);
	return (0);
}
