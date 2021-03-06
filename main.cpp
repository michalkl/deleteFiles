//---------------------------------------------------------------------------

#include <vcl.h>
#include <FileCtrl.hpp>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <direct.h>
#include <inifiles.hpp>
#include <fstream.h>
#include <string>
#include <vector>

#include "Main.h"
#include "FileFunctions.h"
#include "SortMap.h"
#include <boost/scoped_ptr.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainWindow *MainWindow;
//---------------------------------------------------------------------------
__fastcall TMainWindow::TMainWindow(TComponent* Owner)
	: TForm(Owner){
	//rysuje list�
	createListView();
	//ustawia warto�ci domy�lne comboBox�w itp.
	cbbAge->ItemIndex = 4;
	cbbSize->ItemIndex = 3;
	cbbMaxDirSize->ItemIndex = 3;
	cbbChosenDirSize->ItemIndex = 2;
	cbbRepeatTime->ItemIndex = 4;
	edtRepeatTime->Text = "1";
	tmrAutoDelete->Enabled = false;
	//wczytanie automatyczne ustawie� po starcie programu
	loadConfigFromIni();
}
//---------------------------------------------------------------------------
/**
 * deleteExceptions - Funkcja zawiera wszystkie wyj�tki jakich nie ma usuwa� program
 * @param const String path - sprawdzana �cie�ka
 * @return bool exception - true jesli trafiono na wyj�tek, false - je�li nie
 */
bool TMainWindow::deleteExceptions(const String &path){
	bool exception = false;
	const int size = 5;
	int exists[size];

	for (UINT i = 0; i < size - 1; i++) {
		exists[i] = 0;
	}

	if (path.Length() < 2) {
		ShowMessage("Path is too short!");
		return exception = true;
	}
	if (path == "C:\\") {
		ShowMessage("Can't delete C:\\ path!");
		return exception = true;
	}
	if (path == ExtractFilePath(Application->ExeName)) {
		ShowMessage("Can't delete this path, program files!");
		return exception = true;
	}
	AnsiString pathAnsi = path;

	exists[0] = strstr(pathAnsi.c_str(), "\\Program Files") != NULL;
	exists[1] = strstr(pathAnsi.c_str(), "\\windows") != NULL;
	exists[2] = strstr(pathAnsi.c_str(), "\\Win32") != NULL;
//	exists[3] = strstr(pathAnsi.c_str(), "\\deleteFiles") != NULL;

	for (UINT i = 0; i < size - 1; i++) {
		if (exists[i]) {
			ShowMessage("Can't delete this path!");
			exception = true;
			return exception;
		}
	}

	return exception;
}
//---------------------------------------------------------------------------
/**
 * addElementToListView - Funkcja dodaje element do ListView
 * @param String path - �cie�ka do folderu podanego przez u�ytkownika
 * @param UINT iterator - zawiera informacj�, kt�r� �cie�k� z wektora vecOfPaths aktualnie obs�ugujemy
 */
void TMainWindow::addElementToListView(const String &path, const TSearchRec &sr, UINT &iterator){

	//potrzebne do wy�wietlania ListView
	TListItem *ListIt =  NULL;

	//filname sk�ada si� z aktualnej �cie�ki oraz aktualnego pliku
	AnsiString fileName = path + sr.Name;

	//konwersja String na const char * dla funkcji getFilesize i getFileDate itp.
	const char * fileNameChar = fileName.c_str();
	//usuwa okre�lon� liczb� liter ze stringa (ca�� �cie�k�, opr�cz nazwy pliku)
	AnsiString fileNameDeleted = fileName;
	//pobranie d�ugo�ci ka�dej ze �cie�ek i usuwanie jej d�ugo�ci z nazwy wy�wietlanego pliku
	fileNameDeleted = fileNameDeleted.Delete(1, vecOfPaths[iterator].Length());

	ListIt = lvFileListView->Items->Add();
	//dodaje nazw� pliku do ListView
	ListIt->Caption = fileNameDeleted;
	//wpisuje do ListView rozmiar danego pliku
	ListIt->SubItems->Add(sr.Size);
	//wpisuje do ListView date danego pliku
	ListIt->SubItems->Add(FileDateToDateTime(sr.Time));
	//zwraca r�nic� pomi�dzy aktualn� dat�, a dat� pliku w postaci dni
	double diff2Days = FileFunctions::getDiffIntDate(fileNameChar);
	diff2Days = ceil(diff2Days/86400.0);
	ListIt->SubItems->Add(diff2Days);
}
//---------------------------------------------------------------------------
/**
 * listCatalogs - Funkcja wype�niaj�ca ListView plikami znajduj�cymi si� w katalogu
 * wybranym przez u�ytkownika
 * @param String path - �cie�ka do folderu podanego przez u�ytkownika
 * @param UINT iterator - zawiera informacj�, kt�r� �cie�k� z wektora vecOfPaths aktualnie obs�ugujemy
 */
void TMainWindow::listCatalogs(const String &path, UINT &iterator){
	TSearchRec sr;
	int result = 0, resultInCatalog = 0;
	String pathInCatalog;

	//FindFirst searches the directory specified by path for the first file that
	//matches the file name implied by path and the attributes specified by the
	//Attr parameter. The result is returned in the sr parameter. FindFirst returns
	//0 if a file was successfully located, otherwise, it returns an error code.
	// faAnyFile - faDirectory - wybiera ka�dy plik oprocz folderow

	//Przeszukuje najpierw wszystkie pliki opr�cz folder�w i wypisuje
	if (FindFirst(path + "*.*", faAnyFile - faDirectory, sr) == 0) {
		while (result == 0){
			if ((sr.Name != ".") && (sr.Name != "..")){
				//funkcja wype�nia ListView
				addElementToListView(path, sr, iterator);
				//pozwala na dokonywanie zmian w trakcie dzia�ania p�tli
				Application->ProcessMessages();
			}
			result = FindNext(sr);
		}
	}
	FindClose(sr);
	//Przeszukuje wszystkie pliki, wybiera foldery, wchodzi do srodka i ponownie przeszukuje
	if (FindFirst(path + "*.*", faDirectory + faHidden, sr) == 0){
		while (resultInCatalog == 0){
			if ((sr.Name != ".") && (sr.Name != "..") && (sr.Attr & faDirectory)) {
				//dodaje do �cie�ki aktualny folder
				pathInCatalog = path + sr.Name + "\\";
				//funkcja wype�nia ListView
				addElementToListView(path, sr, iterator);
				//wywo�uje ponownie przeszukiwanie dla danego folderu
				if(chkViewFiles->Checked) listCatalogs(pathInCatalog, iterator);
			}
			//pozwala na dokonywanie zmian w trakcie dzia�ania p�tli
			Application->ProcessMessages();
			resultInCatalog = FindNext(sr);
		}
	}
	//zamyka przeszukiwanie folderu
	FindClose(sr);

	//wy�wietla list�
	lvFileListView->ViewStyle = vsReport;
}
//---------------------------------------------------------------------------
/**
 * fillMaps - Funkcja dodaje elementy do mapy
 * @param String path - �cie�ka do folderu podanego przez u�ytkownika
 * @param UINT it - zawiera informacj�, kt�r� �cie�k� z wektora vecOfPaths aktualnie obs�ugujemy
 */
void TMainWindow::fillMaps(const String &path, UINT &it){
	TSearchRec sr;
	int result = 0, resultInCatalog = 0;
	String pathInCatalog;
	AnsiString fileName;
	double sizeOfFile = 0;

	//FindFirst searches the directory specified by path for the first file that
	//matches the file name implied by path and the attributes specified by the
	//Attr parameter. The result is returned in the sr parameter. FindFirst returns
	//0 if a file was successfully located, otherwise, it returns an error code.
	// faAnyFile - faDirectory - wybiera ka�dy plik oprocz folderow

	//Przeszukuje najpierw wszystkie pliki opr�cz folder�w i wpisuje do mapy
	if (FindFirst(path + "*.*", faAnyFile - faDirectory, sr) == 0) {
		while (result == 0){
			if ((sr.Name != ".") && (sr.Name != "..")){
				//filname sk�ada si� z aktualnej �cie�ki oraz aktualnego pliku
				fileName = path + sr.Name;
				//dodanie elementu do konkretnej mapy zale�nej od iteratora
				vecOfFilesMaps[it].addElementToMap(fileName, sr);
				//zwi�ksza licznik plik�w
				numberOfFiles++;
				//pozwala na dokonywanie zmian w trakcie dzia�ania p�tli
				Application->ProcessMessages();
			}
			result = FindNext(sr);
		}
	}
	FindClose(sr);

	//Przeszukuje wszystkie pliki, wybiera foldery, wchodzi do srodka i ponownie przeszukuje
	if (FindFirst(path + "*.*", faDirectory + faHidden, sr) == 0){
		while (resultInCatalog == 0){
			if ((sr.Name != ".") && (sr.Name != "..") && (sr.Attr & faDirectory)) {
				//dodaje do �cie�ki aktualny folder
				pathInCatalog = path + sr.Name + "\\";
				//dodanie elementu do mapy
				vecOfFoldersMaps[it].addElementToMap(pathInCatalog, sr);
				//wywo�uje ponownie przeszukiwanie dla danego folderu
				fillMaps(pathInCatalog, it);
				//pozwala na dokonywanie zmian w trakcie dzia�ania p�tli
				Application->ProcessMessages();
			}
			resultInCatalog = FindNext(sr);
		}
	}
	//zamyka przeszukiwanie folderu
	FindClose(sr);
}
//---------------------------------------------------------------------------
/**
 * createListView - Funkcja rysuje listView
 */
void TMainWindow::createListView() {
	TListColumn *ListCol = NULL;

	//Dodaje kolumn� do lvFileListView
	ListCol = lvFileListView->Columns->Add();
	ListCol->Caption = "File name";
	ListCol->Width = 300;
	ListCol->Alignment = taRightJustify;

	//Dodaje kolumn� do lvFileListView
	ListCol = lvFileListView->Columns->Add();
	ListCol->Caption = "Size (B)";
	ListCol->Width = 70;
	ListCol->Alignment = taRightJustify;

	//Dodaje kolumn� do lvFileListView
	ListCol = lvFileListView->Columns->Add();
	ListCol->Caption = "Modification date";
	ListCol->Width = 140;
	ListCol->Alignment = taRightJustify;

	//Dodaje kolumn� do lvFileListView
	ListCol = lvFileListView->Columns->Add();
	ListCol->Caption = "Age of a file (Days)";
	ListCol->Width = 110;
	ListCol->Alignment = taRightJustify;

	//Dodaje kolumn� do lvFileListView
	ListCol = lvFileListView->Columns->Add();
	ListCol->Caption = " ";
	ListCol->Width = 5;
}
//---------------------------------------------------------------------------
/**
 * btnAutoDeleteClick - Funkcja na klikni�cie przycisku Delete/Change
 */
