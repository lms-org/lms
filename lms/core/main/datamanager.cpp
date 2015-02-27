#include <core/datamanager.h>
#include <core/module.h>

#include <deque>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <core/extra/colors.h>
#include <core/configurationmanager.h>

// TODO alternative? wann nicht definiert?
#define DATA_PACKET

namespace lms{
int DataManager::MemoryDelimiter = 0xefbeadde; //0xdeadbeef;

// TODO nein, ich will hier keine Kommandozeilen-Argumente
DataManager::DataManager () {
	data = NULL;
	data_size = 0;
}

DataManager::~DataManager () {
    if (data != NULL) {
        delete[] data;
        data = NULL;
    }
}
ConfigFile *DataManager::config(const char* configName) {
    return conf_mgr.getConfig(configName);
}

void DataManager::register_new_channel(GenericHandle *handle, std::string name, size_t length) {
    info_storage &info = retrieval[name].iterator->second;

    info.registered = true;
    info.length = length;
    // TODO der modulname kommt vom aktiven modul?
    info.registerer = activeModule->getName();
    info.handle = handle;
    data_size += length;
}

DataManager::info_storage& DataManager::acquire_channel_subscription(const char* name, Access::Operation op) {
	auto it = retrieval.find(name);

	if (it == retrieval.end()) {
		//Must allocate a new channel
		info_storage s;
		s.length = -1;
		s.registered = false;
		s.id = management.size();
		s.handle = NULL;
		auto res = management.insert(
				management.end(),
				std::pair<std::string, info_storage>(std::string(name), s));
		auto new_it = retrieval.insert(
            std::pair<std::string, storage_access>(
                std::string(name),
                storage_access(res, op)
            )
        );

		it = new_it.first;
        // TODO was soll diese fehlermeldung
		if (!new_it.second)
			printf(ERROR_COLORED("Could not acquire channel: Unknown Reason\n"));
	}

	it->second.iterator->second.acquired.push_back(activeModule->getName());
	return it->second.iterator->second;
}

void DataManager::set_channel(int handle, void* data, int length) {
	auto stor = memory[handle];

	if (length == -1)
		length = stor.length;
//	printf("%i IN: %i ", handle, stor.length);
//	for (int i = 0; i < length; i++)
//		printf("%x ", *(((unsigned char*)data)+i));
//	printf("\n");

	memcpy(stor.start, data, std::min(length, stor.length));
}

bool DataManager::get_channel(int handle, void** data, int* length) {
	auto stor = memory[handle];

//	void *data = stor.start;
//	printf("%i OUT: %i: @%i: %s ", handle, stor.length, stor.start, stor.name);
//	if (stor.length > 20)
//		exit(1);
//	for (int i = 0; i < stor.length; i++)
//		printf("%x ", *(((unsigned char*)data)+i));
//	printf("\n");
	if (length != NULL) {
		*length = stor.length;
	}
	*data = stor.start;
	return true;
}


void DataManager::initialize_data() {
	printf("\nInitializing... ");
	if (data != NULL)
		destroy_data();
	data_size += sizeof(MemoryDelimiter) * management.size();

	#ifdef DATA_PACKET
	data = new char[data_size];
	memset(data, 0, data_size);
	#endif

    memory.resize(management.size());
	char* start = data;

	bool ok = true;
	for (auto it = management.begin(); it != management.end(); ++it) {
//		printf("start: %i\tlength %i\n", start, it->length);
		if (!it->second.registered) {
			printf(ERROR_COLORED("Could not find Channel Name " EMPHASIZE("%s")), it->first.c_str());
			it->second.length = 0;
			ok = false;
		}

		memory[it->second.id].length = it->second.length;
		memory[it->second.id].name = it->first;
		memory[it->second.id].id = it->second.id;

		#ifdef DATA_PACKET
		memory[it->second.id].start = start;
		it->second.handle->set_default(start);
		start += it->second.length;
		memcpy(start, &MemoryDelimiter, sizeof(MemoryDelimiter));
		start += sizeof(MemoryDelimiter);

		#else
		memory[it->second.id].start = new char [it->second.length + sizeof(MemoryDelimiter) + 1];
		memset(memory[it->second.id].start, 0, it->second.length);
		memcpy(memory[it->second.id].start, &MemoryDelimiter, sizeof(MemoryDelimiter));

		#endif

	}
//	printf("\nData: %i \n", data_size);
//	for (int i = 0; i < data_size; ++i) {
//		printf("%02x ", (unsigned int) ((unsigned char*)data)[i]);
//
//	}

    // TODO warum jetzt auf Null setzen
	data_size = 0;
	if (!ok) {
		printf("\n\nErrors occurred in channel Mapping. Exiting.\n\n");
		exit(1);
	}
	printf("Done\n");
//	exit(0);
}

void DataManager::destroy_data() {
	if (data == NULL) return;
	for (auto it = management.begin(); it != management.end(); ++it) {
        it->second.handle->destroy(memory[it->second.id].start);
    }

    delete[] data;
    data = NULL;
}

void DataManager::print_mapping() {
	for (auto it = management.begin(); it != management.end(); ++it) {
		printf("\n");
		if (it->second.registered)
			printf(COLOR_YELLOW);
		else
			printf(COLOR_RED);

        // TODO lösche COLOR_WHITE
		printf ("%12s" COLOR_WHITE ": " GREEN("%15s") " [%6i] --> ",
			it->first.c_str(),
			it->second.registerer.c_str(),
            it->second.length
			);
		for (auto iit = it->second.acquired.begin();
				iit != it->second.acquired.end(); ++iit) {
			if (*iit != it->second.registerer)
				printf("%-15s ", iit->c_str());
		}

	}
}
}
/*
void DataManager::print_state() {
	printf("Currently Active Module: ");
	if (activeModule == NULL) {
		printf("\033[31mNULL\033[0m");
	} else {
		printf("%s", activeModule->getName().c_str());
	}

	printf("\n");
}
*/
