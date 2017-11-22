//---------------------------------------------------------------------------

#pragma hdrstop

#include "FileFunctions.h"
#include <sys/stat.h>
#include <time.h>
#pragma package(smart_init)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/**
 * getFileDate - Pobiera datê pliku
 * @param const char* filename - œcie¿ka do pliku
 * @return String - Zwraca datê pliku lub -1 jeœli plik nie istnieje.
 */
String FileFunctions::getFileDate(const char* filename) {
	struct stat st;
	if(stat(filename, &st) != 0) {
		return -1;
	}

	//konwertuje zmienn¹ st_mtime na zrozumia³¹ datê
	time_t t = st.st_mtime;
	struct tm *info;
	info = localtime(&t);
	char buffer[80];

	//wyœwietla czas w zadanej konfiguracji
	strftime(buffer,sizeof(buffer),"%d.%m.%Y %H:%M:%S", info);
	//konwertuje char buffer na string
	String str(buffer);

	return str;
}
//---------------------------------------------------------------------------
/**
 * getCurrentDate - Wyœwietla aktualny czas (dzieñ.miesi¹c.rok godzina:minuta:sekunda)
 * @return String - Zwraca aktualn¹ datê.
 */
String FileFunctions::getCurrentDate(){
	time_t currentTime;
	struct tm *tmTime;
	char buffer[80];

	time(&currentTime);

	tmTime = localtime(&currentTime);

	//wyœwietla czas w zadanej konfiguracji
	strftime(buffer,80,"%d.%m.%Y %H:%M:%S", tmTime);

	String str(buffer);
	return str;
}
//---------------------------------------------------------------------------
/**
 * getCurrentDay - Wyœwietla aktualny czas (dzieñ.miesi¹c.rok)
 * @return String - Zwraca aktualn¹ datê.
 */
String FileFunctions::getCurrentDay(){
	time_t currentTime;
	struct tm *tmTime;
	char buffer[80];

	time(&currentTime);

	tmTime = localtime(&currentTime);

	//wyœwietla czas w zadanej konfiguracji
	strftime(buffer,80,"%d.%m.%Y", tmTime);

	String str(buffer);
	return str;
}
//---------------------------------------------------------------------------
/**
 * getCurrentTime - Wyœwietla aktualny czas (godzina:minuta:sekunda)
 * @return String - Zwraca aktualn¹ datê.
 */
String FileFunctions::getCurrentTime(){
	time_t currentTime;
	struct tm *tmTime;
	char buffer[80];

	time(&currentTime);

	tmTime = localtime(&currentTime);

	//wyœwietla czas w zadanej konfiguracji
	strftime(buffer,80,"%H:%M:%S", tmTime);

	String str(buffer);
	return str;
}
//---------------------------------------------------------------------------
/**
 * getFileSize - Pobiera rozmiar pliku
 * @param const char* filename - œcie¿ka do pliku
 * @return - Zwraca rozmiar pliku lub -1 jeœli plik nie istnieje.
 */
size_t FileFunctions::getFileSize(const char* filename) {
	struct stat st;
	if(stat(filename, &st) != 0) {
		return -1;
	}
	return st.st_size;
}
//---------------------------------------------------------------------------
/**
 * getDiffIntDate - Ró¿nica pomiêdzy dat¹ utwo¿enia pliku, a aktualnym czasem (ró¿nica jako double wyra¿ona w sekundach)
 * @param const char* filename - œcie¿ka do pliku
 * @return String - Zwraca ró¿nicê pomiêdzy dat¹ pliku, a aktualnym czasem
 */
double FileFunctions::getDiffIntDate(const char* filename) {
	struct stat st;
	if(stat(filename, &st) != 0) {
		return -1;
	}

	//aktualna data
	time_t currentTime;
	struct tm *tmTime;

	time(&currentTime);
	//ró¿nica pomiêdzy aktualn¹ dat¹, a dat¹ pliku w sekundach
	double diff = currentTime - st.st_mtime;

	return diff;
}
//---------------------------------------------------------------------------
/**
 * removeSpaces - Funkcja usuwa spacje ze stringa
 * @param UnicodeString input - ³añcuch string ze spacjami do usuniêcia
 * @return UnicodeString input - ³añcuch string bez spacji
 */
String FileFunctions::removeSpaces(UnicodeString &input){
	for (int i = 1; i <= input.Length(); i++) {
		if(input.SubString(i,1) == " ") {
			input.Delete(i, 1);
			i--;
		}
	}
	return input;
}
//---------------------------------------------------------------------------
/*
 * dirExists - Funkcja sprawdza, czy dir istnieje
 * @param String - œcie¿ka do sprawdzenia
 * @return bool - true jeœli œcie¿ka istnieje, false jeœli nie
 */
bool FileFunctions::dirExists(const AnsiString& dirName_in){
  DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
	return false;  //coœ nie tak z path

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	return true;   // dir istnieje

  return false;    // dir nie istnieje lub œcie¿ka to nie dir
}

