#include <stdio.h>
#include <set>
#include <string>
#include "logger.h"
#include "datapath.h"

static std::set<std::string> paths;

void add_data_path(const char *path)
{
	paths.insert(path);
}

#ifndef TARGET_IPHONE
std::string datafile_path(const char *fname)
{
	std::string res;
	if(!fname) {
		return res;
	}

	std::set<std::string>::const_iterator it = paths.begin();
	while(it != paths.end()) {
		const std::string &path = *it++;
		res = path + "/" + std::string(fname);
		FILE *fp = fopen(res.c_str(), "r");
		if(fp) {
			fclose(fp);
			return res;
		}
	}

	// It's not found. Return an empty string
	return std::string();
}
#else
#include <CoreFoundation/CoreFoundation.h>

std::string datafile_path(const char *fname)
{
	std::string res;
	if(!fname) {
		return res;
	}

	CFBundleRef bundle;
	CFURLRef url;
	CFStringRef cfname;

	cfname = CFStringCreateWithCString(0, fname, kCFStringEncodingASCII);

	bundle = CFBundleGetMainBundle();
	if(!(url = CFBundleCopyResourceURL(bundle, cfname, 0, 0))) {
		return fname;
	}

	char path[1024];
	if(!CFURLGetFileSystemRepresentation(url, 1, (unsigned char*)path, sizeof path)) {
		return fname;
	}
	return std::string(path);
}
#endif
