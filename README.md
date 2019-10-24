# CppCSVParser
C++ implementation of CSV parser

Usage, 

  CSVParser objLixParser;
	stCsvParser *csvparser = objParser.MakeNewCSVParserFromStream(_zCSVString.c_str(), ",", 0);
	stCsvRow *pstCsvRow;

	typedef std::vector<std::string> Row;
	std::vector<Row> vecDecodedData;
	while ((pstCsvRow = objParser.GetRow(csvparser)))
	{
		printf("==NEW LINE==\n");
		const char **rowFields = objParser.GetFields(pstCsvRow);
		std::vector<std::string> row;
		for (int i = 0; i < objParser.GetNumberOfFields(pstCsvRow); i++)
		{
			printf("FIELD: %s\n", rowFields[i]);
			row.push_back(rowFields[i]);
		}
		vecDecodedData.push_back(row);
		objParser.DestoryCSVParserRow(pstCsvRow);
	}
	objParser.DestoryCSVParser(csvparser);

	// check whether we have more than two records
	if (vecDecodedData.size() >= 2)
	{
  // process your own data based on structure.
  }
