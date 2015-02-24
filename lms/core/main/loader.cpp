#include <core/loader.h>

#include <stdio.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>

#include <core/shared_base.h>

template<typename _Target> 
union converter {
    void* src; 
    _Target target;
};

Loader::Loader() {
	char path[PATH_MAX];
	memset (path, 0, PATH_MAX);
	if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
		perror("readlink failed");
		exit(1);
	}
	programm_directory = path;
	programm_directory = programm_directory.substr(0, programm_directory.rfind("/"));
	programm_directory = programm_directory.substr(0, programm_directory.rfind("/"));
	printf("Programm Directory: %s\n", programm_directory.c_str());

}

Loader::module_list Loader::getModules() {
    std::string place = "external/modules";
    module_list list;

	DIR *dp;
	dirent *d;
//	printf("Reading %s...\n", make_searchpath(place).c_str());
	if((dp = opendir( make_searchpath((char*)stringbuffer, place.c_str()) )) == NULL) {
		printf("Could not Open Directory: %s: ", place.c_str());
		perror("Reason: ");
		return Loader::module_list(); 
	}

	while((d = readdir(dp)) != NULL) {
		if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
		if (strcmp(d->d_name, "CMakeFiles") == 0) continue;
        if( d->d_type == DT_UNKNOWN )
        {
            // File-type unknown, check via stat()
            struct stat buf;
            make_searchpath( (char*) stringbuffer, ( place + std::string("/") + std::string(d->d_name) ).c_str() );
            if( -1 == stat((char*)stringbuffer, &buf) || !( S_ISDIR( buf.st_mode ) ) )
            {
                continue;
            }
        }
        else if( d->d_type != DT_DIR )
        {
            // Filetype could be determined, but is no directory -> skip
            continue;
        }

//		printf("Testing %s: %s\n", place.c_str(), d->d_name);

		
		char* libname = make_filename(d->d_name, place.c_str());
//		printf("\tOpening %s... ", libname.c_str());	
		void* lib = dlopen (libname, RTLD_LAZY);
		if (lib != NULL) {
//			printf("OK\n\tTesting for Necessary functions... ");
            //Testing for Necessary functions
			if (dlsym(lib, "getName") != NULL && dlsym(lib, "getInstance") != NULL) {
//				printf("OK\n");
                ///Union-Hack! To Avoid a warning message
                converter<const char*(*)()> conv; 
                conv.src = dlsym(lib,"getName");
				const char *name = conv.target();
				printf("\033[032mFound new %10s: %s\033[0m\n", place.c_str(), name);
				module_entry entry;
				entry.name = name;
				entry.module = d->d_name;
				entry.place = place;
				list.push_back(entry);
			} else {
				printf("\033[031mFailed. Could not find getName or getInstance in %s\033[0m",libname );
			}
			dlclose(lib);

			//Library successfull opened 
		} else { 
			printf("\033[031mFailed to Open %s %s\033[0m\n ", place.c_str(), libname);
			printf("\t Reason: ");
			auto f = fopen(libname, "rb");
			if (!f) 
				printf("\t\033[033mFile not found\033[0m\n");
			else {
				fclose(f);
				printf("\t\033[033mReason : %s\033[0m\n", dlerror());
			}
		}
	}
	closedir (dp);
	return list;
}

Shared_Base* Loader::load( const module_entry& entry) {
//	printf("Loading %s\n", make_filename(entry.module,entry.place).c_str());
	void *lib = dlopen(make_filename(entry.module.c_str(), entry.place.c_str()),RTLD_NOW);
	if(lib == NULL) {
		printf("\033[031mCould not open shared object \033[033m%s\033[0m\nReason: %s\n", 
				make_filename(entry.module.c_str(), entry.place.c_str()), dlerror());
		exit(1);
	}
	void* func = dlsym(lib, "getInstance");
	if (func == NULL) {
		perror("dlsym");

		printf("Could not find getInstance in %s\n", 
				make_filename(entry.module.c_str(), entry.place.c_str()));
		exit(1);
	}
    ///Union-Hack to avoid a warning message
    converter <void*(*)()> conv; 
    conv.src = func;
    
	return (Shared_Base*)conv.target();
}

void Loader::unload(Shared_Base* a) {
	delete a;
}


char* Loader::make_filename(const char* module, const char* place) {
	memset(stringbuffer, 0, sizeof(stringbuffer));
	make_searchpath(stringbuffer, place);
	strcat(stringbuffer, "/");
	strcat(stringbuffer, module);
	strcat(stringbuffer, "/lib");
	strcat(stringbuffer, place);
	strcat(stringbuffer, "_");
	strcat(stringbuffer, module);
	strcat(stringbuffer, ".so");
	
	return stringbuffer;
	//return make_searchpath(place) + 
	//		"/" + module + 
	//		"/lib" + place + "_" + module + ".so";
}

char* Loader::make_searchpath(char* buffer, const char* place) {
    strcpy(buffer, programm_directory.c_str());
	strcat(buffer, "/");
	strcat(buffer, place);
	return buffer;
}


