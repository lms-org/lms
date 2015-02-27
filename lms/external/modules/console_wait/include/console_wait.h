#ifndef IMPORTER_CONSOLE_WAIT_IMPORTER_H
#define IMPORTER_CONSOLE_WAIT_IMPORTER_H

#include <core/datamanager.h>
#include <core/shared_base.h>

#include <termios.h>

class Console_wait : public lms::Shared_Base {
public:
	bool initialize();
	bool deinitialize();

	bool cycle();

protected:
	termios saved_attributes;
	

};

#endif
