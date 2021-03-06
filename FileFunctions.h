//---------------------------------------------------------------------------

#ifndef FileFunctionsH
#define FileFunctionsH
#include <System.Classes.hpp>
//---------------------------------------------------------------------------
class FileFunctions {
public:
	static String getFileDate(const char* filename);
	static String getCurrentDate();
	static String getCurrentDay();
	static String getCurrentTime();
	static size_t getFileSize(const char* filename);
	static double getDiffIntDate(const char* filename);
	static String removeSpaces(UnicodeString &input);
	static bool dirExists(const AnsiString& dirName_in);
};
#endif
