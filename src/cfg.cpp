#include <stdlib.h>
#include <string.h>
#include <vmath/vmath.h>
#include "cfg.h"
#include "datapath.h"
#include "ncf/ncf.h"

#ifndef _MSC_VER
#include <alloca.h>
#else
#include <memory.h>
#endif

ConfigFile::ConfigFile()
{
    ncf=NULL;
	valid = false;
    own_ncf = false;
}

ConfigFile::ConfigFile(const char *fname)
{
    ncf=NULL;
	valid = false;
    own_ncf = false;
	open(fname);
}

ConfigFile::~ConfigFile()
{
    if(ncf && own_ncf)
        delete ncf;
}

bool ConfigFile::open(const char *fname)
{
    ncf=new NCF;
    own_ncf=true;
    ncf->set_source(datafile_path(fname).c_str());
	if(ncf->parse() != 0) {
		valid = false;
		return false;
	}

	valid = true;
	return true;
}

bool ConfigFile::is_open() const
{
	return valid;
}

ConfigEntry ConfigFile::get(const char *optname) const
{
	if(!valid) {
		return ConfigEntry();
	}

	char *path = (char*)alloca(strlen(optname) + 1);
	strcpy(path, optname);

    const NCF *node = ncf;

	char *dot;
	while(node && (dot = strchr(path, '.'))) {
		*dot = 0;
        node = node->get_group_by_name(path);
		path = dot + 1;
	}

	if(!path || !node || !node->query_property(path)) {
		return ConfigEntry();
	}

    return ConfigEntry(path, node->get_property_by_name(path));
}


std::list<ConfigEntry> ConfigFile::get_all(const char *groupname) const
{
	std::list<ConfigEntry> res;
	if(!valid) {
		return res;
	}

	char *path = (char*)alloca(strlen(groupname) + 1);
	strcpy(path, groupname);

    const NCF *node = ncf;

	char *dot;
	while(node && (dot = strchr(path, '.'))) {
		*dot = 0;
        node = node->get_group_by_name(path);
		path = dot + 1;
	}

    if(!path || !node || !(node = node->get_group_by_name(path))) {
		return res;
	}

	for(unsigned int i=0; i<node->count_properties(); i++) {
        ConfigEntry ent(node->get_property_name_by_index(i), node->get_property_by_index(i));
		res.push_back(ent);
	}
	return res;
}


const char *ConfigFile::get_str(const char *optname, const char *def) const
{
	ConfigEntry ent = get(optname);
	if(ent.is_valid()) {
		return ent.value_str();
	}
	return def;
}

int ConfigFile::get_int(const char *optname, int def) const
{
	ConfigEntry ent = get(optname);
	if(ent.is_valid()) {
		return ent.value_int();
	}
	return def;
}

float ConfigFile::get_num(const char *optname, float def) const
{
	ConfigEntry ent = get(optname);
	if(ent.is_valid()) {
		return ent.value_num();
	}
	return def;
}

Vector4 ConfigFile::get_vec(const char *optname, const Vector4 &def) const
{
	ConfigEntry ent = get(optname);
	if(ent.is_valid()) {
		return ent.value_vec();
	}
	return def;
}

void ConfigFile::set_ncf(NCF *n)
{
    if(own_ncf)
        delete ncf;
    ncf=n;
    valid=true;
    own_ncf=false;
}


// --- ConfigEntry ---
ConfigEntry::ConfigEntry()
{
	name = valstr = 0;
}

ConfigEntry::ConfigEntry(const char *name, const char *val)
{
	this->name = name;
	valstr = val;
}

bool ConfigEntry::is_valid() const
{
	return valstr != 0;
}

const char *ConfigEntry::get_name() const
{
	return name;
}

const char *ConfigEntry::value_str() const
{
	return valstr;
}

bool ConfigEntry::is_number() const
{
	if(!valstr) {
		return false;
	}

	char *endp;
	strtod(valstr, &endp);

	return endp != valstr;
}

int ConfigEntry::value_int() const
{
	return atoi(valstr);
}

float ConfigEntry::value_num() const
{
	return atof(valstr);
}

Vector4 ConfigEntry::value_vec() const
{
	float v[4] = {0, 0, 0, 1};
	sscanf(valstr, "%f %f %f %f", v, v + 1, v + 2, v + 3);
	return Vector4(v[0], v[1], v[2], v[3]);
}

