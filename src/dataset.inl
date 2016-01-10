#include <stdio.h>
#include <string.h>
#include "datapath.h"

template <typename T>
DataSet<T>::DataSet(T (*load_func)(const char*), void (*destr_func)(T))
{
	load = load_func;
	destroy = destr_func;
}

template <typename T>
DataSet<T>::~DataSet()
{
	clear();
}

template <typename T>
void DataSet<T>::clear()
{
	if(destroy) {
		typename std::map<std::string, T>::iterator it = data.begin();
		while(it != data.end()) {
			destroy(it++->second);
		}
	}
	data.clear();
}

template <typename T>
void DataSet<T>::add(const char *name, T item)
{
    data[name] = item;
}

template <typename T>
T DataSet<T>::get(const char *name) const
{
	typename std::map<std::string, T>::const_iterator iter = data.find(name);
	if(iter != data.end()) {
		return iter->second;
	}

	const char *fname, *slash;
	if((slash = strrchr(name, '/'))) {
		fname = slash + 1;
	} else {
		fname = name;
	}

	std::string path = datafile_path(fname);
	if(path.empty()) {
		fprintf(stderr, "can't find data file: %s\n", name);
		return 0;
	}

	T res = load(path.c_str());
	if(res) {
		data[name] = res;
	}
	return res;
}