void __fastcall TMainWindow::btnAutoDeleteClick(TObject *Sender){
	//Sprawdza, czy podano liczb� czy inny znak w edtRepeatTime
	if (chkRepeat->Checked && getUserRepeatTime() == -1) {
		ShowMessage("Enter valid repead time!");
		return;
	}
	else if (chkRepeat->Checked){
		//ustawienie interwa�u timera do automatycznego usuwania
		tmrAutoDelete->Interval = 1000 * getUserRepeatTime();
	}
	//sprawdza, czy zaznaczono jak�kolwiek opcj� w Category of files to delete
	if (chkImageDelete->Checked == false && chkTxtDelete->Checked == false &&
		chkLogDelete->Checked == false && chkOwnFileExtension->Checked == false &&
		chkAllFilesDelete->Checked == false){
		ShowMessage("Select a category of files to delete!");
		return;
	}
	//kliknieto AutoDelete - zabrania zmieni� warto�ci w danych kom�rakch
	if (buttonChangeFlag == false) {
		//pobiera warto�� userAge podan� przez u�ytkowanika je�li wybrano t� opcj�
		//oraz sprawdza, czy podano w�a�ciw� warto��, je�li nie (userAge = -1) to wychodzi
		if (chkMaxAge->Checked) {
			double userAge = getUserAge();
			//Sprawdzenie, czy podano liczb� czy inny znak w edtUserAge
			if (userAge == -1) return;
		}
		//pobiera warto�� userSize podan� przez u�ytkowanika je�li wybrano t� opcj�
		//oraz sprawdza, czy podano w�a�ciw� warto��, je�li nie (userSize = -1) to wychodzi
		if (chkMaxSize->Checked){
			double userSize = getUserSize();
			//Sprawdzenie, czy podano liczb� czy inny znak w edtUserSize
			if (userSize == -1) return;
		}

		//ustawienie koloru i napisu kontrolki edtProgramState
		programState(" BUSY", clRed);

		//ustawia flag� wci�ni�cia przycisku
		buttonChangeFlag = true;

		//wpisuje rozszerzenia z edtFileExtension do vecOfExtensions
		extensionApply(buttonChangeFlag);

		//od�wie�a komponenty w oknie programu
		refreshComponents();

		//usuwanie plik�w
		deleteFilesFunction();

		//od�wie�enie listy plik�w
		refreshListOfFiles();

		//uzupe�nienie edtChosenDirSize wielko�ci� aktualnie zaznaczonej �cie�ki
		cbbSelectedPathsToDeleteChange(NULL);

		//ustawienie koloru i napisu kontrolki edtProgramState
		programState(" FREE", clGreen);

		//je�li zaznaczona opcj� Repeat to w��cz timer
		if(chkRepeat->Checked){
			tmrAutoDelete->Enabled = true;
		}
	}
	//klikni�to Change zezwala zmieni� warto�ci w danych kom�rakch
	else{
		//ustawia flag� wci�ni�cia przycisku
		buttonChangeFlag = false;
		//wy��czenie timera do automatyczengo usuwania
		tmrAutoDelete->Enabled = false;
		//wpisuje rozszerzenia z edtFileExtension do vecOfExtensions
		extensionApply(buttonChangeFlag);
		//od�wie�a komponenty w oknie programu
		refreshComponents();
	}
}
//---------------------------------------------------------------------------
/**
 * deleteFilesFunction - zbi�r funkcji do usuwania plik�w
 */
void TMainWindow::deleteFilesFunction(){
	//wyczyszczenie map i wyzerowanie licznika dirSize
	clearMaps();

	//funkcje do usuwania plik�w z danej lokalizacji
	if (chkMaxDirSize->Checked == false) {
		//usuni�cie plik�w ze wszystkich �cie�ek po kolei
		for (UINT i = 0; i < vecOfPaths.size(); i++) {
			//utworzenie pustych obiekt�w SortMap aby wpisa� do nich p�niej mapy
			vecOfFilesMaps.push_back(SortMap());
			vecOfFoldersMaps.push_back(SortMap());
			//dodanie plik�w i folder�w do map
			fillMaps(vecOfPaths[i], i);
			//usuwa wg rozmiar/wiek plik�w
			deleteFiles(vecOfPaths[i], i);
			//dodanie rozmiar�w dir do wektora vecOfDirSizes
			vecOfDirSizes[i] = vecOfFilesMaps[i].getDirSize();
			//dodanie rozmiar�w dir do cbbSizeOfDir
			cbbSizeOfDir->Items->Add(vecOfDirSizes[i]);
		}
	}else{
		//usuwa ze wszystkich �cie�ek po kolei
		for (UINT i = 0; i < vecOfPaths.size(); i++) {
            //utworzenie pustych obiekt�w SortMap aby wpisa� do nich p�niej mapy
			vecOfFilesMaps.push_back(SortMap());
			vecOfFoldersMaps.push_back(SortMap());
			//dodanie plik�w i folder�w do map
			fillMaps(vecOfPaths[i], i);
			//dodanie aktualnej wielko�ci do wektora vecOfDirSizes
			vecOfDirSizes.push_back(vecOfFilesMaps[i].getDirSize());
			//usuwa najstarsze pliki aby utrzyma� sta�y rozmiar dir
			deleteFilesByDirSize(vecOfPaths[i], i);
		}
	}
}
//---------------------------------------------------------------------------
/**
 * clearMaps - Funkcja czy�ci mapy plik�w i zeruje licznik rozmiaru dir
 */
void TMainWindow::clearMaps(){
	//usuwa zawarto�� map i zeruje licznik dirSize
	for (UINT i = 0; i < vecOfPaths.size(); i++) {
		//wyzerowanie licznika dirSize
		vecOfFilesMaps[i].clearDirSize();
	}

	//wyczyszczenie map plik�w i folder�w
	vecOfFilesMaps.clear();
	vecOfFoldersMaps.clear();
	vecOfDirSizes.clear();
}
//---------------------------------------------------------------------------
/**
 * chkMaxAgeClick - Funkcja klikni�cia checkBoxa
 */
void __fastcall TMainWindow::chkMaxAgeClick(TObject *Sender){
	//od�wie�a komponenty cbb, edt, chk itd.
	refreshComponents();
}
//---------------------------------------------------------------------------
/**
 * chkMaxSizeClick - Funkcja klikni�cia checkBoxa
 */
void __fastcall TMainWindow::chkMaxSizeClick(TObject *Sender){
	//od�wie�a komponenty cbb, edt, chk itd.
	refreshComponents();
}
//---------------------------------------------------------------------------
/**
 * chkRepeatClick - Funkcja klikni�cia checkBoxa
 */
void __fastcall TMainWindow::chkRepeatClick(TObject *Sender)
{
	//od�wie�a komponenty cbb, edt, chk itd.
	refreshComponents();

	//zmienia napis na przycisku btnAutoDelete w zale�no�ci od opcji
	if(chkRepeat->Checked){
		btnAutoDelete->Caption = "Auto Delete";
	}
	else{
		btnAutoDelete->Caption = "Delete";
	}
}
//---------------------------------------------------------------------------
/**
 * deleteCheck - Funkcja sprawdza czy plik nale�y usun�� czy nie
 * @param const char *fileNameChar - �cie�ka do pliku sprawdzanego
 * @return bool decision - true jesli nale�y usun�� plik, false - je�li zachowa�
 */
bool TMainWindow::deleteCheck(const char * fileNameChar){
	double sizeOfFile = 0, ageOfFile = 0, userSize = 0, userAge = 0;
	bool decision;

	//je�li nie jest zaznaczona opcja usuwanie wg wielko�ci dir
	if (chkMaxDirSize->Checked == false) {
		//pobiera r�nic� pomi�dzy aktualn� dat�, a dat� pliku w postaci dni
		ageOfFile = FileFunctions::getDiffIntDate(fileNameChar);
		//pobiera warto�� userAge podan� przez u�ytkowanika je�li wybrano t� opcj�
		if (chkMaxAge->Checked) {
			userAge = getUserAge();
		}
		//pobiera rozmiar danego pliku
		sizeOfFile = FileFunctions::getFileSize(fileNameChar);

		//pobiera warto�� userSize podan� przez u�ytkowanika je�li wybrano t� opcj�
		if (chkMaxSize->Checked){
			userSize = getUserSize();
		}

		//je�li wiek pliku jest wi�kszy ni� wiek podany przez u�ytkowanika - usuwa plik
		if (chkMaxAge->Checked && ageOfFile > userAge && !(chkMaxSize->Checked)) {
			decision = true;
			reasonLog = "(zbyt du�y wiek pliku)";
		}
		//je�li wielko�� pliku jest wi�ksza ni� warto�� u�ytkowanika - usuwa plik
		else if (chkMaxSize->Checked && sizeOfFile >= userSize && !(chkMaxAge->Checked)) {
			decision = true;
			reasonLog = "(zbyt du�y rozmiar pliku)";
		}
		//je�li obie opcje s� zaznaczone (size i age)
		else if(chkMaxAge->Checked && chkMaxSize->Checked && ageOfFile > userAge && sizeOfFile > userSize){
			decision = true;
			reasonLog = "(zbyt du�y rozmiar i wiek pliku)";
		}
		else{
			decision = false;
		}
	}
	//je�li jest zaznaczona opcja usuwanie wg wielko�ci dir
	if (chkMaxDirSize->Checked) {
		//pobiera r�nic� pomi�dzy aktualn� dat�, a dat� pliku w postaci dni
		ageOfFile = FileFunctions::getDiffIntDate(fileNameChar);
    }
	//sprawdzanie rozszerze� itp.
	AnsiString fileNameAnsi = fileNameChar;

	if (decision) {
		//pobranie rozszerzenia pliku (strchr - Funkcja zwraca wska�nik na pierwsze
		//znalezione wyst�pienia znaku w �a�cuchu znak�w, kt�ry zosta� przekazany jako argument.)
		char * fileExtension = strchr(fileNameAnsi.c_str(), '.');

		//rozszerzenie aktualnego pliku w postaci np. ".jpg"
		String fileExt = fileExtension;

		//sprawdzanie dla obraz�w
		if (chkImageDelete->Checked) {
			//rozszerzenie aktualnego pliku w postaci np. ".jpg"

			if (fileExt == ".bmp" || fileExt == ".dib" || fileExt == ".jpg" ||
				fileExt == ".png" || fileExt == ".jpe" || fileExt == ".jfif" ||
				fileExt == ".gif" || fileExt == ".tif" || fileExt == ".jpeg") {

				return decision = true;
			}else{
				decision = false;
			}
		}
		//sprawdzanie dla log�w
		if (chkLogDelete->Checked) {
			//rozszerzenie aktualnego pliku w postaci np. ".jpg"

			if (fileExt == ".log") {
				return decision = true;
			}else{
				decision = false;
			}
		}
		//sprawdzanie dla txt
		if (chkTxtDelete->Checked) {
			//rozszerzenie aktualnego pliku w postaci np. ".jpg"

			if (fileExt == ".txt" || fileExt == ".xml") {
				return decision = true;
			}else{
				decision = false;
			}
		}
		//sprawdzanie dla w�asnego dodanego rozszerzenia
		if(chkOwnFileExtension->Checked){
			//rozszerzenie aktualnego pliku w postaci np. ".jpg"

			//por�wnanie ze wszystkimi rozszerzeniami dodanymi przez usera
			for (UINT i = 0; i < vecOfExtensions.size(); i++){
				if (fileExt == "." + vecOfExtensions[i]) {
					return decision = true;
				}
			}                   
			decision = false;
		}
	}
	return decision;
}
//---------------------------------------------------------------------------
/**
 * deleteFiles - Funkcja usuwaj�ca z katalogu "path" wybrane pliki
 * @param String path - �cie�ka do folderu podanego przez u�ytkownika
 * @param UINT it - zawiera informacj�, kt�r� �cie�k� z wektora vecOfPaths aktualnie obs�ugujemy
 */
