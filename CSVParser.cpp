#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "CSVParser.h"

CSVParser::CSVParser()
{

}
stCSVParser *CSVParser::MakeNewCSVParser(const char *filePath, const char *delimiter, int firstLineIsHeader)
{
	stCSVParser *csvParser = (stCSVParser*) malloc(sizeof(stCSVParser));
	if (filePath == NULL)
	{
		csvParser->m_pzFilePath = NULL;
	}
	else
	{
		int filePathLen = strlen(filePath);
		csvParser->m_pzFilePath = (char*) malloc((filePathLen + 1));
		strcpy(csvParser->m_pzFilePath, filePath);
	}
	csvParser->m_iIsFirstLineIsHeader = firstLineIsHeader;
	csvParser->m_pzErrorMsg = NULL;
	if (delimiter == NULL)
	{
		csvParser->m_zDelimeter = ',';
	}
	else if (IsdelimiterIsAccepted(delimiter))
	{
		csvParser->m_zDelimeter = *delimiter;
	}
	else
	{
		csvParser->m_zDelimeter = '\0';
	}
	csvParser->m_pstHeaderRow = NULL;
	csvParser->m_pFileHandler = NULL;
	csvParser->m_iFromString = 0;
	csvParser->m_pzCSVString = NULL;
	csvParser->m_iStringIter = 0;

	return csvParser;
}

CSVParser::~CSVParser()
{

}
stCSVParser *CSVParser::MakeNewCSVParserFromStream(const char *csvString, const char *delimiter, int firstLineIsHeader)
{
	stCSVParser *csvParser = MakeNewCSVParser(NULL, delimiter, firstLineIsHeader);
	csvParser->m_iFromString = 1;
	if (csvString != NULL)
	{
		int csvStringLen = strlen(csvString);
		csvParser->m_pzCSVString = (char*) malloc(csvStringLen + 1);
		strcpy(csvParser->m_pzCSVString, csvString);
	}
	return csvParser;
}

void CSVParser::DestoryCSVParser(stCSVParser *csvParser)
{
	if (csvParser == NULL)
	{
		return;
	}
	if (csvParser->m_pzFilePath != NULL)
	{
		free(csvParser->m_pzFilePath);
	}
	if (csvParser->m_pzErrorMsg != NULL)
	{
		free(csvParser->m_pzErrorMsg);
	}
	if (csvParser->m_pFileHandler != NULL)
	{
		fclose(csvParser->m_pFileHandler);
	}
	if (csvParser->m_pstHeaderRow != NULL)
	{
		DestoryCSVParserRow(csvParser->m_pstHeaderRow);
	}
	if (csvParser->m_pzCSVString != NULL)
	{
		free(csvParser->m_pzCSVString);
	}
	free(csvParser);
}

void CSVParser::DestoryCSVParserRow(stLixCsvRow *csvRow)
{
	int i;
	for (i = 0; i < csvRow->m_iNoOfFields; i++)
	{
		free(csvRow->m_pzCSVFields[i]);
	}
	free(csvRow->m_pzCSVFields);
	free(csvRow);
}

const stLixCsvRow *CSVParser::GetHeader(stCSVParser *csvParser)
{
	if (!csvParser->m_iIsFirstLineIsHeader)
	{
		SetErrorMessage(csvParser, "Cannot supply header, as current CsvParser object does not support header");
		return NULL;
	}
	if (csvParser->m_pstHeaderRow == NULL)
	{
		csvParser->m_pstHeaderRow = GetInternalRow(csvParser);
	}
	return csvParser->m_pstHeaderRow;
}

stLixCsvRow *CSVParser::GetRow(stCSVParser *csvParser)
{
	if (csvParser->m_iIsFirstLineIsHeader && csvParser->m_pstHeaderRow == NULL)
	{
		csvParser->m_pstHeaderRow = GetInternalRow(csvParser);
	}
	return GetInternalRow(csvParser);
}

int CSVParser::GetNumberOfFields(const stLixCsvRow *csvRow)
{
	return csvRow->m_iNoOfFields;
}

const char **CSVParser::GetFields(const stLixCsvRow *csvRow)
{
	return (const char**) csvRow->m_pzCSVFields;
}

