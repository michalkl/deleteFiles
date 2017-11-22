//---------------------------------------------------------------------------

#pragma hdrstop

#include "FileFunctions.h"
#include <sys/stat.h>
#include <time.h>
#pragma package(smart_init)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/**
 * getFileDate - Pobiera dat� pliku
 * @param const char* filename - �cie�ka do pliku
 * @return String - Zwraca dat� pliku lub -1 je�li plik nie istnieje.
 */
String FileFunctions::getFileDate(const char* filename) {
	struct stat st;
	if(stat(filename, &st) != 0) {
		return -1;
	}

	//konwertuje zmienn� st_mtime na zrozumia�� dat�
	time_t t = st.st_mtime;
	struct tm *info;
	info = localtime(&t);
	char buffer[80];

	//wy�wietla czas w zadanej konfiguracji
	strftime(buffer,sizeof(buffer),"%d.%m.%Y %H:%M:%S", info);
	//konwertuje char buffer na string
	String str(buffer);

	return str;
}
//---------------------------------------------------------------------------
/**
 * getCurrentDate - Wy�wietla aktualny czas (dzie�.miesi�c.rok godzina:minuta:sekunda)
 * @return String - Zwraca aktualn� dat�.
 */
String FileFunctions::getCurrentDate(){
	time_t currentTime;
	struct tm *tmTime;
	char buffer[80];

	time(&currentTime);

	tmTime = localtime(&currentTime);

	//wy�wietla czas w zadanej konfiguracji
	strftime(buffer,80,"%d.%m.%Y %H:%M:%S", tmTime);

	String str(buffer);
	return str;
}
//---------------------------------------------------------------------------
/**
 * getCurrentDay - Wy�wietla aktualny czas (dzie�.miesi�c.rok)
 * @return String - Zwraca aktualn� dat�.
 */
String FileFunctions::getCurrentDay(){
	time_t currentTime;
	struct tm *tmTime;
	char buffer[80];

	time(&currentTime);

	tmTime = localtime(&currentTime);

	//wy�wietla czas w zadanej konfiguracji
	strftime(buffer,80,"%d.%m.%Y", tmTime);

	String str(buffer);
	return str;
}
//---------------------------------------------------------------------------
/**
 * getCurrentTime - Wy�wietla aktualny czas (godzina:minuta:sekunda)
 * @return String - Zwraca aktualn� dat�.
 */
String FileFunctions::getCurrentTime(){
	time_t currentTime;
	struct tm *tmTime;
	char buffer[80];

	time(&currentTime);

	tmTime = localtime(&currentTime);

	//wy�wietla czas w zadanej konfiguracji
	strftime(buffer,80,"%H:%M:%S", tmTime);

	String str(buffer);
	return str;
}
//---------------------------------------------------------------------------
/**
 * getFileSize - Pobiera rozmiar pliku
 * @param const char* filename - �cie�ka do pliku
 * @return - Zwraca rozmiar pliku lub -1 je�li plik nie istnieje.
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
 * getDiffIntDate - R�nica pomi�dzy dat� utwo�enia pliku, a aktualnym czasem (r�nica jako double wyra�ona w sekundach)
 * @param const char* filename - �cie�ka do pliku
 * @return String - Zwraca r�nic� pomi�dzy dat� pliku, a aktualnym czasem
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
	//r�nica pomi�dzy aktualn� dat�, a dat� pliku w sekundach
	double diff = currentTime - st.st_mtime;

	return diff;
}
//---------------------------------------------------------------------------
/**
 * removeSpaces - Funkcja usuwa spacje ze stringa
 * @param UnicodeString input - �a�cuch string ze spacjami do usuni�cia
 * @return UnicodeString input - �a�cuch string bez spacji
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
 * @param String - �cie�ka do sprawdzenia
 * @return bool - true je�li �cie�ka istnieje, false je�li nie
 */
bool FileFunctions::dirExists(const AnsiString& dirName_in){
  DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
	return false;  //co� nie tak z path

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	return true;   // dir istnieje

  return false;    // dir nie istnieje lub �cie�ka to nie dir
}

