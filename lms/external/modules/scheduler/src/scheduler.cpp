#include <scheduler.h>
#include <core/executionmanager.h>
#include <core/type/static_image.h>
#include <stdio.h>
#include <unistd.h>

// TODO umbenennen, name irreführend

bool Scheduler::initialize() {
	printf("Init: scheduler\n");
    /*
     * That error has to be fixed using cmake as it will also happen to all modules
     *
     */
    datamanager()->writeChannel<std::string>(this, "STRING");

    lms::type::StaticImage<320,240,char> *image =
            datamanager()->writeChannel<lms::type::StaticImage<320,240,char>>(this, "IMAGE_BLA");

    char b = (*image)(100,20);
    image->fill(12);

    //lms::ConfigFile *config = datamanager()->config("scheduler");

    looptime = 10.; //config->get_or_default("looptime", (double)10.);
    warn_tolerance = 3;//config->get_or_default("warn_tolerance", (double)3.);
    gettimeofday(&last, NULL);
	return true;
}

bool Scheduler::deinitialize() {
    return true;
}

bool Scheduler::cycle () {

    timeval now;

    gettimeofday(&now, NULL);

    double delta = (now.tv_sec - last.tv_sec + 1e-6 * (now.tv_usec - last.tv_usec)) * 1e3;

    double sleep = looptime - delta;
    if (sleep > 0) {
        printf("Scheduler sleeps for %fms Cycle rate of %2.2fHz\n", sleep, (1/(delta + sleep) * 1e3));
        usleep(sleep * 1e3);
    }
    /// Zeitmessung Zykluszeit
    gettimeofday(&cycleTimeEnd, NULL);
    cycletime = (cycleTimeEnd.tv_sec - cycleTimeStart.tv_sec + 1e-6 * (cycleTimeEnd.tv_usec - cycleTimeStart.tv_usec)) * 1e3;
    gettimeofday(&cycleTimeStart, NULL);
    printf("\033[031m Cycletime: %fms \n \033[0m", cycletime);

    /// Zeitmessung Scheduler
    gettimeofday(&last, NULL);

    if (delta > looptime + warn_tolerance) {
        printf("\033[031mCYCLING TOO SLOW: \033[0m%fms per Loop\n", delta);
        /*std::map<std::string, double> timing;
        ExecutionManager::getTiming(timing);
		for (auto it = timing.begin(); it != timing.end(); ++it) {
			printf("\tModule % 25s: %fms\n", it->first.c_str(), it->second * 1e-3);
        }*/
    }
	return true;
}

