

#ifndef __include_h__
#define __include_h__


#include "typedef.h"
#include <time.h>
#include <queue>
#include <string>
#include <sstream>
#include "stdio.h"
#include <stdlib.h>//"stdlib.h"

#include "string.h"
#include <sys/stat.h>


#include "myString.h"

#define SHOWMESSAGE(msg)		{ String s; s = msg; printf( "%s\r\n" , s.c_str() );}
#define delay( ms )             { usleep( ms * 1000 ) ;}






#endif
