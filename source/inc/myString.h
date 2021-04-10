
#ifndef __MYSTRING_H__
#define __MYSTRING_H__

#include <cctype>
#include <iostream>
#include <algorithm> 
#include <wchar.h>
#include <ctype.h>


using std::string;
class String : public std::string
{
public:

String() : std::string()
	{
	}

String(int v) : std::string()
	{
		std::string::operator=(std::to_string(v));
	}
String(long v) : std::string()
	{
		std::string::operator=(std::to_string(v));
	}
String(long long v) : std::string()
	{
		std::string::operator=(std::to_string(v));
	}
String(unsigned v) : std::string()
	{
		std::string::operator=(std::to_string(v));
	}
String(unsigned long v) : std::string()
	{
		std::string::operator=(std::to_string(v));
	}
String(unsigned long long v) : std::string()
	{
		std::string::operator=(std::to_string(v));
	}
String(float v) : std::string()
	{
		if( (long long)v == v)
			std::string::operator=( std::to_string ( (long long)v) );
		else
			std::string::operator=( std::to_string (v) );
	}
String(double v) : std::string()
	{
		if( (long long)v == v)
			std::string::operator=( std::to_string ( (long long)v) );
		else
			std::string::operator=( std::to_string (v) );
	}
String(long double v) : std::string()
	{
		if( (long long)v == v)
			std::string::operator=( std::to_string ( (long long)v) );
		else
			std::string::operator=( std::to_string (v) );
	}
String(const char text[]) : std::string()
	{
		std::string::operator=( text );
	}	
String(char c) : std::string()
	{
		std::string::operator=( c );
	}	
String (std::string s) : std::string()
	{
		std::string::operator=( s );
	}	
// String (const String &s) : std::string()
// 	{
// 		std::string::operator=( s );
// 	}	

// String &operator=(const String& str)
// 	{
// 		std::string::operator=(str);	return *this;
// 	}
// String &operator=(String& str)
// 	{
// 		std::string::operator=(str);	return *this;
// 	}

String &operator+= (const String& str)
	{
		std::string::operator+=(str);	return *this;
	}
String &operator+= (String& str)
	{
		std::string::operator+=(str);	return *this;
	}

bool operator==(String &v)
	{
		return std::string::compare(v) == 0;
	}
bool operator!=(String &v)
	{
		return std::string::compare(v) != 0;
	}

// const char *c_str()
// {
// 	return std::string::c_str();
// }


size_type Length()
	{
		return std::string::length();
	}

size_type Pos(String v)
	{
		size_type p = std::string::find(v);
		return p == std::string::npos ? 0 : p + 1;
	}

double ToDouble()
	{
		return std::stod(*this);
	}

int ToInt()
	{
		return std::stoi(*this);
	}

int ToIntDef(int def)
	{
		// try
		// {
		// 	return std::stoi(*this);
		// }
		// catch (...)
		// {
		// 	return def;
		// }
        return std::stoi(*this);
	}
String SubString(int index, int count)
	{
		return ((*this).substr(index - 1, count)).c_str();
	}

String UpperCase()
	{
		String tmp = *this;
		for(unsigned int i = 0; i < tmp.size(); i++)
		{
	 	   tmp.at(i) = toupper(tmp.at(i));
		}
		return tmp;
	}
String LowerCase()
	{
		String tmp = *this;
		for(unsigned int i = 0; i < tmp.size(); i++)
		{
	 	   tmp.at(i) = tolower(tmp.at(i));
		}
		return tmp;
	}

String IntToHex(QWORD source_int, int digi)
	{
		static const char* digits = "0123456789ABCDEF";
		String temp_str = "";

		for(int i = 0 ; i < digi ; i++)
		{
			String t = digits[source_int & 0x0F];
			t += temp_str;
			temp_str = t;
			source_int >>= 4;
		}
	    return temp_str;
	}

String Delete(size_type pos, size_type len)
	{
		this->erase(pos - 1, len);	return *this;
	}

String Insert(String s,size_type pos)
	{
		this->insert(pos - 1, s);
		return *this;
	}

String SetLength(size_type len)
	{
		this->resize(len);
		return *this;
	}


// trim from start (in place)
static inline void LTrim(std::string &s) 
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));	
	}

// trim from end (in place)
static inline void RTrim(std::string &s) 
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

String Trim()
	{		
		LTrim( *this );
		RTrim( *this );
		return *this;
	}

unsigned int HextoInt()	//HEX STRING TO U INT
	{
		return strtoul(this->c_str(), NULL, 16);
	}


~String()
	{
	}
};




#endif

