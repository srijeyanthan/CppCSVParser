#include <stdio.h>

typedef struct stCsvRow
{
	char **m_pzCSVFields;
	int m_iNoOfFields;
} stCsvRow;

typedef struct stCsvParser
{
	char *m_pzFilePath;
	char m_zDelimeter;
	int m_iIsFirstLineIsHeader;
	char *m_pzErrorMsg;
	stCsvRow *m_pstHeaderRow;
	FILE *m_pFileHandler;
	int m_iFromString;
	char *m_pzCSVString;
	int m_iStringIter;
} stCsvParser;
class CSVParser
{
public:
	CSVParser();
	virtual ~CSVParser();
	stCsvParser *MakeNewCSVParser(const char *filePath, const char *delimiter, int firstLineIsHeader);
	stCsvParser *MakeNewCSVParserFromStream(const char *csvString, const char *delimiter, int firstLineIsHeader);
	void DestoryCSVParser(stCsvParser *csvParser);
	void DestoryCSVParserRow(stCsvRow *csvRow);
	const stCsvRow *GetHeader(stCsvParser *csvParser);
	stCsvRow *GetRow(stCsvParser *csvParser);
	int GetNumberOfFields(const stCsvRow *csvRow);
	const char **GetFields(const stCsvRow *csvRow);
	const char* GetErrorMessage(stCsvParser *csvParser);
private:
	stCsvRow *GetInternalRow(stCsvParser *csvParser);
	int IsdelimiterIsAccepted(const char *delimiter);
	void SetErrorMessage(stCsvParser *csvParser, const char *errorMessage);

};
