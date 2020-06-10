#ifndef JSOBJECT_HPP
#define JSOBJECT_HPP

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include <limits>
#include <cmath>

#define JS_TYPE_INVALID -1
#define JS_TYPE_NULL     0
#define JS_TYPE_BOOLEAN  1
#define JS_TYPE_INTEGER  2
#define JS_TYPE_FLOAT    3
#define JS_TYPE_TEXT     4
#define JS_TYPE_ARRAY    5
#define JS_TYPE_OBJECT   6

#define JS_NULL (void*)NULL

class jsarray;
class jsobject;

class jsvar {
private:
	int type;
	bool boolean;
	long long inumber;
	double fnumber;
	std::string text;
	class jsarray *array;
	class jsobject *object;

public:
	jsvar();
	jsvar(bool value);
	jsvar(char value);
	jsvar(short value);
	jsvar(int value);
	jsvar(long value);
	jsvar(long long value);
	jsvar(unsigned char value);
	jsvar(unsigned short value);
	jsvar(unsigned int value);
	jsvar(unsigned long value);
	jsvar(unsigned long long value);
	jsvar(float value);
	jsvar(double value);
	jsvar(const char *value);
	jsvar(std::string value);
	jsvar(class jsarray &value);
	jsvar(class jsobject &value);
	jsvar(void *value);
	~jsvar();

	operator bool();
	operator char();
	operator short();
	operator int();
	operator long();
	operator long long();
	operator unsigned char();
	operator unsigned short();
	operator unsigned int();
	operator unsigned long();
	operator unsigned long long();
	operator float();
	operator double();
	operator std::string();
	operator class jsarray&();
	operator class jsobject&();

	jsvar& operator=(bool value);
	jsvar& operator=(char value);
	jsvar& operator=(short value);
	jsvar& operator=(int value);
	jsvar& operator=(long value);
	jsvar& operator=(long long value);
	jsvar& operator=(unsigned char value);
	jsvar& operator=(unsigned short value);
	jsvar& operator=(unsigned int value);
	jsvar& operator=(unsigned long value);
	jsvar& operator=(unsigned long long value);
	jsvar& operator=(float value);
	jsvar& operator=(double value);
	jsvar& operator=(const char *value);
	jsvar& operator=(std::string value);
	jsvar& operator=(class jsarray &value);
	jsvar& operator=(class jsobject &value);
	jsvar& operator=(void *value);

	jsvar& operator[](int index);
	jsvar& operator[](size_t index);
	jsvar& operator[](const char *key);
	jsvar& operator[](std::string key);

	friend std::ostream &operator<<(std::ostream &output, jsvar &var);

	int getType();
	std::string toString(bool pretty = false, int indent = 0);

	void append(jsvar &value);
	void append(bool value);
	void append(char value);
	void append(short value);
	void append(int value);
	void append(long value);
	void append(long long value);
	void append(unsigned char value);
	void append(unsigned short value);
	void append(unsigned int value);
	void append(unsigned long value);
	void append(unsigned long long value);
	void append(float value);
	void append(double value);
	void append(const char *value);
	void append(std::string value);
	void append(class jsarray &value);
	void append(class jsobject &value);
	void append(void *value);
	void remove(int index);
	void remove(size_t index);
	void remove(const char *key);
	void remove(std::string key);
	size_t length();
	std::vector<std::string> keys();
	bool hasProperty(std::string key);
	void clear();
	std::string stringify(bool pretty = false, int indent = 0);

	static jsvar jsarray();
	static jsvar jsobject();
};

class jsarray {
private:
	std::vector<jsvar> list;

public:
	jsarray();
	~jsarray();

	jsvar& operator[](int index);
	jsvar& operator[](size_t index);

	void append(jsvar &value);
	void append(bool value);
	void append(char value);
	void append(short value);
	void append(int value);
	void append(long value);
	void append(long long value);
	void append(unsigned char value);
	void append(unsigned short value);
	void append(unsigned int value);
	void append(unsigned long value);
	void append(unsigned long long value);
	void append(float value);
	void append(double value);
	void append(const char *value);
	void append(std::string value);
	void append(jsarray &value);
	void append(jsobject &value);
	void append(void *value);
	void remove(int index);
	void remove(size_t index);
	size_t length();
	void clear();

	std::string stringify(bool pretty = false, int indent = 0);

	static jsvar parse(std::string json, size_t *headPtr = NULL);
};

class jsobject {
private:
	std::map<std::string,jsvar> dict;

public:
	jsobject();
	~jsobject();

