//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.ValEdit.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include<map>
#include "SortMap.h"
//---------------------------------------------------------------------------
class SortMap;

class TMainWindow : public TForm
{
__published:	// IDE-managed Components
	TLabel *lbl1;
	TLabel *lbl2;
	TEdit *edtCurrentTime;
	TLabel *lbl5;
	TListView *lvFileListView;
	TEdit *edtAge;
	TEdit *edtSize;
	TLabel *lbl3;
	TButton *btnAutoDelete;
	TCheckBox *chkMaxAge;
	TCheckBox *chkMaxSize;
	TCheckBox *chkDeleteEmptiedCatalogs;
	TCheckBox *chkDeleteAllEmpty;
	TComboBox *cbbAge;
	TComboBox *cbbSize;
	TEdit *edtRepeatTime;
	TComboBox *cbbRepeatTime;
	TTimer *tmrAutoDelete;
	TCheckBox *chkRepeat;
	TTimer *tmrAutoRefresh;
	TButton *btnSaveConfig;
	TButton *btnLoadConfig;
	TCheckBox *chkImageDelete;
	TCheckBox *chkLogDelete;
	TCheckBox *chkTxtDelete;
	TLabel *lblcateogryOfFiles;
	TEdit *edtFileExtension;
	TCheckBox *chkOwnFileExtension;
	TCheckBox *chkViewFiles;
	TLabel *lblsize;
	TCheckBox *chkMaxDirSize;
	TEdit *edtMaxDirSize;
	TComboBox *cbbMaxDirSize;
	TEdit *edtProgramState;
	TCheckBox *chkAllFilesDelete;
	TComboBox *cbbSelectedPathsToDelete;
	TButton *btnAddPath;
	TButton *btnDeletePath;
	TMemo *mmo1;
	TEdit *edtPathsCounter;
	TComboBox *cbbSizeOfDir;
	TCheckBox *chkAutoDeleteOnStartup;
	TEdit *edtChosenDirSize;
	TComboBox *cbbChosenDirSize;
	TSaveDialog *SaveDialog1;
	void __fastcall btnAutoDeleteClick(TObject *Sender);
	void __fastcall chkMaxAgeClick(TObject *Sender);
	void __fastcall chkMaxSizeClick(TObject *Sender);
	void __fastcall tmrAutoDeleteTimer(TObject *Sender);
	void __fastcall chkRepeatClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall tmrAutoRefreshTimer(TObject *Sender);
	void __fastcall btnSaveConfigClick(TObject *Sender);
	void __fastcall btnLoadConfigClick(TObject *Sender);
	void __fastcall chkOwnFileExtensionClick(TObject *Sender);
	void __fastcall chkMaxDirSizeClick(TObject *Sender);
	void __fastcall chkViewFilesClick(TObject *Sender);
	void __fastcall chkAllFilesDeleteClick(TObject *Sender);
	void __fastcall btnAddPathClick(TObject *Sender);
	void __fastcall btnDeletePathClick(TObject *Sender);
	void __fastcall chkAutoDeleteOnStartupClick(TObject *Sender);
	//obs�uga tray'a
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall OnAppMinimize(TObject *Sender);
	void __fastcall trayIconClick(TObject *Sender);
	void __fastcall cbbSelectedPathsToDeleteChange(TObject *Sender);
	void __fastcall cbbChosenDirSizeChange(TObject *Sender);

private:	// User declarations
	TTrayIcon* trayIcon;
	void listCatalogs(const String &path, UINT &iterator);
	void fillMaps(const String &path, UINT &it);
	void deleteFiles(const String &path, UINT &it);
	void deleteFilesByDirSize(const String &path, UINT &it);
	void addElementToListView(const String &path, const TSearchRec &sr, UINT &iterator);
	bool deleteCheck(const char * fileNameChar);
	double getAgeOption();
	double getSizeOption();
	double getMaxDirSizeOption();
	double getChosenDirSizeOption();
	double getRepeadTime();
	double getUserSize();
	double getUserAge();
	double getUserRepeatTime();
	bool deleteExceptions(const String &path);
	void sendToTxt(const String &text);
	void saveConfigToIni();
	void saveLogToTxt(const String &pathOfFolder);
	void loadConfigFromIni();
	void createListView();
	void refreshComponents();
	void programState(const String &progState, System::Uitypes::TColor color);
	void fillVectorOfExtensions(const AnsiString &tmp);
	void fillVectorOfPaths();
	void refreshListOfFiles();
	bool checkDot(String input);
	String deleteExtFromFile(String &input);
	void deleteError(const String &pathOfFile);
	void extensionApply(bool flag);
	void fillCbbSizeOfDir();
	void clearMaps();
	void deleteFilesFunction();
	bool checkIfUserDir(AnsiString &path);

	//zmienne globalne
	bool buttonChangeFlag;
	bool selectFlag;
	bool loadCfgFlag;
	bool startFlag;
	int fileCounterLog;
	bool catalogDeletedFlag;
	String reasonLog;
	double numberOfFiles;
	double maxAgeOfFile;
	unsigned long sizeOfDir;
	bool buttonApplyExtensionFlag;
	vector<AnsiString>vecOfExtensions;
	vector<AnsiString>vecOfPaths;
	vector<AnsiString>vecOfDirSizes;
	vector<SortMap> vecOfFilesMaps;
	vector<SortMap> vecOfFoldersMaps;
	int iterator;
	int globalIteratorAddClick;

public:		// User declarations
	__fastcall TMainWindow(TComponent* Owner);
	__fastcall ~TMainWindow(); //definicja destruktora
};
//---------------------------------------------------------------------------
extern PACKAGE TMainWindow *MainWindow;
//---------------------------------------------------------------------------
#endif
