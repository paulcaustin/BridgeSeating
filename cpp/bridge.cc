// Bridge Rotation / Seating Chart optimizer
// Copyright 2003 by Paul Chamberlain
#include <iostream>

bool interrupted_flag = 0;

int main(int argc, char *argv) {
	parseargs(argc, argv);
	set_interrupts();
	BridgePopulation pop(args);
	while (pop.notdone() && !interrupted()) {
		pop.iterate();
		if (status_needed(pop)) {
			pop.generate_status(cin);
		}
	}
	pop.final_report(cin);
	return(0);
}

void set_interrupts() {
}

bool interrupted() {
	return interrupted_flag;
}

bool status_needed(BridgePopulation pop) {
	return pop.get_iteration_count() % 1000;
}