	jsvar& operator[](const char *key);
	jsvar& operator[](std::string key);

	void remove(const char *key);
	void remove(std::string key);
	std::vector<std::string> keys();
	bool hasProperty(std::string key);
	void clear();

	std::string stringify(bool pretty = false, int indent = 0);

	static jsvar parse(std::string json, size_t *headPtr = NULL);
	static jsvar parseFromFile(std::string filename);
};


/***************************************************************
 *  public methods: jsvar                                      *
 ***************************************************************/
inline jsvar::jsvar() {
	type = JS_TYPE_INVALID;
}

inline jsvar::jsvar(bool value) {
	type = JS_TYPE_BOOLEAN;
	boolean = value;
}

inline jsvar::jsvar(char value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(short value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(int value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(long value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(long long value) {
	type = JS_TYPE_INTEGER;
	inumber = value;
}

inline jsvar::jsvar(unsigned char value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(unsigned short value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(unsigned int value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(unsigned long value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(unsigned long long value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
}

inline jsvar::jsvar(float value) {
	type = JS_TYPE_FLOAT;
	fnumber = (double)value;
}

inline jsvar::jsvar(double value) {
	type = JS_TYPE_FLOAT;
	fnumber = value;
}

inline jsvar::jsvar(const char *value) {
	type = JS_TYPE_TEXT;
	text = std::string(value);
}

inline jsvar::jsvar(std::string value) {
	type = JS_TYPE_TEXT;
	text = value;
}

inline jsvar::jsvar(class jsarray &value) {
	type = JS_TYPE_ARRAY;
	array = &value;
}

inline jsvar::jsvar(class jsobject &value) {
	type = JS_TYPE_OBJECT;
	object = &value;
}

inline jsvar::jsvar(void *value) {
	if (value != NULL) {
		fprintf(stderr, "Invalid access of jsvar type null\n");
		exit(1);
	}
	type = JS_TYPE_NULL;
}

inline jsvar::~jsvar() {
}

inline jsvar::operator bool() {
	if (type != JS_TYPE_BOOLEAN) {
		fprintf(stderr, "Invalid access of jsvar type boolean\n");
		exit(1);
	}
	return boolean;
}

inline jsvar::operator char() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (char)inumber;
	return (char)fnumber;
}

inline jsvar::operator short() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (short)inumber;
	return (short)fnumber;
}

inline jsvar::operator int() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (int)inumber;
	return (int)fnumber;
}

inline jsvar::operator long() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (long)inumber;
	return (long)fnumber;
}

inline jsvar::operator long long() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return inumber;
	return (long long)fnumber;
}

inline jsvar::operator unsigned char() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (unsigned char)inumber;
	return (unsigned char)fnumber;
}

inline jsvar::operator unsigned short() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (unsigned short)inumber;
	return (unsigned short)fnumber;
}

inline jsvar::operator unsigned int() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (unsigned int)inumber;
	return (unsigned int)fnumber;
}

inline jsvar::operator unsigned long() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (unsigned long)inumber;
	return (unsigned long)fnumber;
}

inline jsvar::operator unsigned long long() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (unsigned long long)inumber;
	return (unsigned long long)fnumber;
}

inline jsvar::operator float() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (float)inumber;
	printf("conversion to float: %.6f\n", (float)fnumber);
	return (float)fnumber;
}

inline jsvar::operator double() {
	if (type != JS_TYPE_INTEGER && type != JS_TYPE_FLOAT) {
		fprintf(stderr, "Invalid access of jsvar type number\n");
		exit(1);
	}
	if (type == JS_TYPE_INTEGER)
		return (double)inumber;
	return fnumber;
}

inline jsvar::operator std::string() {
	if (type != JS_TYPE_TEXT) {
		fprintf(stderr, "Invalid access of jsvar type text\n");
		exit(1);
	}
	return text;
}

inline jsvar::operator class jsarray&() {
	if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	return *array;
}

inline jsvar::operator class jsobject&() {
	if (type != JS_TYPE_OBJECT) {
		fprintf(stderr, "Invalid access of jsvar type object\n");
		exit(1);
	}
	return *object;
}

inline jsvar& jsvar::operator=(bool value) {
	type = JS_TYPE_BOOLEAN;
	boolean = value;
	return *this;
}