void TMainWindow::deleteFiles(const String &path, UINT &it){
	TSearchRec sr;
	int result = 0;
	int resultInCatalog = 0;
	String pathInCatalog, pathOfFolder;
	AnsiString fileName, pathAnsiString, newPath;
	const char * fileNameChar;
	const char * pathChar;
	double fileCounter = 0;
	bool deletedFileFlag = false;
	bool isThereAnotherCatalog = false;

	//FindFirst searches the directory specified by path for the first file that
	//matches the file name implied by path and the attributes specified by the
	//Attr parameter. The result is returned in the sr parameter. FindFirst returns
	//0 if a file was successfully located, otherwise, it returns an error code.
	// faAnyFile - wybiera ka�dy plik

	//Przeszukuje najpierw wszystkie pliki opr�cz folder�w i wypisuje w ListView
	if (FindFirst(path + "*.*", faAnyFile - faDirectory, sr) == 0) {
		while (result == 0){
			if ((sr.Name != ".") && (sr.Name != "..")){
				//filname sk�ada si� z aktualnej �cie�ki oraz aktualnego pliku
				fileName = path + sr.Name;
				//konwersja String na const char * dla funkcji getFilesize i getFileDate itp.
				fileNameChar = fileName.c_str();
				//sprawdza czy dany plik spe�nia warunki usuni�cia
				if (deleteCheck(fileNameChar)) {
					//sprawdza, czy uda�o si� usun�� plik
					if (DeleteFile(fileName) == 0){
						//wy�wielta komunikat b��du usuni�cia pliku w logach
						String fName = fileName;
						deleteError(fName);
					}else{
                        //je�li istnieje jaki� plik w mapie
						if (vecOfFilesMaps[it].filesMap.size() > 0) {
							//usuni�cie elementu z mapy
							vecOfFilesMaps[it].deleteElementFromMap(vecOfFilesMaps[it].filesMap.find(sr.Time));
						}else{
							//je�li nie ma w mapie element�w
							ShowMessage("Delete Failed - cannot delete file. Check log for failed file info.");
							return;
						}
					}

					deletedFileFlag = true;
					//licznik licz�cy ile plik�w pozosta�o w aktualnym folderze
					fileCounter--;
					fileCounterLog++;
				}
				fileCounter++;
			}
			//pozwala na dokonywanie zmian w trakcie dzia�ania p�tli
			Application->ProcessMessages();
			result = FindNext(sr);
		}
		//zapisanie log�w do pliku
		saveLogToTxt(path);
		fileCounterLog = 0;
	}
	FindClose(sr);
	//Przeszukuje wszystkie pliki i wybiera foldery
	if (FindFirst(path + "*.*", faDirectory + faHidden, sr) == 0){
		while (resultInCatalog == 0){
			if ((sr.Name != ".") && (sr.Name != "..") && (sr.Attr & faDirectory)) {
				//dodaje do �cie�ki aktualny folder
				pathInCatalog = path + sr.Name + "\\";
				//wywo�uje ponowne przeszukiwanie dla danego folderu
				deleteFiles(pathInCatalog, it);
				//flaga mowi o tym czy jest kolejny katalog wewnatrz aktualnego
				isThereAnotherCatalog = true;
			}
			//pozwala na dokonywanie zmian w trakcie dzia�ania p�tli
			Application->ProcessMessages();
			//szuka nastepnego pliku spe�niaj�cego warunek
			resultInCatalog = FindNext(sr);
		}
	}
	//zamyka przeszukiwanie folderu
	FindClose(sr);
	//usuwanie pustego folderu jesli usuni�to z niego jakiekolwiek pliki
	//i zaznaczona opcja deleteEmptyCatalogs
	if (deletedFileFlag && fileCounter == 0 && chkDeleteEmptiedCatalogs->Checked && isThereAnotherCatalog == false) {
		//konwersja String na const char * dla funkcji RemoveDirectory
		pathAnsiString = path;
		pathChar = pathAnsiString.c_str();

		//funkcja checkIfUserDir sprawdza czy aktualny folder nie jest direm usera
		//funkcja RemoveDirectory zwraca 1 je�li uda�o si� usun��, 0 je�li nie
		if (checkIfUserDir(pathAnsiString) == false && RemoveDirectory(pathChar)){
			//ustawienie flagi usuni�cia katalogu
			catalogDeletedFlag = true;
			reasonLog = "(Zaznaczona opcja - Delete emptied catalogs)";
			//zapisanie log�w do pliku je�li uda�o si� usun�� pusty folder
			saveLogToTxt(pathChar);
		}
	}
	//usuwa wszystkie puste katalogi je�li zaznaczona opcja deleteAllEmptyCatalogs
	if (chkDeleteAllEmpty->Checked) {
		//konwersja String na const char * dla funkcji RemoveDirectory
		pathAnsiString = path;
		pathChar = pathAnsiString.c_str();

		//funkcja checkIfUserDir sprawdza czy aktualny folder nie jest direm usera
		//funkcja RemoveDirectory zwraca 1 je�li uda�o si� usun��, 0 je�li nie
		if (checkIfUserDir(pathAnsiString) == false && RemoveDirectory(pathChar)){
			//ustawienie flagi usuni�cia katalogu
			catalogDeletedFlag = true;
			reasonLog = "(Zaznaczona opcja - Delete all empty catalogs)";
			//zapisanie log�w do pliku je�li uda�o si� usun�c pusty folder
			saveLogToTxt(pathChar);
		}
	}
	//ustawienie warto�ci w vecOfDirSizes na aktulane warto�ci dir currentSizeOfDir
	vecOfDirSizes[it] = vecOfFilesMaps[it].getDirSize();
}
//---------------------------------------------------------------------------
/**
 * checkIfUserDir - Funkcja sprawdza, czy dana �cie�ka to nie �cie�ka dir podana przez usera
 * @param String path - �cie�ka do folderu podanego przez u�ytkownika
 * @return bool - true - je�li to �cie�ka usera (nie chcemy usun��), false - je�li nie
 */