stLixCsvRow *CSVParser::GetInternalRow(stCSVParser *csvParser)
{
	int numRowRealloc = 0;
	int acceptedFields = 64;
	int acceptedCharsInField = 64;
	if (csvParser->m_pzFilePath == NULL && (!csvParser->m_iFromString))
	{
		SetErrorMessage(csvParser, "Supplied CSV file path is NULL");
		return NULL;
	}
	if (csvParser->m_pzCSVString == NULL && csvParser->m_iFromString)
	{
		SetErrorMessage(csvParser, "Supplied CSV string is NULL");
		return NULL;
	}
	if (csvParser->m_zDelimeter == '\0')
	{
		SetErrorMessage(csvParser, "Supplied delimiter is not supported");
		return NULL;
	}
	if (!csvParser->m_iFromString)
	{
		if (csvParser->m_pFileHandler == NULL)
		{
			csvParser->m_pFileHandler = fopen(csvParser->m_pzFilePath, "r");
			if (csvParser->m_pFileHandler == NULL)
			{
				int errorNum = errno;
				const char *errStr = strerror(errorNum);
				char *errMsg = (char*) malloc(1024 + strlen(errStr));
				strcpy(errMsg, "");
				sprintf(errMsg, "Error opening CSV file for reading: %s : %s", csvParser->m_pzFilePath, errStr);
				SetErrorMessage(csvParser, errMsg);
				free(errMsg);
				return NULL;
			}
		}
	}
	stLixCsvRow *csvRow = (stLixCsvRow*) malloc(sizeof(stLixCsvRow));
	csvRow->m_pzCSVFields = (char**) malloc(acceptedFields * sizeof(char*));
	csvRow->m_iNoOfFields = 0;
	int fieldIter = 0;
	char *currField = (char*) malloc(acceptedCharsInField);
	int inside_complex_field = 0;
	int currFieldCharIter = 0;
	int seriesOfQuotesLength = 0;
	int lastCharIsQuote = 0;
	int isEndOfFile = 0;
	while (1)
	{
		char currChar = (csvParser->m_iFromString) ? csvParser->m_pzCSVString[csvParser->m_iStringIter] : fgetc(csvParser->m_pFileHandler);
		csvParser->m_iStringIter++;
		int endOfFileIndicator;
		if (csvParser->m_iFromString)
		{
			endOfFileIndicator = (currChar == '\0');
		}
		else
		{
			endOfFileIndicator = feof(csvParser->m_pFileHandler);
		}
		if (endOfFileIndicator)
		{
			if (currFieldCharIter == 0 && fieldIter == 0)
			{
				SetErrorMessage(csvParser, "Reached EOF");
				free(currField);
				DestoryCSVParserRow(csvRow);
				return NULL;
			}
			currChar = '\n';
			isEndOfFile = 1;
		}
		if (currChar == '\r')
		{
			continue;
		}
		if (currFieldCharIter == 0 && !lastCharIsQuote)
		{
			if (currChar == '\"')
			{
				inside_complex_field = 1;
				lastCharIsQuote = 1;
				continue;
			}
		}
		else if (currChar == '\"')
		{
			seriesOfQuotesLength++;
			inside_complex_field = (seriesOfQuotesLength % 2 == 0);
			if (inside_complex_field)
			{
				currFieldCharIter--;
			}
		}
		else
		{
			seriesOfQuotesLength = 0;
		}
		if (isEndOfFile || ((currChar == csvParser->m_zDelimeter || currChar == '\n') && !inside_complex_field))
		{
			currField[lastCharIsQuote ? currFieldCharIter - 1 : currFieldCharIter] = '\0';
			csvRow->m_pzCSVFields[fieldIter] = (char*) malloc(currFieldCharIter + 1);
			strcpy(csvRow->m_pzCSVFields[fieldIter], currField);
			free(currField);
			csvRow->m_iNoOfFields++;
			if (currChar == '\n')
			{
				return csvRow;
			}
			if (csvRow->m_iNoOfFields != 0 && csvRow->m_iNoOfFields % acceptedFields == 0)
			{
				csvRow->m_pzCSVFields = (char**) realloc(csvRow->m_pzCSVFields, ((numRowRealloc + 2) * acceptedFields) * sizeof(char*));
				numRowRealloc++;
			}
			acceptedCharsInField = 64;
			currField = (char*) malloc(acceptedCharsInField);
			currFieldCharIter = 0;
			fieldIter++;
			inside_complex_field = 0;
		}
		else
		{
			currField[currFieldCharIter] = currChar;
			currFieldCharIter++;
			if (currFieldCharIter == acceptedCharsInField - 1)
			{
				acceptedCharsInField *= 2;
				currField = (char*) realloc(currField, acceptedCharsInField);
			}
		}
		lastCharIsQuote = (currChar == '\"') ? 1 : 0;
	}
}

int CSVParser::IsdelimiterIsAccepted(const char *delimiter)
{
	char actualDelimiter = *delimiter;
	if (actualDelimiter == '\n' || actualDelimiter == '\r' || actualDelimiter == '\0' || actualDelimiter == '\"')
	{
		return 0;
	}
	return 1;
}

void CSVParser::SetErrorMessage(stCSVParser *csvParser, const char *errorMessage)
{
	if (csvParser->m_pzErrorMsg != NULL)
	{
		free(csvParser->m_pzErrorMsg);
	}
	int errMsgLen = strlen(errorMessage);
	csvParser->m_pzErrorMsg = (char*) malloc(errMsgLen + 1);
	strcpy(csvParser->m_pzErrorMsg, errorMessage);
}

const char *CSVParser::GetErrorMessage(stCSVParser *csvParser)
{
	return csvParser->m_pzErrorMsg;
}
