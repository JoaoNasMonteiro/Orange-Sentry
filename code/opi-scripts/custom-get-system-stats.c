#include <stdio.h>
#include <unistd.h>

//a script to obtain the system stats of the system through reading files
//Stuff to fetch: CPU usage, CPU temperature, % RAM usage
//can be ran once (standard) or to monitor many times thorugh -m
//can be redirected to a specific file through -o]


typedef struct {
	unsigned long long user, nice, sys, idle, iowait, irq, softirq, steal, total;
} cpu_stats;

int get_cpu_temp(float* temp);
int get_raw_cpu_usage(cpu_stats* cpu_usage);
int get_cpu_stats();

int main(){
get_cpu_stats();
}

int get_cpu_temp(float* temp){
	FILE* f = fopen("/sys/class/thermal/thermal_zone1/temp", "r");
	if (f == NULL) {
		printf("Error - failed to open file");
		return 1;
	}
	int millidegrees;
	if(fscanf(f, "%d", &millidegrees) != 1){
		fclose(f);
		return 2;
	}
	*temp = millidegrees / 1000.0;
	fclose(f);
	return 0;
}

int get_raw_cpu_usage(cpu_stats* stats){
	FILE* f = fopen("/proc/stat", "r");

	char label[5];
	if(fscanf(f, "%s %llu %llu %llu %llu %llu %llu %llu %llu", 
           label, 
           &stats->user, &stats->nice, &stats->sys, &stats->idle, 
           &stats->iowait, &stats->irq, &stats->softirq, &stats->steal) != 9){
		fclose(f);
		return 2;
	}

	stats->total = stats->user + stats->nice + stats->sys + stats->idle + stats->iowait + stats->irq + stats->softirq + stats->steal;
	
	fclose(f);
	return 0;	
}

int get_cpu_stats(){
	cpu_stats current_stats;
	cpu_stats prev_stats;

	float cpu_temp;
	get_raw_cpu_usage(&current_stats);

	float cpu_usage;

	unsigned long long dTotal, dIdle; 
	
	while (1){
		sleep(1);
		prev_stats = current_stats;
		get_raw_cpu_usage(&current_stats); 

		dTotal = current_stats.total - prev_stats.total;
		dIdle = (current_stats.idle + current_stats.iowait) - (prev_stats.idle + prev_stats.iowait);

		if (dTotal == 0){
			cpu_usage = 0.0;
		} else {
			cpu_usage = (1 - ((double)dIdle / (double)dTotal)) * 100;
		}
		


		get_cpu_temp(&cpu_temp);

		printf("A temperatura é %f, o uso da cpu está em %f%% \n", cpu_temp, cpu_usage);

		
	}
	return (0);
}