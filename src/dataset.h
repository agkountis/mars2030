/** DataSet is a generic resource database with fast O(logn) lookups by name
 * it can be used for texture managers, mesh managers, sound effect managers etc
 *
 * The constructor takes a load function and a destructor function to be called
 * when a nonexistent resource is requested and needs to be loaded, and when
 * the DataSet is destroyed. The destructor is optional and can be set to null
 * if not needed.
 *
 * Requesting a resource works by simply calling get, example:
 * ----------------------------------------------------------
 * \code
 * Texture *load_texture(const char *fname);
 * void free_texture(Texture *tex);
 *
 * DataSet<Texture*> texman(load_texture, free_texture);
 * Texture *foo = texman.get("foo.png");
 * \endcode
 */
#ifndef DATASET_H_
#define DATASET_H_

#include <string>
#include <map>

template <typename T>
class DataSet {
protected:
	mutable std::map<std::string, T> data;

	T (*load)(const char*);
	void (*destroy)(T);

public:
	DataSet(T (*load_func)(const char*), void (*destr_func)(T) = 0);
	~DataSet();

	void clear();

    void add(const char *name, T item);
	T get(const char *name) const;
};

#include "dataset.inl"

#endif	// DATASET_H_
