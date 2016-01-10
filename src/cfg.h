#ifndef CFG_H_
#define CFG_H_

#include <string>
#include <list>
#include "ncf/ncf.h"

class ConfigEntry;

class ConfigFile {
private:
    NCF *ncf;
    bool own_ncf;
	bool valid;

public:
	ConfigFile();
	ConfigFile(const char *fname);
	~ConfigFile();

	bool open(const char *fname);

	bool is_open() const;

	ConfigEntry get(const char *optname) const;
	std::list<ConfigEntry> get_all(const char *groupname) const;

	// convenience functions
	const char *get_str(const char *optname, const char *def = 0) const;
	int get_int(const char *optname, int def = 0) const;
	float get_num(const char *optname, float def = 0.0f) const;
	Vector4 get_vec(const char *optname, const Vector4 &def = Vector4(0, 0, 0, 1)) const;
    void set_ncf(NCF *n);
};

class ConfigEntry {
private:
	const char *name, *valstr;

public:
	ConfigEntry();
	ConfigEntry(const char *name, const char *val);

	bool is_valid() const;

	const char *get_name() const;
	const char *value_str() const;

	bool is_number() const;

	int value_int() const;
	float value_num() const;
	Vector4 value_vec() const;
};


#endif	// CFG_H_