bool TMainWindow::checkIfUserDir(AnsiString &path){
	//bool checkIfCatalogIsDir = false;
	//sprawdzenie, czy aktualny katalog jest dir usera je�li tak, to nie usuwamy
	for (UINT i = 0; i < vecOfPaths.size(); i++){
		if (path == vecOfPaths[i]) {
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------
/**
 * deleteFilesByDirSize - Funkcja usuwaj�ca z katalogu "path" wybrane pliki dop�ki
 * folder nie osi�gnie zadanego rozmiaru (usuwa najstarsze pliki)
 * @param String path - �cie�ka do folderu podanego przez u�ytkownika
 * @param UINT it - zawiera informacj�, kt�r� �cie�k� z wektora vecOfPaths aktualnie obs�ugujemy
 */
void TMainWindow::deleteFilesByDirSize(const String &path, UINT &it){
	bool deleteflag = false;

	AnsiString pathAnsiString;
	const char * pathChar;

	unsigned __int64 currentSizeOfDir = 0, userSizeToMaintain = 0, sizeOfDataToDelete = 0, deletedFileSizeCounter = 0;
	//przechowuje aktualn� wielko�� dir w trakcie usuwania
	currentSizeOfDir = StrToInt64(vecOfDirSizes[it]);
	//wielko�� podana przez usera do podtrzymania przemno�ona przez opcj� B/KB/MB/GB
	userSizeToMaintain = edtMaxDirSize->Text.ToDouble() * getMaxDirSizeOption();

	//je�li wielko�� podana przez usera jest mniejsza ni� aktualny rozmiar dir
	//to usu� tyle plik�w ile wynosi r�nica pomi�dzy tymi warto�ciami
	if (currentSizeOfDir > userSizeToMaintain) {
		sizeOfDataToDelete = currentSizeOfDir - userSizeToMaintain;
		deleteflag = true;
		//dop�ki suma wielko�ci plik�w fileSizeCounter jest mniejsza ni�
		//sizeOfDataToDelete usuwaj pliki od najstarszego
		while(deletedFileSizeCounter < sizeOfDataToDelete){
			//je�li nie ma plik�w w mapie to przerwij
			if (vecOfFilesMaps[it].filesMap.size() == 0) {
			   break;
			}
			//p�tla sprawdza, czy uda�o si� usun�� dany plik, je�li nie, usuwa nast�pny
			//do skutku, a� osi�gni�ty zostanie zadany poziom dir
			while (DeleteFile(vecOfFilesMaps[it].filesMap.begin()->second.pathOfFile) == 0) {
				//wy�wielta komunikat b��du usuni�cia pliku w logach
				deleteError(vecOfFilesMaps[it].filesMap.begin()->second.pathOfFile);
				//je�li istnieje jaki� plik w mapie
				if (vecOfFilesMaps[it].filesMap.size() > 0) {
					//usuni�cie pierwszego elementu z mapy
					vecOfFilesMaps[it].deleteElementFromMap(vecOfFilesMaps[it].filesMap.begin());
				}else{
					//je�li nie ma w mapie element�w
					ShowMessage("Delete Failed - cannot delete file. Check log for failed file info. (Probably incorrect file name and dir empty beside it.)");
					return;
				}
			}
			//zwi�kszenie deletedFileSizeCounter o rozmiar aktualnego pliku
			deletedFileSizeCounter += vecOfFilesMaps[it].filesMap.begin()->second.size;

			//usuni�cie pierwszego elementu z mapy
			vecOfFilesMaps[it].deleteElementFromMap(vecOfFilesMaps[it].filesMap.begin());

			//wpisanie aktualnego rozmiaru dir do zmiennej
			currentSizeOfDir = vecOfFilesMaps[it].getDirSize();

			//pozwala na dokonywanie zmian w trakcie dzia�ania p�tli
			Application->ProcessMessages();

			//iterator licznika do log�w
			fileCounterLog++;
		}
		//zapisanie log�w do pliku
		String convertToStr = deletedFileSizeCounter;
		reasonLog = " (Utrzymanie wielkosci dir, w sumie usunieto: " + convertToStr + " B)";
		saveLogToTxt(vecOfPaths[it]);
		//zerowanie licznika log�w
		fileCounterLog = 0;
	}
	//usuwa wszystkie puste katalogi je�li zaznaczona opcja deleteAllEmptyCatalogs
	if (chkDeleteAllEmpty->Checked) {
		for (std::multimap<long long int, FileInfo>::iterator iter = vecOfFoldersMaps[it].filesMap.begin(); iter != vecOfFoldersMaps[it].filesMap.end(); ++iter) {
			//konwersja String na const char * dla funkcji RemoveDirectory
			pathAnsiString = iter->second.pathOfFile;
			pathChar = pathAnsiString.c_str();
			//je�li uda�o si� usun�� folder usu� r�wnie� z wektora
			if (RemoveDirectory(pathChar)){
				//usuni�cie elementu z mapy je�li uda�o si� usun�� pusty folder
				vecOfFoldersMaps[it].deleteElementFromMap(iter);
			}
		}
	}
	//ustawienie warto�ci w vecOfDirSizes na aktulane warto�ci dir currentSizeOfDir
	vecOfDirSizes[it] = vecOfFilesMaps[it].getDirSize();

	//je�li usuni�to jaki� plik to zmie� warto�� dir size w comboBox
	if (deleteflag) {
		//wype�nienie cbbSizeOfDir
		fillCbbSizeOfDir();
		deleteflag = false;
	}
}
//---------------------------------------------------------------------------
/**
 * getUserSize - Funkcja zwracaj�ca rozmiar plikow (userSize) jakie maj� zosta� usuni�te
 * @return double userSize - min rozmiar plik�w jakie maj� zosta� usuni�te
 */
double TMainWindow::getUserSize(){
	double userSize;

	try {
		userSize = this->edtSize->Text.ToDouble();
		if (userSize < 0) {
			String error = "xxx";
			error = error.ToDouble();
		}
		//przemno�enie przez wybran� opcj� u�ytkowanika
		userSize = userSize*getSizeOption();
	}
	catch(EConvertError *err){
		ShowMessage("The value you provided is an invalid number.");
		this->edtSize->Text = "";
		userSize = -1;
	}

	return userSize;
}
//---------------------------------------------------------------------------
/**
 * getUserAge - Funkcja zwracaj�ca wiek plikow (userAge) podany przez u�ytkownika
 * @return double userAge - min wiek plik�w jakie maj� zosta� usuni�te
 */
double TMainWindow::getUserAge(){
	double userAge;

	try {
		userAge = this->edtAge->Text.ToDouble();
		if (userAge < 0) {
			String error = "xxx";
			error = error.ToDouble();
		}
		//przemno�enie przez wybran� opcj� u�ytkowanika
		userAge = userAge*getAgeOption();
	}
	catch(EConvertError *err){
		ShowMessage("The value you provided is an invalid number.");
		this->edtAge->Text = "";
		userAge = -1;
	}

	return userAge;
}
//---------------------------------------------------------------------------
/**
 * getUserRepeatTime - Funkcja zwracaj�ca wiek plik�w (userAge) podany przez u�ytkownika
 * @return double userRepeatTime - liczba jak� wpisa� u�ytkowanik w polu edtRepeatTime
 */
double TMainWindow::getUserRepeatTime(){
	double userRepeatTime;

	try {
		userRepeatTime = this->edtRepeatTime->Text.ToDouble();
		if (userRepeatTime <= 0) {
			String error = "xxx";
			error = error.ToDouble();
		}
		//przemno�enie przez wybran� opcj� u�ytkowanika
		userRepeatTime = userRepeatTime * getRepeadTime();
	}
	catch(EConvertError *err){
		ShowMessage("The value you provided is an invalid number.");
		this->edtAge->Text = "";
		userRepeatTime = -1;
	}

	return userRepeatTime;
}
//---------------------------------------------------------------------------
/**
 * getAgeOption - Funkcja zwraca mno�nik z wybranej opcji hour/day/month/year
 * @return double choosenAgeOption - zwraca mno�nik z wybranej opcji
 */
double TMainWindow::getAgeOption(){
	double choosenAgeOption;
	if(cbbAge->ItemIndex == 0){
		choosenAgeOption = 1;
	}else if(cbbAge->ItemIndex == 1){
		choosenAgeOption = 60;
	}else if(cbbAge->ItemIndex == 2){
		choosenAgeOption = 3600;
	}else if(cbbAge->ItemIndex == 3){
		choosenAgeOption = 86400;
	}else if(cbbAge->ItemIndex == 4){
		choosenAgeOption = 2592000;
	}else if(cbbAge->ItemIndex == 5){
		choosenAgeOption = 946080000;
	}
	return choosenAgeOption;
}
//---------------------------------------------------------------------------
/**
 * getMaxDirSizeOption - Funkcja zwraca mno�nik z wybranej opcji B/KB/MB/GB
 * @return double choosenSizeOption - zwraca mno�nik z wybranej opcji
 */
double TMainWindow::getMaxDirSizeOption(){
	double choosenSizeOption;
	if(cbbMaxDirSize->ItemIndex == 0){
		choosenSizeOption = 1;
	}else if(cbbMaxDirSize->ItemIndex == 1){
		choosenSizeOption = 1024;
	}else if(cbbMaxDirSize->ItemIndex == 2){
		choosenSizeOption = 1048576;
	}else if(cbbMaxDirSize->ItemIndex == 3){
		choosenSizeOption = 1073741824;
	}
	return choosenSizeOption;
}
//---------------------------------------------------------------------------
/**
 * getChosenDirSizeOption - Funkcja zwraca mno�nik z wybranej opcji B/KB/MB/GB
 * @return double choosenSizeOption - zwraca mno�nik z wybranej opcji
 */
double TMainWindow::getChosenDirSizeOption(){
	double choosenSizeOption;
	if(cbbChosenDirSize->ItemIndex == 0){
		choosenSizeOption = 1;
	}else if(cbbChosenDirSize->ItemIndex == 1){
		choosenSizeOption = 1024;
	}else if(cbbChosenDirSize->ItemIndex == 2){
		choosenSizeOption = 1048576;
	}else if(cbbChosenDirSize->ItemIndex == 3){
		choosenSizeOption = 1073741824;
	}
	return choosenSizeOption;
}
//---------------------------------------------------------------------------
/**
 * getSizeOption - Funkcja zwraca mno�nik z wybranej opcji B/KB/MB/GB
 * @return double choosenSizeOption - zwraca mno�nik z wybranej opcji
 */
double TMainWindow::getSizeOption(){
	double choosenSizeOption;
	if(cbbSize->ItemIndex == 0){
		choosenSizeOption = 1;
	}else if(cbbSize->ItemIndex == 1){
		choosenSizeOption = 1024;
	}else if(cbbSize->ItemIndex == 2){
		choosenSizeOption = 1048576;
	}else if(cbbSize->ItemIndex == 3){
		choosenSizeOption = 1073741824;
	}
	return choosenSizeOption;
}
//---------------------------------------------------------------------------
/**
 * getRepeadTime - Funkcja zwraca mno�nik z wybranej opcji minute/hour/day/month/year
 * @return double choosenTimeOption - zwraca mno�nik z wybranej opcji
 */
double TMainWindow::getRepeadTime(){
	double choosenTimeOption;
	if(cbbRepeatTime->ItemIndex == 0){
		choosenTimeOption = 1;
	}else if(cbbRepeatTime->ItemIndex == 1){
		choosenTimeOption = 60;
	}else if(cbbRepeatTime->ItemIndex == 2){
		choosenTimeOption = 3600;
	}else if(cbbRepeatTime->ItemIndex == 3){
		choosenTimeOption = 86400;
	}else if(cbbRepeatTime->ItemIndex == 4){
		choosenTimeOption = 2592000;
	}else if(cbbRepeatTime->ItemIndex == 5){
		choosenTimeOption = 946080000;
	}
	return choosenTimeOption;
}
//---------------------------------------------------------------------------
/**
 * sendToTxt - Funkcja zapisuje logi do pliku txt o zadanej scie�ce
 * @param String text - tekst wysy�any do pliku txt
 */
void TMainWindow::sendToTxt(const String &text){
	//�cie�ka pliku log o nazwie log_aktualna data
	String path = ExtractFilePath(Application->ExeName) + "Log\\" + "log_" + FileFunctions::getCurrentDay() + ".txt";
	//utworzenie obiektu outfile klasy istream z ustawieniem trybu otwarcia na dopisywanie
	ofstream outfile(path.c_str(), ios::app);
	//je�eli tworzenie obiektu zako�czy si� niepowodzeniem to przerwij operacj�
	if (!outfile) return;

	AnsiString text2 = text;
	//konwersja AnsiString na const char * dla funkcji outfile
	const char * buff = text2.c_str();
	outfile << buff << endl;
	//zamykanie pliku
	outfile.close();
}
//---------------------------------------------------------------------------
/**
 * deleteError - Funkcja zapisuje logi Errora do pliku txt o zadanej scie�ce
 * @param String pathOfFile - �cie�ka pliku kt�ry spowodowa� error
 */
void TMainWindow::deleteError(const String &pathOfFile){
	String textToSend = FileFunctions::getCurrentTime() + " - " + pathOfFile + " - NIE UDALO SIE USUNAC PLIKU - SPRAWDZ DLACZEGO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" + char(13) + char(10);
	sendToTxt(textToSend);
}
//---------------------------------------------------------------------------
/**
 * saveLogToTxt - Funkcja zapisuje logi do pliku txt o zadanej scie�ce
 * @param String pathOfFolder - folder kt�ry aktualnie jest logowany
 */
void TMainWindow::saveLogToTxt(const String &pathOfFolder){
	if (fileCounterLog) {
		String textToSend = FileFunctions::getCurrentTime() + " - " + pathOfFolder + " - Usunieto " + fileCounterLog + " plikow." + reasonLog + char(13) + char(10);
		sendToTxt(textToSend);
	}
	if (catalogDeletedFlag) {
		String textToSend = FileFunctions::getCurrentTime() + " - " + pathOfFolder + " - Usunieto pusty folder." + reasonLog + char(13) + char(10);
		sendToTxt(textToSend);
		catalogDeletedFlag = false;
	}
}
//---------------------------------------------------------------------------
/**
 * tmrAutoDeleteTimer - Funkcja Timera powtarza usuwanie plik�w o podanych parametrach, co podany czas
 */
void __fastcall TMainWindow::tmrAutoDeleteTimer(TObject *Sender){
	//je�li program jest wolny to od�wie�, je�li zaj�ty, to nie przerywaj dzia�ania
	if (edtProgramState->Color == clGreen) {
		//ustawienie koloru i napisu kontrolki edtProgramState
		programState(" BUSY", clRed);

		//usuwanie plik�w
		deleteFilesFunction();

		//wype�nienie cbbSizeOfDir
		fillCbbSizeOfDir();

		//od�wie�enie listy plik�w
		refreshListOfFiles();

		//uzupe�nienie edtChosenDirSize wielko�ci� aktualnie zaznaczonej �cie�ki
		cbbSelectedPathsToDeleteChange(NULL);

		//wyzerowanie licznik�w plik�w
		fileCounterLog = 0;
		catalogDeletedFlag = false;

		//ustawienie koloru i napisu kontrolki edtProgramState
		programState(" FREE", clGreen);
	}
}
//---------------------------------------------------------------------------
/**
 * FormResize - Funkcja ustala minimalne rozmiary okna programu
 */
void __fastcall TMainWindow::FormResize(TObject *Sender){
	MainWindow->Constraints->MinHeight = 540;
	MainWindow->Constraints->MinWidth = 540;
}
//---------------------------------------------------------------------------
/**
 * tmrAutoRefreshTimer - Timer od�wie�a aktualny czas w oknie programu
 */
void __fastcall TMainWindow::tmrAutoRefreshTimer(TObject *Sender){
	edtCurrentTime->Text = FileFunctions::getCurrentDate();
}
//---------------------------------------------------------------------------
/*
 * refreshComponents - Funkcja od�wie�a wszystkie parametry kontrolek i innych komponent�w programu
 */
void TMainWindow::refreshComponents(){
	//chkRepeatClick
	edtRepeatTime->Enabled = chkRepeat->Checked;
	cbbRepeatTime->Enabled = chkRepeat->Checked;

	if(chkRepeat->Checked){
		btnAutoDelete->Caption = "Auto Delete";
	}
	else if (chkRepeat->Checked == false && buttonChangeFlag == false){
		btnAutoDelete->Caption = "Delete";
	}

	//chkMaxAgeClick
	edtAge->Enabled = chkMaxAge->Checked;
	cbbAge->Enabled = chkMaxAge->Checked;

	//chkMaxSizeClick
	edtSize->Enabled = chkMaxSize->Checked;
	cbbSize->Enabled = chkMaxSize->Checked;

	//chkOwnFileExtension
	edtFileExtension->Enabled = chkOwnFileExtension->Checked;

	//chkMaxDirSize
	edtMaxDirSize->Enabled = chkMaxDirSize->Checked;
	cbbMaxDirSize->Enabled = chkMaxDirSize->Checked;

	//je�li opcja allFiles to odznacza wszystkie inne opcje rozszerze�
	if (chkAllFilesDelete->Checked) {
		chkImageDelete->Checked = false;
		chkTxtDelete->Checked = false;
		chkLogDelete->Checked = false;
		chkOwnFileExtension->Checked = false;
		chkImageDelete->Enabled = false;
		chkTxtDelete->Enabled = false;
		chkLogDelete->Enabled = false;
		chkOwnFileExtension->Enabled = false;
	}

	//je�li wybrano �cie�k� 
	if (selectFlag == true) {
		btnAutoDelete->Enabled = true;
		chkMaxAge->Enabled = true;
		chkMaxSize->Enabled = true;
		chkDeleteEmptiedCatalogs->Enabled = true;
		chkDeleteAllEmpty->Enabled = true;
		chkRepeat->Enabled = true;

		//je�li opcja chkMaxDirSize zaznaczona to odznacz chkMaxAgeClick i chkMaxSizeClick
		if (chkMaxDirSize->Checked) {
			chkMaxAge->Checked = false;
			chkMaxAge->Enabled = false;

			chkMaxSize->Checked = false;
			chkMaxSize->Enabled = false;
		}else {
			chkMaxAge->Enabled = true;

			chkMaxSize->Enabled = true;
		}

		//rozszerzenia plik�w
		chkImageDelete->Enabled = true;
		chkTxtDelete->Enabled = true;
		chkLogDelete->Enabled = true;
		chkOwnFileExtension->Enabled = true;

		//je�li opcja allFiles to odznacza wszystkie inne opcje rozszerze�
		if (chkAllFilesDelete->Checked) {
			chkImageDelete->Checked = false;
			chkTxtDelete->Checked = false;
			chkLogDelete->Checked = false;
			chkOwnFileExtension->Checked = false;
			chkImageDelete->Enabled = false;
			chkTxtDelete->Enabled = false;
			chkLogDelete->Enabled = false;
			chkOwnFileExtension->Enabled = false;
		}

		selectFlag = false;
	}

	if (buttonChangeFlag) {
		//je�li klikni�to przycisk delete/autoDelete to blokuje kontrolki
		chkMaxAge->Enabled = false;
		chkMaxSize->Enabled = false;
		chkDeleteEmptiedCatalogs->Enabled = false;
		chkDeleteAllEmpty->Enabled = false;
		cbbAge->Enabled = false;
		cbbSize->Enabled = false;
		cbbRepeatTime->Enabled = false;
		chkRepeat->Enabled = false;
		chkMaxDirSize->Enabled = false;
		edtMaxDirSize->Enabled = false;
		cbbMaxDirSize->Enabled = false;
		chkViewFiles->Enabled = false;
		chkAutoDeleteOnStartup->Enabled = false;

		//rozszerzenia plk�w
		chkAllFilesDelete->Enabled = false;
		chkImageDelete->Enabled = false;
		chkTxtDelete->Enabled = false;
		chkLogDelete->Enabled = false;
		chkOwnFileExtension->Enabled = false;
		edtFileExtension->Enabled = false;

		edtRepeatTime->Enabled = false;
		edtAge->Enabled = false;
		edtSize->Enabled = false;
		btnAutoDelete->Caption = "Change Config";
		btnAddPath->Enabled = false;
		btnDeletePath->Enabled = false;
		btnLoadConfig->Enabled = false;
	}
	else{
		chkMaxAge->Enabled = true;
		chkMaxSize->Enabled = true;
		chkDeleteEmptiedCatalogs->Enabled = true;
		chkDeleteAllEmpty->Enabled = true;
		chkRepeat->Enabled = true;
		btnAddPath->Enabled = true;
		btnDeletePath->Enabled = true;
		chkMaxDirSize->Enabled = true;
		chkViewFiles->Enabled = true;
		chkAutoDeleteOnStartup->Enabled = true;

		//je�li opcja chkMaxDirSize zaznaczona to odznacz chkMaxAgeClick i chkMaxSizeClick
		//oraz odznacz opcje rozszerze�, zaznacz opcj� 'all'
		if (chkMaxDirSize->Checked) {
			chkMaxAge->Checked = false;
			chkMaxAge->Enabled = false;

			chkMaxSize->Checked = false;
			chkMaxSize->Enabled = false;

			chkAllFilesDelete->Checked = true;
		}else {
			chkMaxAge->Enabled = true;
			chkMaxSize->Enabled = true;
		}

		//rozszerzenia plk�w
		chkAllFilesDelete->Enabled = true;
		chkImageDelete->Enabled = true;
		chkTxtDelete->Enabled = true;
		chkLogDelete->Enabled = true;
		chkOwnFileExtension->Enabled = true;

		//je�li opcja allFiles to odznacza wszystkie inne opcje rozszerze�
		if (chkAllFilesDelete->Checked) {
			chkImageDelete->Checked = false;
			chkTxtDelete->Checked = false;
			chkLogDelete->Checked = false;
			chkOwnFileExtension->Checked = false;
			chkImageDelete->Enabled = false;
			chkTxtDelete->Enabled = false;
			chkLogDelete->Enabled = false;
			chkOwnFileExtension->Enabled = false;
		}

		if(chkRepeat->Checked){
			btnAutoDelete->Caption = "Auto Delete";
		}
		else if (chkRepeat->Checked == false && buttonChangeFlag == false){
			btnAutoDelete->Caption = "Delete";
		}
		btnLoadConfig->Enabled = true;
	}

	if (loadCfgFlag) {
		btnAutoDelete->Enabled = true;
		chkMaxAge->Enabled = true;
		chkMaxSize->Enabled = true;
		chkDeleteEmptiedCatalogs->Enabled = true;
		chkDeleteAllEmpty->Enabled = true;
		chkRepeat->Enabled = true;

		//rozszerzenia plk�w
		chkImageDelete->Enabled = true;
		chkTxtDelete->Enabled = true;
		chkLogDelete->Enabled = true;
		chkOwnFileExtension->Enabled = true;

		//je�li opcja chkMaxDirSize zaznaczona to odznacz chkMaxAgeClick i chkMaxSizeClick
		if (chkMaxDirSize->Checked) {
			chkMaxAge->Checked = false;
			chkMaxAge->Enabled = false;

			chkMaxSize->Checked = false;
			chkMaxSize->Enabled = false;

			chkAllFilesDelete->Checked = true;
		}

		//je�li opcja allFiles to odznacza wszystkie inne opcje rozszerze�
		if (chkAllFilesDelete->Checked) {
			chkImageDelete->Checked = false;
			chkTxtDelete->Checked = false;
			chkLogDelete->Checked = false;
			chkOwnFileExtension->Checked = false;
			chkImageDelete->Enabled = false;
			chkTxtDelete->Enabled = false;
			chkLogDelete->Enabled = false;
			chkOwnFileExtension->Enabled = false;
		}

		loadCfgFlag = false;
	}
}
//---------------------------------------------------------------------------
/*
 * btnSaveConfigClick - Funkcja wywo�uje zapisanie ustawie� do pliku ini
 */
void __fastcall TMainWindow::btnSaveConfigClick(TObject *Sender){
	saveConfigToIni();
}
//---------------------------------------------------------------------------
/**
 * checkDot - Funkcja sprawdza, czy w stringu jest kropka, je�li tak to znaczy,
 * �e podano rozszerzenie pliku
 * @param UnicodeString input - �a�cuch string do sprawdzenia czy jest w nim "."
 * @return bool - true - je�li jest kropka, false - je�li nie ma
 */
bool TMainWindow::checkDot(String input){
	for (int i = 1; i <= input.Length(); i++) {
		if(input.SubString(i,1) == ".") {
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------
/**
 * deleteExtFromFile - Funkcja usuwa ze stringa rozszerzenie wraz z kropk�
 * @param UnicodeString input - �a�cuch string z kt�rego usuwamy rozszerzenie
 * @return String fileNameNoExt - �a�cuch string bez rozszerzenia
 */
String TMainWindow::deleteExtFromFile(String &input){
	int posOfDot = 0;

	//przeszukuje Stringa i znajduje pozycje kropki
	for (int i = 1; i <= input.Length(); i++) {
		if(input.SubString(i,1) == ".") {
			posOfDot = i;
		}
	}

	//string z usuni�tym rozszerzeniem
	String fileNameNoExt = input.SubString(1, posOfDot - 1);

	return fileNameNoExt;
}
//---------------------------------------------------------------------------
/*
 * saveConfigToIni - Funkcja zapisuje aktualne ustawienia usuwania do pliku ini
 */
void TMainWindow::saveConfigToIni(){
	boost::scoped_ptr<TIniFile> Ini;

	boost::scoped_ptr<TSaveDialog> SaveDialog1(new TSaveDialog(this));
	SaveDialog1->Title = "Save a canfiguration file";	//tytu�
	SaveDialog1->Filter = L"Ini Files (*.ini)|*.ini|";	//wybranie rozszerze� plik�w jakie mog� zosta� zapisane

	//je�li nie klikni�to zapisz, to wr��
	if (SaveDialog1->Execute() == false) {
		return;
	}

	//przeszukanie stringa SaveDialog1->FileName, jesli jest "." zamieniamy to co po "." na ".ini",
	//je�li nie, to dodajemy ".ini" do ko�ca stringa
	if (checkDot(SaveDialog1->FileName)) {
		String bufor = SaveDialog1->FileName;
		//utworzenie pliku Ini
		Ini.reset(new TIniFile(deleteExtFromFile(bufor) + ".ini"));
	}else{
		//utworzenie pliku Ini
		Ini.reset(new TIniFile(SaveDialog1->FileName + ".ini"));
	}

	//usuni�cie zawarto�ci ini przed zapisaniem aktualnych wartosci
	Ini->EraseSection("Section1");

	//zapisanie countera �cie�ek
	Ini->WriteString("Section1", "Zawartosc edtPathsCounter", edtPathsCounter->Text);
	//zapisanie wektora ze �cie�kami do usuwania
	for (UINT i = 0; i < vecOfPaths.size(); i++) {
		Ini->WriteString("Section1", "Zawartosc wektora sciezek " + StrToInt(i), vecOfPaths[i]);
	}
	//zapiasnie ustawie� wieku pliku
	Ini->WriteBool("Section1", "Ustawienia chkMaxAge", chkMaxAge->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteString("Section1", "Zawartosc edtAge", edtAge->Text); // zapisywanie tekstu do sekcji [Section1].
	Ini->WriteInteger("Section1", "Zawartosc cbbAge", cbbAge->ItemIndex); // zapisywanie tekstu do sekcji [Section1].
	//zapiasnie ustawie� rozmiaru pliku
	Ini->WriteBool("Section1", "Ustawienia chkMaxSize", chkMaxSize->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteString("Section1", "Zawartosc edtSize", edtSize->Text); // zapisywanie tekstu do sekcji [Section1].
	Ini->WriteInteger("Section1", "Zawartosc cbbSize", cbbSize->ItemIndex); // zapisywanie tekstu do sekcji [Section1].
	//zapiasnie ustawie� automatycznego powtarzania usuwania
	Ini->WriteBool("Section1", "Ustawienia chkRepeat", chkRepeat->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteString("Section1", "Zawartosc edtRepeatTime", edtRepeatTime->Text); // zapisywanie tekstu do sekcji [Section1].
	Ini->WriteInteger("Section1", "Ustawienia cbbRepeatTime", cbbRepeatTime->ItemIndex);// zapisywanie warto�ci true/false do sekcji [Section1].
	//zapisaywanie checkBox�w usuwania folder�w
	Ini->WriteBool("Section1", "Ustawienia chkDeleteEmptiedCatalogs", chkDeleteEmptiedCatalogs->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteBool("Section1", "Ustawienia chkDeleteAllEmpty", chkDeleteAllEmpty->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	//zapisywanie usatwie� kategorii plik�w (rozszerze�) do usuni�cia
	Ini->WriteBool("Section1", "Ustawienia chkAllFilesDelete", chkAllFilesDelete->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteBool("Section1", "Ustawienia chkImageDelete", chkImageDelete->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteBool("Section1", "Ustawienia chkTxtDelete", chkTxtDelete->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteBool("Section1", "Ustawienia chkLogDelete", chkLogDelete->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteBool("Section1", "Ustawienia chkOwnFileExtension", chkOwnFileExtension->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteString("Section1", "Zawartosc edtFileExtension", edtFileExtension->Text); // zapisywanie tekstu do sekcji [Section1].
	//zapiasnie ustawie� max rozmiaru directory
	Ini->WriteBool("Section1", "Ustawienia chkMaxDirSize", chkMaxDirSize->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	Ini->WriteString("Section1", "Zawartosc edtMaxDirSize", edtMaxDirSize->Text); // zapisywanie tekstu do sekcji [Section1].
	Ini->WriteInteger("Section1", "Zawartosc cbbMaxDirSize", cbbMaxDirSize->ItemIndex); // zapisywanie tekstu do sekcji [Section1].
	//zapisanie autoDelete na starcie
	Ini->WriteBool("Section1", "Ustawienia chkAutoDeleteOnStartup", chkAutoDeleteOnStartup->Checked);// zapisywanie warto�ci true/false do sekcji [Section1].
	//zapisanie wybranej opcji B/KB/MB/GB cbbChosenDirSize
	Ini->WriteInteger("Section1", "Zawartosc cbbChosenDirSize", cbbChosenDirSize->ItemIndex); // zapisywanie int do sekcji [Section1].)
}
//---------------------------------------------------------------------------
/*
 * loadConfigFromIni - Funkcja odczytuje aktualne ustawienia usuwania plik�w z ini
 */
void TMainWindow::loadConfigFromIni(){
	boost::scoped_ptr<TIniFile> Ini;

	//przy pierwszym uruchomieniu autowczytywanie z autoConfig.ini
	if (startFlag == false) {
		//wybieranie pliku ini automatycznie
		Ini.reset(new TIniFile(ExtractFilePath(Application->ExeName) + "autoConfig.ini"));
		startFlag = true;
	}
	else {
		//otwarcie okna do wyboru pliku z kt�rego chcemy za�adowa� konfiguracje
		boost::scoped_ptr<TOpenDialog> dlg(new TOpenDialog(this));
		dlg->Title= "Choose a configuration file to load";	//tytu�
		dlg->Filter = "Ini files (*.ini)|*.INI";	//wybranie rozszerze� plik�w jakie mog� zosta� zapisane

		if (dlg->Execute() == false) {
			return;
		}

		//utworzenie pliku Ini
		Ini.reset(new TIniFile(dlg->FileName));
	}

	//ustawienie koloru i napisu kontrolki edtProgramState
	programState(" BUSY",clRed);

	//wyczyszczenie wektor�w danych i cbb je�li za�adowano config
	vecOfPaths.clear();
	vecOfDirSizes.clear();
	vecOfFilesMaps.clear();
	vecOfFoldersMaps.clear();
	vecOfExtensions.clear();
	cbbSelectedPathsToDelete->Items->Clear();
	cbbSizeOfDir->Items->Clear();
	edtChosenDirSize->Clear();

	//odczytanie licznika �cie�ek edtPathsCounter
	edtPathsCounter->Text = Ini->ReadString("Section1", "Zawartosc edtPathsCounter", 0); // odczytywanie tekstu z sekcji [Section1].

	if (edtPathsCounter->Text.ToInt()) {
		//wyczyszczenie pola tekstowego cbbSelectedPathsToDelete
		cbbSelectedPathsToDelete->Text = "";
		AnsiString currentPath;
		int iteratorOfPaths = 0;
		int counterOfPaths = edtPathsCounter->Text.ToInt();
		for (int i = 0; i < counterOfPaths; i++) {
			currentPath = Ini->ReadString("Section1", "Zawartosc wektora sciezek " + StrToInt(i), "");
			//sprawdzanie, czy �cie�ka dodawana z pliku ini istnieje
			if (FileFunctions::dirExists(currentPath)) {
				//dodanie �cie�ek do wektora vecOfPaths
				vecOfPaths.push_back(currentPath);
				//dodanie �cie�ek do cbbSelectedPathsToDelete
				cbbSelectedPathsToDelete->Items->Add(vecOfPaths[i]);
			}else{
				//je�li �cie�ka z configa ju� nie istnieje
				iteratorOfPaths++;
				edtPathsCounter->Clear();
				edtPathsCounter->Text = counterOfPaths - iteratorOfPaths;
			}
		}
		//wype�nienie wektora vecOfDirSizes i cbbSizeOfDir
		for (UINT i = 0; i < static_cast<UINT>(edtPathsCounter->Text.ToInt()); i++) {
			//utworzenie pustych obiekt�w SortMap aby wpisa� do nich p�niej mapy
			vecOfFilesMaps.push_back(SortMap());
			vecOfFoldersMaps.push_back(SortMap());

            //dodanie element�w do map plik�w i folder�w
			fillMaps(vecOfPaths[i], i);

			//dodanie rozmiar�w dir do wektora vecOfDirSizes
			vecOfDirSizes.push_back(vecOfFilesMaps[i].getDirSize());

			//dodanie rozmiar�w dir do cbbSizeOfDir
			cbbSizeOfDir->Items->Add(vecOfDirSizes[i]);
		}
	}

	//odczytywanie ustawie� maxDirSize
	chkMaxDirSize->Checked = Ini->ReadBool("Section1", "Ustawienia chkMaxDirSize", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	edtMaxDirSize->Text = Ini->ReadString("Section1", "Zawartosc edtMaxDirSize", ""); // odczytywanie tekstu z sekcji [Section1].
	cbbMaxDirSize->ItemIndex = Ini->ReadInteger("Section1", "Zawartosc cbbMaxDirSize", 3); //odczytywanie tekstu z sekcji [Section1].

	//odczytywanie ustawie� wieku pliku
	chkMaxAge->Checked = Ini->ReadBool("Section1", "Ustawienia chkMaxAge", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	edtAge->Text = Ini->ReadString("Section1", "Zawartosc edtAge", ""); // odczytywanie tekstu z sekcji [Section1].
	cbbAge->ItemIndex = Ini->ReadInteger("Section1", "Zawartosc cbbAge", 4); // odczytywanie tekstu z sekcji [Section1].
	//odczytywanie ustawie� rozmiaru pliku
	chkMaxSize->Checked = Ini->ReadBool("Section1", "Ustawienia chkMaxSize", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	edtSize->Text = Ini->ReadString("Section1", "Zawartosc edtSize", ""); // odczytywanie tekstu z sekcji [Section1].
	cbbSize->ItemIndex = Ini->ReadInteger("Section1", "Zawartosc cbbSize", 3); //odczytywanie tekstu z sekcji [Section1].
	//odczytywanie ustawie� automatycznego powtarzania usuwania
	chkRepeat->Checked = Ini->ReadBool("Section1", "Ustawienia chkRepeat", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	edtRepeatTime->Text = Ini->ReadString("Section1", "Zawartosc edtRepeatTime", 1); // odczytywanie tekstu z sekcji [Section1].
	cbbRepeatTime->ItemIndex = Ini->ReadInteger("Section1", "Ustawienia cbbRepeatTime", 3);// odczytywanie tekstu z sekcji [Section1].
	//odczytywanie checkBox�w usuwania folder�w
	chkDeleteEmptiedCatalogs->Checked = Ini->ReadBool("Section1", "Ustawienia chkDeleteEmptiedCatalogs", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	chkDeleteAllEmpty->Checked = Ini->ReadBool("Section1", "Ustawienia chkDeleteAllEmpty", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	//odczytywanie usatwie� kategorii plik�w (rozszerze�) do usuni�cia
	chkAllFilesDelete->Checked = Ini->ReadBool("Section1", "Ustawienia chkAllFilesDelete", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	chkImageDelete->Checked = Ini->ReadBool("Section1", "Ustawienia chkImageDelete", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	chkTxtDelete->Checked = Ini->ReadBool("Section1", "Ustawienia chkTxtDelete", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	chkLogDelete->Checked = Ini->ReadBool("Section1", "Ustawienia chkLogDelete", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	chkOwnFileExtension->Checked = Ini->ReadBool("Section1", "Ustawienia chkOwnFileExtension", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	edtFileExtension->Text = Ini->ReadString("Section1", "Zawartosc edtFileExtension",""); // odczytywanie tekstu z sekcji [Section1].
	//odczytywanie autoDelete na starcie
	chkAutoDeleteOnStartup->Checked = Ini->ReadBool("Section1", "Ustawienia chkAutoDeleteOnStartup", 0);// odczytywanie warto�ci true/false z sekcji [SEKCJA1].
	//odczytanie wybranej opcji B/KB/MB/GB cbbChosenDirSize
	cbbChosenDirSize->ItemIndex = Ini->ReadInteger("Section1", "Zawartosc cbbChosenDirSize", 2); // odczytywanie int do sekcji [Section1].)

	//ustawienie flagi wczytania ustawie� z pliku ini
	loadCfgFlag = true;

	//od�wie�a komponenty cbb, edt, chk itd.
	refreshComponents();

	//od�wie�enie listy plik�w
	refreshListOfFiles();

	//je�li jest zaznaczona opcja chkAutoDeleteOnStartup to wykonuj od razu
	//AutoDelete z wyznaczonych �cie�ek
	if (chkAutoDeleteOnStartup->Checked) {
		btnAutoDeleteClick(NULL);
	}

	//ustawienie koloru i napisu kontrolki edtProgramState
	programState(" FREE", clGreen);
}
//---------------------------------------------------------------------------
/*
 * btnLoadConfigClick - Funkcja wywo�uje odczytanie ustawie� z pliku ini
 */
void __fastcall TMainWindow::btnLoadConfigClick(TObject *Sender){
	loadConfigFromIni();
}
//---------------------------------------------------------------------------
/*
 * FormCreate - Tworzy okno programu i obiekt TTrayIcon
 */
void __fastcall TMainWindow::FormCreate(TObject *Sender){
	trayIcon= new TTrayIcon(this);
	trayIcon->OnClick = trayIconClick;

	//ukrywa ikon� po w��czeniu programu
	trayIcon->Visible = true;
	Hide();
	this->WindowState = wsMinimized;
    //ukrywa ikon� po klikni�ciu minimalizacji
	Application->OnMinimize = OnAppMinimize;
}
//---------------------------------------------------------------------------
/*
 * OnAppMinimize - po klikni�ciu minimalizacji ukrywa program w tray'u
 */
void __fastcall TMainWindow::OnAppMinimize(TObject *Sender){
	trayIcon->Visible = true;
	Hide();
	this->WindowState = wsMinimized;
}
//---------------------------------------------------------------------------
/*
 * trayIconClick - przywraca okno po klikni�ciu ikony w tray'u
 */
void __fastcall TMainWindow::trayIconClick(TObject *Sender){
	Show();
	this->WindowState = wsNormal;
	Application->BringToFront();
	trayIcon->Visible = false;
}
//---------------------------------------------------------------------------
/**
 * Destruktor
 */
__fastcall TMainWindow::~TMainWindow(){
	delete trayIcon;
	trayIcon = NULL;
}
//---------------------------------------------------------------------------
/**
 * chkOwnFileExtensionClick - Obs�uga klikni�cia kontrolki chkOwnFileExtension
 */
void __fastcall TMainWindow::chkOwnFileExtensionClick(TObject *Sender){
	//od�wie�a komponenty cbb, edt, chk itd.
	refreshComponents();
}
//---------------------------------------------------------------------------
/**
 * fillVectorOfExtensions - Funkcja pobiera ze stringa pozbawionego spacji tekst
 * pomi�dzy przecinkami i umieszcza go do wektora - rozszerzenia plik�w
 * @param AnsiString tmp - �a�cuch string bez spacji
 * @return vector<AnsiString> vecOfExtensions - wektor podanych przez usera rozszerze�
 */
void TMainWindow::fillVectorOfExtensions(const AnsiString &tmp){
	vector<int>vecOfCommaPos;
	vecOfCommaPos.clear();
	vecOfExtensions.clear();

	//znajduje wszystkie przecinki i dodaje ich numer wyst�pienia w stringu do wektora
	for(char * posOfSign = strchr(tmp.c_str(), ',' ); posOfSign != NULL; posOfSign = strchr( posOfSign + 1, ',' )){
		vecOfCommaPos.push_back(posOfSign - tmp.c_str());
	}
	//wpisanie rozszerze� bez spacji
	edtFileExtension->Text = tmp;

	//wype�nienie wektora vecOfExtensions rozszerzeniami plik�w
	if (tmp.Length()>1) {
		//je�li podano tylko 1 rozszerzenie
		if (vecOfCommaPos.size() == 0) {
			vecOfExtensions.push_back(tmp);
			return;
		}
		
		//dla pierwszego rozszerzenia
		vecOfExtensions.push_back(tmp.SubString(0,vecOfCommaPos[0]));

		AnsiString currentExtension;
		//dla wszystkich kolejnych rozszerze� miedzy przecinkami
		for (UINT i = 0; i < vecOfCommaPos.size()-1; i++) {
			currentExtension = tmp.SubString(vecOfCommaPos[i]+2,vecOfCommaPos[i+1] - vecOfCommaPos[i] - 1);
			if (currentExtension.Length() > 0) {
				vecOfExtensions.push_back(currentExtension);
			}
		}
		//dla ostatniego rozszerzenia po przecinku
		currentExtension = tmp.SubString(vecOfCommaPos.back()+2,tmp.Length() - vecOfCommaPos.back() - 1);
		if (currentExtension.Length() > 0) {
			vecOfExtensions.push_back(currentExtension);
		}
	}
}
//---------------------------------------------------------------------------
/**
 * chkMaxDirSizeClick - Obs�uga klikni�cia kontrolki chkMaxDirSize
 */
void __fastcall TMainWindow::chkMaxDirSizeClick(TObject *Sender){
	//od�wie�a komponenty cbb, edt, chk itd.
	refreshComponents();
}
//---------------------------------------------------------------------------
/**
 * programState - Funkcja programState ustawia kolor i napis kontrolki edtProgramState
 */
void TMainWindow::programState(const String &progState, System::Uitypes::TColor color){
	edtProgramState->Text = progState;
	edtProgramState->Color = color;
}
//---------------------------------------------------------------------------
/**
 * chkViewFilesClick - Obs�uga klikni�cia kontrolki chkViewFiles
 */
void __fastcall TMainWindow::chkViewFilesClick(TObject *Sender){
	//ustawienie koloru i napisu kontrolki edtProgramState
	programState(" BUSY", clRed);

	if (chkViewFiles->Checked) {
		refreshListOfFiles();
	}else{
		//czyszzcenie listView
		lvFileListView->Clear();
	}

	//ustawienie koloru i napisu kontrolki edtProgramState
	programState(" FREE", clGreen);
}
//---------------------------------------------------------------------------
/**
 * refreshListOfFiles - od�wie�a list� ListWiew
 */
void TMainWindow::refreshListOfFiles(){
    //ustawienie koloru i napisu kontrolki edtProgramState
	programState(" BUSY", clRed);

	if (chkViewFiles->Checked){
		//czyszczenie listView
		lvFileListView->Clear();
		for (UINT i = 0; i < vecOfPaths.size(); i++) {
			//wywo�anie funkcji kt�ra wype�nia ListView plikami z wybranego katalogu
			listCatalogs(vecOfPaths[i], i);
		}
	}

	//ustawienie koloru i napisu kontrolki edtProgramState
	programState(" FREE", clGreen);
}
//---------------------------------------------------------------------------
/**
 * chkAllFilesDeleteClick - Obs�uga klikni�cia kontrolki chkAllFilesDelete
 */
void __fastcall TMainWindow::chkAllFilesDeleteClick(TObject *Sender){
	//od�wie�a komponenty cbb, edt, chk itd.
	refreshComponents();
}
//---------------------------------------------------------------------------
/**
 * btnExtensionApplyClick - przycisk do potwierdzenia dodanych rozszerze� do usuni�cia
 * dodaje je r�nie� do wektora vecOfExtensions
 */
void TMainWindow::extensionApply(bool flag){
	//je�li wci�ni�to przycisk
	if (flag) {
		//odczytanie nazw rozszerze� z form1
		UnicodeString userFileExtensions = edtFileExtension->Text;

		//funkcja usuwa spacje ze stringa
		UnicodeString tmp = FileFunctions::removeSpaces(userFileExtensions);

		//funkcja zwraca wektor rozszerze� plik�w do usuni�cia
		fillVectorOfExtensions(tmp);
		//wy��cza mo�liwo�� zmiany rozszerze�
		edtFileExtension->Enabled = false;
	}else{
		//w��cza mo�liwo�� zmiany rozszerze�
		edtFileExtension->Enabled = true;
		//wyczyszczenie wektora rozszerze�
		vecOfExtensions.clear();
	}
}
//---------------------------------------------------------------------------
/**
 * btnAddPathClick - dodaje wybran� �cie�k� do cbbSelectedPathsToDelete, cbbSizeOfDir i vecOfPaths, vecOfDirSizes
 */
void __fastcall TMainWindow::btnAddPathClick(TObject *Sender){
	//�cie�ka pocz�tkowa
	String strDirectory = ExtractFilePath(Application->ExeName);
	//umieszcza �cie�k� wybran� przez u�ytkowanika w strDirectory
	int SelectionOK = SelectDirectory(strDirectory, TSelectDirOpts(), 0);

	if (SelectionOK) {
		//je�li trafi� si� jakikolwiek wyj�tek to wy�wietla komunikat i nie pozwala usuwa�
		if (deleteExceptions(strDirectory)){
			return;
		}

		//ustawienie koloru i napisu kontrolki edtProgramState
		programState(" BUSY", clRed);

		//je�li to 1 dodawany element do cbb
		if (cbbSelectedPathsToDelete->Items->Count == 0) {
			//dodanie wybranej �cie�ki do cbbSelectedPathsToDelete
			cbbSelectedPathsToDelete->Items->Add(strDirectory + "\\");
		}else{
			bool pathExistFlag = false;
			//sprawdzenie, czy dana �cie�ka ju� istnieje w cbbSelectedPathsToDelete
			for (int i = 0; i < cbbSelectedPathsToDelete->Items->Count; i++) {
				cbbSelectedPathsToDelete->ItemIndex = i;
				if (cbbSelectedPathsToDelete->Text == strDirectory + "\\") {
					//ustawienie flagi, je�li �cie�ka istnieje
					pathExistFlag = true;
				}
			}
			//je�li �cie�ka ju� by�a w comboBox to komunikat je�li nie to dodajemy
			if (pathExistFlag) {
				ShowMessage("This path is already added! Choose another.");
			}else{
				//dodanie wybranej �cie�ki do cbbSelectedPathsToDelete
				cbbSelectedPathsToDelete->Items->Add(strDirectory + "\\");
			}
		}

		//wype�nia wektor vecOfPaths �cie�kami wybranymi przez usera
		fillVectorOfPaths();

		//wpisanie aktualnej �cie�ki do zmiennej
		String currentPath = strDirectory + "\\";
		UINT bufor = vecOfPaths.size() - 1;

		//je�li zaznaczono opcj� chkViewFiles to listuj pliki
		if(chkViewFiles->Checked) listCatalogs(currentPath, bufor);

		//utworzenie pustych obiekt�w SortMap aby wpisa� do nich p�niej mapy
		vecOfFilesMaps.push_back(SortMap());
		vecOfFoldersMaps.push_back(SortMap());

		//dodanie element�w do map plik�w i folder�w
		fillMaps(currentPath, bufor);

		//dodanie aktualnej wielko�ci do wektora vecOfDirSizes
		vecOfDirSizes.push_back(vecOfFilesMaps[bufor].getDirSize());

		//wype�nienie cbbSizeOfDir wielko�ciami dir
		fillCbbSizeOfDir();

		//od�wie�a komponenty w oknie programu
		refreshComponents();

		//uzupe�nienie edtChosenDirSize wielko�ci� aktualnie zaznaczonej �cie�ki
		cbbSelectedPathsToDeleteChange(NULL);

		//ustawienie koloru i napisu kontrolki edtProgramState
		programState(" FREE", clGreen);
	}
}
//---------------------------------------------------------------------------
/**
 * fillCbbSizeOfDir - Funkcja wype�nia cbbSizeOfDir wielko�ciami dir
 */
void TMainWindow::fillCbbSizeOfDir(){
	//wyczyszczenie cbbSizeOfDir
	cbbSizeOfDir->Items->Clear();

	//dodanie do cbbSizeOfDir elementow zawartych w wektorze vecOfDirSizes
	for (UINT i = 0; i < vecOfDirSizes.size(); i++) {
		cbbSizeOfDir->Items->Add(vecOfDirSizes[i]);
	}
}
//---------------------------------------------------------------------------
/**
 * fillVectorOfPaths - Funkcja wype�nia wektor vecOfPaths �cie�kami wybranymi przez usera
 */
void TMainWindow::fillVectorOfPaths(){
	//wyczyszczenie wektora �cie�ek
	vecOfPaths.clear();

	//wype�nienie wektora kolejnymi wybranymi przez usera scie�kami z cbbSelectedPathsToDelete
	for (int i = 0; i < cbbSelectedPathsToDelete->Items->Count; i++) {
		cbbSelectedPathsToDelete->ItemIndex = i;
		//dodanie danej �cie�ki do wektora
		String currentPath = cbbSelectedPathsToDelete->Text;
		vecOfPaths.push_back(currentPath);
	}
	//wy�wietla liczb� wybranych �cie�ek w edtPathsCounter
	edtPathsCounter->Text = IntToStr(cbbSelectedPathsToDelete->Items->Count);
}
//---------------------------------------------------------------------------
/**
 * btnDeletePathClick - usuwa wybran� �cie�k� z cbbSelectedPathsToDelete, cbbSizeOfDir i vecOfPaths, vecOfDirSizes
 */
void __fastcall TMainWindow::btnDeletePathClick(TObject *Sender){
	//zabezpieczenie przed usuwaniem element�w, gdy �aden nie jest zaznaczony
	if (cbbSelectedPathsToDelete->Text == "") {
		ShowMessage("No selection.");
		return;
	}
	int selectedItemIndex = cbbSelectedPathsToDelete->ItemIndex;
	//usuwa wybran� �cie�k� z cbbSelectedPathsToDelete
	cbbSelectedPathsToDelete->Items->Delete(selectedItemIndex);

	//wype�nia od nowa wektor vecOfPaths �cie�kami wybranymi przez usera
	fillVectorOfPaths();

	//usuwanie warto�ci size z cbb i wektora
	vecOfDirSizes.erase(vecOfDirSizes.begin() + selectedItemIndex);

	//wype�nienie cbbSizeOfDir wielko�ciami dir
	fillCbbSizeOfDir();

	//wyczyszczenie edtChosenDirSize
	edtChosenDirSize->Clear();

	//uzupe�nienie edtChosenDirSize wielko�ci� aktualnie zaznaczonej �cie�ki
	cbbSelectedPathsToDeleteChange(NULL);

	//od�wie� list� plik�w ListView
	refreshListOfFiles();

	//wyczyszczenie wektora wielko�ci dir�w
	if (vecOfDirSizes.size() == 0) {
		cbbSelectedPathsToDelete->Clear();
	}

    //je�li nie ma ju� �cie�ek w cbbSelectedPathsToDelete to wyswietl tekst
	if (cbbSelectedPathsToDelete->Items->Count == 0) {
		cbbSelectedPathsToDelete->Text = "No selection.";
	}

	//usuwanie element�w wektor�w map
	vecOfFilesMaps.erase(vecOfFilesMaps.begin() + selectedItemIndex);
	vecOfFoldersMaps.erase(vecOfFoldersMaps.begin() + selectedItemIndex);
}
//---------------------------------------------------------------------------
/**
 * chkAutoDeleteOnStartupClick - Obs�uga klikni�cia kontrolki chkAutoDeleteOnStartup
 */
void __fastcall TMainWindow::chkAutoDeleteOnStartupClick(TObject *Sender){
	//od�wie�a komponenty w oknie programu
	refreshComponents();
}
//---------------------------------------------------------------------------
/**
 * cbbSelectedPathsToDeleteChange - ustawia edtChosenDirSize na wielko�� wybranego dir
 */
void __fastcall TMainWindow::cbbSelectedPathsToDeleteChange(TObject *Sender){
	int selectedItemIndex = cbbSelectedPathsToDelete->ItemIndex;
	//pobiera wielko�� dir z wektora i przemna�a j� przez wybran� opcj�
	if (selectedItemIndex != -1) {
		double sizeToShow = vecOfDirSizes[selectedItemIndex].ToDouble() / getChosenDirSizeOption();

		String convert;
		//konwersja zeby wy�wietla� liczbe z 2 miejscami po przecinku
		convert.sprintf(L"%.2f", sizeToShow);
		//wy�wietlenie liczby w edtChosenDirSize
		edtChosenDirSize->Text = convert;
	}
}
//---------------------------------------------------------------------------
/**
 * cbbChosenDirSizeChange - od�wie�a edtChosenDirSize wg wybranej opcji B/KB/MB/GB
 */
void __fastcall TMainWindow::cbbChosenDirSizeChange(TObject *Sender){
	//zabezpieczenie przed zmian� comboboxa w trakcie dzia�ania programu
	if (edtChosenDirSize->Text == "") {
		return;
	}
	cbbSelectedPathsToDeleteChange(NULL);
}
//---------------------------------------------------------------------------