inline jsvar& jsvar::operator=(char value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(short value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(int value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(long value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(long long value) {
	type = JS_TYPE_INTEGER;
	inumber = value;
	return *this;
}

inline jsvar& jsvar::operator=(unsigned char value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(unsigned short value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(unsigned int value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(unsigned long value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(unsigned long long value) {
	type = JS_TYPE_INTEGER;
	inumber = (long long)value;
	return *this;
}

inline jsvar& jsvar::operator=(float value) {
	type = JS_TYPE_FLOAT;
	fnumber = (double)value;
	return *this;
}

inline jsvar& jsvar::operator=(double value) {
	type = JS_TYPE_FLOAT;
	fnumber = value;
	return *this;
}

inline jsvar& jsvar::operator=(const char *value) {
	type = JS_TYPE_TEXT;
	text = std::string(value);
	return *this;
}

inline jsvar& jsvar::operator=(std::string value) {
	type = JS_TYPE_TEXT;
	text = value;
	return *this;
}

inline jsvar& jsvar::operator=(class jsarray &value) {
	type = JS_TYPE_ARRAY;
	array = &value;
	return *this;
}

inline jsvar& jsvar::operator=(class jsobject &value) {
	type = JS_TYPE_OBJECT;
	object = &value;
	return *this;
}

inline jsvar& jsvar::operator=(void *value) {
	if (value != NULL) {
		fprintf(stderr, "Invalid access of jsvar type null\n");
		exit(1);
	}
	type = JS_TYPE_NULL;
	return *this;
}

inline jsvar& jsvar::operator[](int index) {
	return (*this)[(size_t)index];
}

inline jsvar& jsvar::operator[](size_t index) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	return (*array)[index];
}

inline jsvar& jsvar::operator[](const char *key) {
	return (*this)[std::string(key)];
}

inline jsvar& jsvar::operator[](std::string key) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_OBJECT;
		object = new class jsobject();
	}
	else if (type != JS_TYPE_OBJECT) {
		fprintf(stderr, "Invalid access of jsvar type object\n");
		exit(1);
	}
	return (*object)[key];
}

inline std::ostream &operator<<(std::ostream &output, jsvar &var) {
	if (var.type == JS_TYPE_TEXT)
		output << var.text;
	else
		output << var.toString();

	return output;
}

inline int jsvar::getType() {
	return type;
}

inline std::string jsvar::toString(bool pretty, int indent) {
	size_t pos, start, escape;
	char escchar;
	char canum[64];
	std::string esctext;
	std::string result = "";

	switch (type) {
		case JS_TYPE_NULL:
			result = "null";
			break;
		case JS_TYPE_BOOLEAN:
			if (boolean == true)
				result = "true";
			else
				result = "false";
			break;
		case JS_TYPE_INTEGER:
			sprintf(canum, "%lld", inumber);
			result = std::string(canum);
			break;
		case JS_TYPE_FLOAT:
			if (std::isnan(fnumber)) {
				result = "NaN";
			}
			else if (std::isinf(fnumber)) {
				result = "Infinity";
			}
			else {
				sprintf(canum, "%.14lf", fnumber);
				result = std::string(canum);
				pos = result.find_last_not_of("0");
				if (pos != std::string::npos)
					result = result.substr(0, pos+1);
				if (result[result.length()-1] == '.')
					result += "0";
			}
			break;
		case JS_TYPE_TEXT:
			esctext = text;
			start = 0;
			escape = esctext.find_first_of("\"\b\r\n\f\t\v");
			while (escape != std::string::npos) {
				if (esctext[escape] == '\"')
					escchar = '\"';
				else if (esctext[escape] == '\b')
					escchar = 'b';
				else if (esctext[escape] == '\r')
					escchar = 'r';
				else if (esctext[escape] == '\n')
					escchar = 'n';
				else if (esctext[escape] == '\f')
					escchar = 'f';
				else if (esctext[escape] == '\t')
					escchar = 't';
				else if (esctext[escape] == '\v')
					escchar = 'v';
				esctext = esctext.substr(0, escape) + "\\" + escchar + esctext.substr(escape+1);
				start = escape + 2;
				escape = esctext.find_first_of("\"\b\r\n\f\t\v", start);
			}
			result += "\"" + esctext + "\"";
			break;
		case JS_TYPE_ARRAY:
			result = array->stringify(pretty, indent);
			break;
		case JS_TYPE_OBJECT:
			result = object->stringify(pretty, indent);
			break;
	}
	return result;
}

inline void jsvar::append(jsvar &value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(bool value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(char value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(short value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(int value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(long value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(long long value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(unsigned char value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(unsigned short value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(unsigned int value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(unsigned long value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(unsigned long long value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(float value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(double value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(const char *value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(std::string value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(class jsarray &value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(class jsobject &value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::append(void *value) {
	if (type == JS_TYPE_INVALID) {
		type = JS_TYPE_ARRAY;
		array = new class jsarray();
	}
	else if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->append(value);
}

inline void jsvar::remove(int index) {
	if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->remove((size_t)index);
}

inline void jsvar::remove(size_t index) {
	if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	array->remove(index);
}

inline void jsvar::remove(const char *key) {
	if (type != JS_TYPE_OBJECT) {
		fprintf(stderr, "Invalid access of jsvar type object\n");
		exit(1);
	}
	object->remove(std::string(key));
}

inline void jsvar::remove(std::string key) {
	if (type != JS_TYPE_OBJECT) {
		fprintf(stderr, "Invalid access of jsvar type object\n");
		exit(1);
	}
	object->remove(key);
}

inline size_t jsvar::length() {
	if (type != JS_TYPE_ARRAY) {
		fprintf(stderr, "Invalid access of jsvar type array\n");
		exit(1);
	}
	return array->length();
}

inline std::vector<std::string> jsvar::keys() {
	if (type != JS_TYPE_OBJECT) {
		fprintf(stderr, "Invalid access of jsvar type object\n");
		exit(1);
	}
	return object->keys();
}

inline bool jsvar::hasProperty(std::string key) {
	if (type != JS_TYPE_OBJECT) {
		fprintf(stderr, "Invalid access of jsvar type object\n");
		exit(1);
	}
	return object->hasProperty(key);
}

inline void jsvar::clear() {
	if (type == JS_TYPE_ARRAY)
		delete array;
	else if (type == JS_TYPE_OBJECT)
		delete object;

	type = JS_TYPE_INVALID;
}

inline std::string jsvar::stringify(bool pretty, int indent) {
	if (type != JS_TYPE_ARRAY && type != JS_TYPE_OBJECT) {
		fprintf(stderr, "Invalid access of jsvar type array/object\n");
		exit(1);
	}
	if (type == JS_TYPE_ARRAY)
		return array->stringify(pretty, indent);
	return object->stringify(pretty, indent);
}

inline jsvar jsvar::jsarray() {
	return jsvar(*new class jsarray());
}

inline jsvar jsvar::jsobject() {
	return jsvar(*new class jsobject());
}


/***************************************************************
 *  public methods: jsarray                                    *
 ***************************************************************/

inline jsarray::jsarray() {
}

inline jsarray::~jsarray() {
	clear();
}

inline jsvar& jsarray::operator[](int index) {
	return (*this)[(size_t)index];
}

inline jsvar& jsarray::operator[](size_t index) {
	if (index >= list.size()) {
		fprintf(stderr, "Invalid access of jsarray: index out of bounds\n");
		exit(1);
	}
	return list[index];
}

inline void jsarray::append(jsvar &value) {
	list.push_back(value);
}

inline void jsarray::append(bool value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(char value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(short value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(int value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(long value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(long long value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(unsigned char value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(unsigned short value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(unsigned int value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(unsigned long value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(unsigned long long value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(float value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(double value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(const char *value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(std::string value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(jsarray &value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(jsobject &value) {
	list.push_back(jsvar(value));
}

inline void jsarray::append(void *value) {
	list.push_back(jsvar(value));
}

inline void jsarray::remove(int index) {
	remove((size_t)index);
}

inline void jsarray::remove(size_t index) {
	if (index >= list.size()) {
		fprintf(stderr, "Invalid access of jsarray: index out of bounds\n");
		exit(1);
	}
	list.erase(list.begin()+index);
}

inline size_t jsarray::length() {
	return list.size();
}

inline void jsarray::clear() {
	int i, type;
	for (i=0; i<list.size(); i++) {
		type = list[i].getType();
		if (type == JS_TYPE_ARRAY || type == JS_TYPE_OBJECT)
			list[i].clear();
	}
}

inline std::string jsarray::stringify(bool pretty, int indent) {
	int i;
	char pad[128];
	std::string padding = "";
	std::string ipadding = "";
	std::string result = "[";

	if (pretty) {
		sprintf(pad, "%*s", 4*indent, "");
		padding = std::string(pad);
		ipadding = padding + "    ";
	}

	for (i=0; i<list.size(); i++) {
		if (pretty)
			result += "\n" + ipadding;
		result += list[i].toString(pretty, indent + 1) + ",";
	}

	if (list.size() > 0) {
		if (pretty) {
			result[result.length()-1] = '\n';
			result += padding + "]";
		}
		else {
			result[result.length()-1] = ']';
		}
	}
	else
		result += "]";

	return result;
}

inline jsvar jsarray::parse(std::string json, size_t *headPtr) {
	size_t begin = (headPtr == NULL) ? 0 : *headPtr;
	size_t head = json.find_first_not_of(" \t\r\n", begin);

	std::string comment = json.substr(head, 2);
	while (comment == "//" || comment == "/*") {
		if (comment == "//")
			head = json.find("\n", head) + 1;
		else
			head = json.find("*/", head) + 2;
		if (head == std::string::npos) {
			fprintf(stderr, "Error parsing JSON\n");
			return jsvar();
		}
		head = json.find_first_not_of(" \t\r\n", head);
		comment = json.substr(head, 2);
	}

	if (json[head] != '[') {
		fprintf(stderr, "Error parsing JSON\n");
		return jsvar();
	}

	jsarray* arrptr = new jsarray();
	head = json.find_first_not_of(" \t\r\n", head+1);

	std::string value;
	long long hnum;
	size_t pos, start, dot, hex, escape;
	while (json[head] != ']') {
		// ignore comments
		comment = json.substr(head, 2);
		while (comment == "//" || comment == "/*") {
			if (comment == "//")
				head = json.find("\n", head) + 1;
			else
				head = json.find("*/", head) + 2;
			if (head == std::string::npos) {
				fprintf(stderr, "Error parsing JSON\n");
				return jsvar();
			}
			head = json.find_first_not_of(" \t\r\n", head);
			comment = json.substr(head, 2);
		}

		// determine value type
		// null
		if (json[head] == 'n') {
			if (json.substr(head, 4) == "null") {
				arrptr->append(JS_NULL);
				head += 4;
			}
			else {
				fprintf(stderr, "Error parsing JSON\n");
				delete arrptr;
				return jsvar();
			}
		}
		// boolean
		else if (json[head] == 't' || json[head] == 'f') {
			if (json.substr(head, 4) == "true") {
				arrptr->append(true);
				head += 4;
			}
			else if (json.substr(head, 5) == "false") {
				arrptr->append(false);
				head += 5;
			}
			else {
				fprintf(stderr, "Error parsing JSON\n");
				delete arrptr;
				return jsvar();
			}
		}
		// number (infinity or nan)
		else if (json[head] == 'I' || json[head] == 'N' || json.substr(head, 2) == "-I" || json.substr(head, 2) == "-N") {
			if (json.substr(head, 8) == "Infinity") {
				arrptr->append(std::numeric_limits<double>::infinity());
				head += 8;
			}
			else if (json.substr(head, 9) == "-Infinity") {
				arrptr->append(std::numeric_limits<double>::infinity());
				head += 9;
			}
			else if (json.substr(head, 3) == "NaN") {
				arrptr->append(std::numeric_limits<double>::quiet_NaN());
				head += 3;
			}
			else if (json.substr(head, 4) == "-NaN") {
				arrptr->append(std::numeric_limits<double>::quiet_NaN());
				head += 4;
			}
			else {
				fprintf(stderr, "Error parsing JSON\n");
				delete arrptr;
				return jsvar();
			}
		}
		// number
		else if (json[head] == '-' || json[head] == '+' || json[head] == '.' || isdigit(json[head])) {
			pos = json.find_first_not_of(".x0123456789ABCDEFabcdef", head+1);
			if (pos == std::string::npos) {
				fprintf(stderr, "Error parsing JSON\n");
				delete arrptr;
				return jsvar();
			}
			value = json.substr(head, pos-head);
			hex = value.find("x");
			if (hex != std::string::npos) {
				if ((value.substr(0,2) != "0x" && value.substr(0,3) != "-0x" && value.substr(0,3) != "+0x") || value.find_first_of(".x", hex+1) != std::string::npos) {
					fprintf(stderr, "Error parsing JSON\n");
					delete arrptr;
					return jsvar();
				}
				sscanf(value.substr(hex+1).c_str(), "%llx", &hnum);
				if (value[0] == '-')
					hnum *= -1;
				arrptr->append(hnum);
			}
			else {
				if (value.find_first_of("ABCDEFabcdef") != std::string::npos) {
					fprintf(stderr, "Error parsing JSON\n");
					delete arrptr;
					return jsvar();
				}
				dot = value.find(".");
				if (dot == std::string::npos) {
					arrptr->append(atoll(value.c_str()));
				}
				else {
					if (value.find(".", dot + 1) != std::string::npos) {
						fprintf(stderr, "Error parsing JSON\n");
						delete arrptr;
						return jsvar();
					}
					arrptr->append(atof(value.c_str()));
				}
			}
			head = pos;
		}
		// text
		else if (json[head] == '\"' || json[head] == '\'') {
			start = 1;
			do {
				pos = json.find(json[head], head+start);
				if (pos == std::string::npos) {
					fprintf(stderr, "Error parsing JSON\n");
					delete arrptr;
					return jsvar();
				}
				start = pos - head + 1;
			} while(json[pos-1] == '\\');
			start = 1;
			value = json.substr(head+start, pos-(head+start));
			escape = value.find("\n");
			while (escape != std::string::npos) {
				if (escape == 0 || value[escape-1] != '\\') {
					fprintf(stderr, "Error parsing JSON\n");
					delete arrptr;
					return jsvar();
				}
				escape = value.find("\n", escape+1);
			}
			escape = value.find("\\");
			while (escape != std::string::npos) {
				value.erase(escape, 1);
				if (value[escape] == 'b')
					value[escape] = '\b';
				else if (value[escape] == 'r')
					value[escape] = '\r';
				else if (value[escape] == 'n')
					value[escape] = '\n';
				else if (value[escape] == 'f')
					value[escape] = '\f';
				else if (value[escape] == 't')
					value[escape] = '\t';
				else if (value[escape] == 'v')
					value[escape] = '\v';
				escape = value.find("\\", escape+1);
			}
			arrptr->append(value);
			head = pos + start;
		}
		// array
		else if (json[head] == '[') {
			jsvar narr = parse(json, &head);
			arrptr->append(narr);
		}
		// object
		else if (json[head] == '{') {
			jsvar nobj = jsobject::parse(json, &head);
			arrptr->append(nobj);
		}

		// ignore comments
		head = json.find_first_not_of(" \t\r\n", head);
		comment = json.substr(head, 2);
		while (comment == "//" || comment == "/*") {
			if (comment == "//")
				head = json.find("\n", head) + 1;
			else
				head = json.find("*/", head) + 2;
			if (head == std::string::npos) {
				fprintf(stderr, "Error parsing JSON\n");
				delete arrptr;
				return jsvar();
			}
			head = json.find_first_not_of(" \t\r\n", head);
			comment = json.substr(head, 2);
		}

		head = json.find_first_not_of(", \t\r\n", head);
	}

	if (headPtr != NULL)
		*headPtr = head + 1;

	return jsvar(*arrptr);
}


/***************************************************************
 *  public methods: jsobject                                   *
 ***************************************************************/

inline jsobject::jsobject() {
}

inline jsobject::~jsobject() {
	clear();
}

inline jsvar& jsobject::operator[](const char *key) {
	return (*this)[std::string(key)];
}

inline jsvar& jsobject::operator[](std::string key) {
	if (dict.count(key) <= 0)
		dict[key] = jsvar();
	return dict[key];
}

inline void jsobject::remove(const char *key) {
	remove(std::string(key));
}

inline void jsobject::remove(std::string key) {
	dict.erase(key);
}

inline std::vector<std::string> jsobject::keys() {
	std::map<std::string,jsvar>::iterator it;
	std::vector<std::string> result;

	for (it = dict.begin(); it != dict.end(); it++) {
		if (it->second.getType() != JS_TYPE_INVALID)
			result.push_back(it->first);
	}
	return result;
}

inline bool jsobject::hasProperty(std::string key) {
	if (dict.count(key) > 0)
		return true;
	return false;
}

inline void jsobject::clear() {
	std::map<std::string,jsvar>::iterator it;
	int type;
	for (it = dict.begin(); it != dict.end(); it++) {
		type = it->second.getType();
		if (type == JS_TYPE_ARRAY || type == JS_TYPE_OBJECT)
			it->second.clear();
	}
}

inline std::string jsobject::stringify(bool pretty, int indent) {
	std::map<std::string,jsvar>::iterator it;
	char pad[128];
	size_t start, escape;
	std::string esckey;
	std::string padding = "";
	std::string ipadding = "";
	std::string result = "{";

	if (pretty) {
		sprintf(pad, "%*s", 4*indent, "");
		padding = std::string(pad);
		ipadding = padding + "    ";
	}

	int count = 0;
	for (it = dict.begin(); it != dict.end(); it++) {
		if (it->second.getType() != JS_TYPE_INVALID) {
			if (pretty)
				result += "\n" + ipadding;
			esckey = it->first;
			start = 0;
			escape = esckey.find_first_of("\"\'");
			while (escape != std::string::npos) {
				esckey = esckey.substr(0, escape) + "\\" + esckey.substr(escape);
				start = escape + 2;
				escape = esckey.find_first_of("\"\'", start);
			}
			result += "\"" + esckey + "\":";
			if (pretty)
				result += " ";
			result += it->second.toString(pretty, indent + 1) + ",";
			count++;
		}
	}

	if (count > 0) {
		if (pretty) {
			result[result.length()-1] = '\n';
			result += padding + "}";
		}
		else {
			result[result.length()-1] = '}';
		}
	}
	else
		result += "}";

	return result;
}

inline jsvar jsobject::parse(std::string json, size_t *headPtr) {
	size_t begin = (headPtr == NULL) ? 0 : *headPtr;
	size_t head = json.find_first_not_of(" \t\r\n", begin);

	std::string comment = json.substr(head, 2);
	while (comment == "//" || comment == "/*") {
		if (comment == "//")
			head = json.find("\n", head) + 1;
		else
			head = json.find("*/", head) + 2;
		if (head == std::string::npos) {
			fprintf(stderr, "Error parsing JSON\n");
			return jsvar();
		}
		head = json.find_first_not_of(" \t\r\n", head);
		comment = json.substr(head, 2);
	}

	if (json[head] != '{') {
		if (json[head] == '[')
			return jsarray::parse(json, &head);
		fprintf(stderr, "Error parsing JSON\n");
		return jsvar();
	}

	jsobject* objptr = new jsobject();
	head = json.find_first_not_of(" \t\r\n", head+1);

	std::string key, value;
	long long hnum;
	size_t pos, start, dot, hex, escape;
	while (json[head] != '}') {
		// ignore comments
		comment = json.substr(head, 2);
		while (comment == "//" || comment == "/*") {
			if (comment == "//")
				head = json.find("\n", head) + 1;
			else
				head = json.find("*/", head) + 2;
			if (head == std::string::npos) {
				fprintf(stderr, "Error parsing JSON\n");
				return jsvar();
			}
			head = json.find_first_not_of(" \t\r\n", head);
			comment = json.substr(head, 2);
		}

		// get key
		if (json[head] == '\"' || json[head] == '\'') {
			start = 1;
			do {
				pos = json.find(json[head], head+start);
				if (pos == std::string::npos) {
					fprintf(stderr, "Error parsing JSON\n");
					delete objptr;
					return jsvar();
				}
				start = pos - head + 1;
			} while(json[pos-1] == '\\');
			start = 1;
		}
		else {
			start = 0;
			pos = json.find_first_not_of("_$1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", head);
		}
		key = json.substr(head+start, pos-(head+start));
		escape = key.find("\\");
		while (escape != std::string::npos) {
			if (key[escape+1] == '\"' || key[escape+1] == '\'')
				key.erase(escape, 1);
			escape = key.find("\\");
		}
		
		head = json.find_first_not_of(" \t\r\n", pos+start);
		if (json[head] != ':') {
			fprintf(stderr, "Error parsing JSON\n");
			delete objptr;
			return jsvar();
		}
		// determine value type
		head = json.find_first_not_of(" \t\r\n", head+1);
		// null
		if (json[head] == 'n') {
			if (json.substr(head, 4) == "null") {
				(*objptr)[key] = JS_NULL;
				head += 4;
			}
			else {
				fprintf(stderr, "Error parsing JSON\n");
				delete objptr;
				return jsvar();
			}
		}
		// boolean
		else if (json[head] == 't' || json[head] == 'f') {
			if (json.substr(head, 4) == "true") {
				(*objptr)[key] = true;
				head += 4;
			}
			else if (json.substr(head, 5) == "false") {
				(*objptr)[key] = false;
				head += 5;
			}
			else {
				fprintf(stderr, "Error parsing JSON\n");
				delete objptr;
				return jsvar();
			}
		}
		// number (infinity or nan)
		else if (json[head] == 'I' || json[head] == 'N' || json.substr(head, 2) == "-I" || json.substr(head, 2) == "-N") {
			if (json.substr(head, 8) == "Infinity") {
				(*objptr)[key] = std::numeric_limits<double>::infinity();
				head += 8;
			}
			else if (json.substr(head, 9) == "-Infinity") {
				(*objptr)[key] = std::numeric_limits<double>::infinity();
				head += 9;
			}
			else if (json.substr(head, 3) == "NaN") {
				(*objptr)[key] = std::numeric_limits<double>::quiet_NaN();
				head += 3;
			}
			else if (json.substr(head, 4) == "-NaN") {
				(*objptr)[key] = std::numeric_limits<double>::quiet_NaN();
				head += 4;
			}
			else {
				fprintf(stderr, "Error parsing JSON\n");
				delete objptr;
				return jsvar();
			}
		}
		// number
		else if (json[head] == '-' || json[head] == '+' || json[head] == '.' || isdigit(json[head])) {
			pos = json.find_first_not_of(".x0123456789ABCDEFabcdef", head+1);
			if (pos == std::string::npos) {
				fprintf(stderr, "Error parsing JSON\n");
				delete objptr;
				return jsvar();
			}
			value = json.substr(head, pos-head);
			hex = value.find("x");
			if (hex != std::string::npos) {
				if ((value.substr(0,2) != "0x" && value.substr(0,3) != "-0x" && value.substr(0,3) != "+0x") || value.find_first_of(".x", hex+1) != std::string::npos) {
					fprintf(stderr, "Error parsing JSON\n");
					delete objptr;
					return jsvar();
				}
				sscanf(value.substr(hex+1).c_str(), "%llx", &hnum);
				if (value[0] == '-')
					hnum *= -1;
				(*objptr)[key] = hnum;
			}
			else {
				if (value.find_first_of("ABCDEFabcdef") != std::string::npos) {
					fprintf(stderr, "Error parsing JSON\n");
					delete objptr;
					return jsvar();
				}
				dot = value.find(".");
				if (dot == std::string::npos) {
					(*objptr)[key] = atoll(value.c_str());
				}
				else {
					if (value.find(".", dot + 1) != std::string::npos) {
						fprintf(stderr, "Error parsing JSON\n");
						delete objptr;
						return jsvar();
					}
					(*objptr)[key] = atof(value.c_str());
				}
			}
			head = pos;
		}
		// text
		else if (json[head] == '\"' || json[head] == '\'') {
			start = 1;
			do {
				pos = json.find(json[head], head+start);
				if (pos == std::string::npos) {
					fprintf(stderr, "Error parsing JSON\n");
					delete objptr;
					return jsvar();
				}
				start = pos - head + 1;
			} while(json[pos-1] == '\\');
			start = 1;
			value = json.substr(head+start, pos-(head+start));
			escape = value.find("\n");
			while (escape != std::string::npos) {
				if (escape == 0 || value[escape-1] != '\\') {
					fprintf(stderr, "Error parsing JSON\n");
					delete objptr;
					return jsvar();
				}
				escape = value.find("\n", escape+1);
			}
			escape = value.find("\\");
			while (escape != std::string::npos) {
				value.erase(escape, 1);
				if (value[escape] == 'b')
					value[escape] = '\b';
				else if (value[escape] == 'r')
					value[escape] = '\r';
				else if (value[escape] == 'n')
					value[escape] = '\n';
				else if (value[escape] == 'f')
					value[escape] = '\f';
				else if (value[escape] == 't')
					value[escape] = '\t';
				else if (value[escape] == 'v')
					value[escape] = '\v';
				escape = value.find("\\", escape+1);
			}
			(*objptr)[key] = value;
			head = pos + start;
		}
		// array
		else if (json[head] == '[') {
			jsvar narr = jsarray::parse(json, &head);
			(*objptr)[key] = narr;
		}
		// object
		else if (json[head] == '{') {
			jsvar nobj = parse(json, &head);
			(*objptr)[key] = nobj;
		}

		// ignore comments
		head = json.find_first_not_of(" \t\r\n", head);
		comment = json.substr(head, 2);
		while (comment == "//" || comment == "/*") {
			if (comment == "//")
				head = json.find("\n", head) + 1;
			else
				head = json.find("*/", head) + 2;
			if (head == std::string::npos) {
				fprintf(stderr, "Error parsing JSON\n");
				delete objptr;
				return jsvar();
			}
			head = json.find_first_not_of(" \t\r\n", head);
			comment = json.substr(head, 2);
		}

		head = json.find_first_not_of(", \t\r\n", head);
	}

	if (headPtr != NULL)
		*headPtr = head + 1;

	return jsvar(*objptr);
}

inline jsvar jsobject::parseFromFile(std::string filename) {
	FILE *f = fopen(filename.c_str(), "rb");
	if (f == NULL) {
		fprintf(stderr, "Error opening file %s\n", filename.c_str());
		return jsvar();
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *text = (char*)malloc(fsize * sizeof(char));
	if (fread(text, fsize, 1, f) != 1) {
		fprintf(stderr, "Error reading file %s\n", filename.c_str());
		return jsvar();
	}
	fclose(f);

	return parse(text);
}

#endif // JSOBJECT_HPP
