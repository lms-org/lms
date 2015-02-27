#ifndef IMPORTER_SCHEDULER_IMPORTER_H
#define IMPORTER_SCHEDULER_IMPORTER_H

#include <core/datamanager.h>

#include <core/shared_base.h>
#include <core/datamanager.h>
#include <core/datamanager.h>
#include <time.h>
#include <sys/time.h>

class Scheduler : public lms::Shared_Base {
    IMPLEMENT_BASE
public:
	bool initialize();
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
