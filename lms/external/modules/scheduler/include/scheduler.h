#ifndef IMPORTER_SCHEDULER_IMPORTER_H
#define IMPORTER_SCHEDULER_IMPORTER_H

#include <core/datamanager.h>

#include <core/module.h>
#include <core/datamanager.h>
#include <core/datamanager.h>
#include <time.h>
#include <sys/time.h>

class Scheduler : public lms::Module {

public:

    bool initialize(lms::DataManager* d);
	bool deinitialize();

	bool cycle();

protected:
    double looptime;
    double warn_tolerance;
    double cycletime;

    timeval last;
    timeval cycleTimeStart;
    timeval cycleTimeEnd;
};

#endif
