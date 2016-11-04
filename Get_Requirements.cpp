/// @file
#include "cin.h"
#include "Req_Info.h"

#pragma warning( push )
#pragma warning( disable : 4042 )		// Disable Warning ('identifier' : has bad storage class)

extern "C" CIN MgErr CINLoad(RsrcFile rf) 
{
	TString R_IN = TString();
	TStringArray CacheName = TStringArray();	// Cache Names With Conditions Met
	TStringArray CacheNamed = TStringArray();  // Cache Names from File
	TStringArray CacheValue = TStringArray();
	TStringArray Parsed_ValueCache = TStringArray();
	return noErr;
}

extern "C" CIN MgErr CINUnload(void)
{
	TString R_IN = TString();
	TStringArray CacheName = TStringArray();	// Cache Names With Conditions Met
	TStringArray CacheNamed = TStringArray();  // Cache Names from File
	TStringArray CacheValue = TStringArray();
	TStringArray Parsed_ValueCache = TStringArray();
	return noErr;
}

/// @addtogroup Utility
/// @{
/// @addtogroup ReplaceWide Replace Wide Characters
/// @{

/** @brief <B>ReplaceWide:\n</B>
   This function will reformat '&lt;' (for <) as ' lt '. \n
   It will reformat '&gt;' (for >) as ' gt '.\n

@param[in,out] error Error Status Information
@param[in] Data to have < and > replaced
@param[out] TString with < and > replaced
*/
TString ReplaceWide(ErrorCluster *error, TString Data)
{
	Data.Trim();

	if( error->Status())
		return TString();

	char *Buffer = new char[Data.Length() + 1];
	strcpy( Buffer, Data );
	Buffer[Data.Length()]='\0';
	int i, cnt;
	bool Wide = false;

	for( i = cnt = 0; Buffer[i] != '\0'; i++ )	
	{
		char c = Buffer[i];

		if( c == '&')				// if & it is Wide and & will be replaced with ' '
		{
			Buffer[cnt++] = ' ';
			Wide = true;
		}
		else if( Wide && c == ';')
		{
			Buffer[cnt++] = ' ';		// if Wide replace ';' with ' '	
			Wide = false;				// End Wide
		}
		else
			Buffer[cnt++] = c;			// Add char
	}

	Buffer[cnt] = '\0';					// mark end of string

	TString TData = Buffer;

	delete [] Buffer;
	return TData;

}
/// @}
/// @}

/// @addtogroup Utility
/// @{
/// @addtogroup RemoveSpace Remove White Space
/// @{

/** @brief <B>RemoveSpace:\n</B>
   This function will remove white space from the string handed in. 

@param[in,out] error Error Status Information
@param[in] String with spaces
@param[out] TString with spaces removed
*/
TString RemoveSpace( ErrorCluster *error, TString String )
{
	if( error->Status())
		return TString();

	String.Trim();

	char *Buffer = new char[String.Length() + 1];
	strcpy( Buffer, String );
	Buffer[String.Length()]='\0';
	int i, cnt;

	for( i = cnt = 0; Buffer[i] != '\0'; i++ )	// Remove mulitple Spaces, remove comments
	{
		char c = Buffer[i];

		if( !isspace(c) )						// if not a space
			Buffer[cnt++] = c;					// Add char
	}

	Buffer[cnt] = '\0';							// mark end of string

	TString TData = Buffer;

	delete [] Buffer;
	return ReplaceWide(error, TData);
}
/// @}
/// @}

/// @addtogroup Utility
/// @{
/// @addtogroup ParseString Parse String
/// @{

/** @brief <B>ParseString:\n</B>
   This function will parse a string and return an array with parameters separated by way\n
   of ':', ';' and ','. \n

@param[in,out] error Error Status Information
@param[in] String with ':', ';' and ','
@param[out] TStringArray elements separated by ':', ';' and ','.
*/
TStringArray ParseString( ErrorCluster *error, TString String )
{
	String.Trim();

	if( error->Status() || String.IsEmpty() )
		return TStringArray();

	TString Temp = String;

	if(error->Status())
		return TStringArray();
	
	TStringArray Items;
	Items.Initialize("\0",0);

	char *Rd = new char[Temp.Length() + 1];
	strcpy( Rd, Temp );
	Rd[Temp.Length()]='\0';

	char *token;
	token = strtok(Rd, SEPCHARS ); // Get the first token
		
	while( token != NULL )
	 {
	     Items.Insert(token, -1);
		 token = strtok( NULL, SEPCHARS ); // Get next token
	  }

	delete [] Rd; 
	
	return Items;
}
/// @}
/// @}

/// @addtogroup Utility
/// @{
/// @addtogroup LoadFile Load XML Requirements from File
/// @{

/** @brief <B>LoadFile:\n</B>
   This function will load the requirement information (XML) file. 

@param[in,out] error Error Status Information
@param[in] Path to Requirement information to be loaded
@param[out] TString data contained in file
*/
TString LoadFile(ErrorCluster *error, const TString &Path)
{
	if (error->Status())			// if error in, bail out
		return TString();

	int32 FileSize;				// Total File size
	char *FileBuffer = NULL;	// Buffer containing raw (unparsed) lines

	FILE *in = fopen( Path, "r" );	// open file (read only)
	if( in == NULL )      
    {
		error->SetError(-1, "Unable to open Requirements File '%s'", Path.CStr() );
		return TString();
	}

	if( (FileSize =	_filelength(_fileno(in)) ) <= 0 )	// Get file length
	{
		error->SetError(6, "Req_Info: Unable to read file '%s'", Path.CStr());
		fclose (in);
		return TString();
	}
	else
	{
		if( (FileBuffer = new char[FileSize+3]) == NULL )	// allocate memory for complete file
			error->SetError( 61, "Req_Info: Load File - Insufficient memory available");
		else
		{
			size_t BytesRead = fread( FileBuffer, sizeof(char), FileSize, in );	// read complete file
			FileBuffer[ BytesRead ] = '\0';	// Tag the end
			FileBuffer[ BytesRead+1 ] = '\0';
		}

		if( ferror(in) != 0 )	// check for file error
			error->SetError(6, "Req_Info: Error reading file");

		fclose(in);				// Close file
		TString TData = RemoveSpace( error, FileBuffer);

		delete [] FileBuffer;
		FileBuffer = NULL;
		
		if(error->Status())
			return TString();

		return TData;
	}

}
/// @}
/// @}

/// @addtogroup Utility
/// @{
/// @addtogroup BTWN_TAGS Get Value Between XML TAGS
/// @{

/** @brief <B>BTWN_TAGS:\n</B>
   This function will return data that is between 2 (XML) tags. 

@param[in,out] error Error Status Information
@param[in] Start leading tag
@param[in] Stop ending tag
@param[in] Data to search
@param[out] TString string between Start and Stop tags
*/
TString BTWN_TAGS(ErrorCluster *error, const TString &Start, const TString &Stop, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	if(Start.IsEmpty() || Stop.IsEmpty())
	{
		error->SetError(-1,"BTWN_TAGS- Tag missing");
		return TString();
	}

	int result_A = 0;
	int result_B = 0;
	char *pdest;
	char *Stop_Sect;

	if(Data.IsEmpty())
	{
		error->SetError(-1, "BTWN_TAGS - No data to parse.");
		return TString();
	}

	char *Rd = new char[Data.Length()+1];
	strcpy( Rd, Data );
	Rd[Data.Length()]='\0';

	pdest = strstr(Rd, Start.CStr()); // Look for start of block
		
	if(pdest != NULL) 
	{
		int result_A = pdest - Rd + Start.Length();

		Stop_Sect = strstr(pdest, Stop.CStr());	// Get Section

		if(Stop_Sect != NULL)
		{
			result_B = Stop_Sect - pdest - Stop.Length() + 1;
				
			if(result_B > 0)
			{
				delete [] Rd;
				return Data.Mid(result_A, result_B); // Return data between Tags
			}
			else
			{
				delete [] Rd;
				return TString();
			}
			
		}
	}
										
	if( Rd != NULL)
		delete [] Rd;

	return TString();

}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup ReqmtString Get Requirements String from XML
/// @{

/** @brief <B>ReqmtString:\n</B>
   This function returns the value between <Requirements> and </Requirements>. 

@param[in,out] error Error Status Information
@param[in] Data to be searched
@param[out] TString string between <Requirements> and </Requirements>.
*/
TString ReqmtString(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();
	
	TString DataOut = TString();
	TString Temp = BTWN_TAGS(error, "<Requirements>", "</Requirements>", Data);

	if(!error->Status())
	{
		DataOut = TString("<Requirements>\n");
		DataOut+=Temp;
		DataOut+= TString("\n</Requirements>\n");
	}

	return DataOut;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_Reqmts Parse Requirements from XML
/// @{

/** @brief <B>Parse_Reqmts:\n</B>
   This function will return an array of requirement sections

@param[in,out] error Error Status Information
@param[in] Data requirements string to be processed
@param[out] TStringArray each element is a requirement
*/
TStringArray Parse_Reqmts(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TStringArray();

	TStringArray TData;
	TString StrData;
	TData.Initialize("\0",0);
	char *pdest;
	int result = 0;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Reqmts - No data to parse.");
		
	if(!error->Status())
	{
		StrData = BTWN_TAGS(error, "<Requirement_element>", "</Requirement_element>", Data);
		if(!error->Status() && !StrData.IsEmpty())
		do
		{
			TData.Insert(StrData);		// Data found, Store it	
			pdest = strstr(Data.CStr(), StrData.CStr() ); // Look for start of block
		
			if(pdest != NULL) 
			{
				result = pdest - Data.CStr() + StrData.Length();			
				StrData = BTWN_TAGS(error, "<Requirement_element>", "</Requirement_element>", Data.Mid(result,-1));
			}
										
		}while(!error->Status() && pdest != NULL && !StrData.IsEmpty());

		if(!error->Status() && TData.GetSize() == 0)
			error->SetError( -1, "Parse_Reqmts- Error Parsing File.");
	}

	return TData;
}
/// @}
/// @}

/// @addtogroup ValueCache
/// @{
/// @addtogroup Parse_ValueCache Parse Value Cache from XML
/// @{

/** @brief <B>Parse_ValueCache:\n</B>
	This function will return an array of cached values from the requirements sections\n 
	This function will not error if the section is not found.\n

@param[in,out] error Error Status Information
@param[in] Data value cache string to be processed
@param[out] TStringArray each element contains a 'cache' value as such [Name][Condition][Name][Condition]
*/
TStringArray Parse_ValueCache(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TStringArray();

	TStringArray TData;
	TStringArray TempData;
	TString StrData;
	TData.Initialize("\0",0);
	TempData.Initialize("\0",0);
	char *pdest;
	int result = 0;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_ValueCache - No data to parse.");
	
	if(!error->Status())
	{
		StrData = BTWN_TAGS(error, "<Cache_Element>", "</Cache_Element>", Data);
		
		if(!error->Status() && !StrData.IsEmpty())
		do
		{
			TempData.Insert(StrData);		// Data found, Store it	
			pdest = strstr(Data.CStr(), StrData.CStr() ); // Look for start of block

			if(pdest != NULL) 
			{
				result = pdest - Data.CStr() + StrData.Length();			
				StrData = BTWN_TAGS(error, "<Cache_Element>", "</Cache_Element>", Data.Mid(result,-1));

			}
										
		}while(pdest != NULL && !StrData.IsEmpty());

		if(!error->Status() && TempData.GetSize() > 0) // Sort array: [Name][Condition][Name][Condition]
		{
			for( int a = 0; !error->Status() && a < TempData.GetSize(); a++)
			{
				TData.Insert(BTWN_TAGS(error, "<name>", "</name>", TempData[a]));
				TStringArray Named = ParseString(error, BTWN_TAGS(error, "<name>", "</name>", TempData[a]));
				CacheNamed.Insert(Named[0]);
				TData.Insert(BTWN_TAGS(error, "<Condition>", "</Condition>", TempData[a]));
			}
		}

		/* Don't error if Section isn't Found */
	}

	return TData;
}
/// @}
/// @}

/// @addtogroup ValueCache
/// @{
/// @addtogroup IsCacheNamed Is Named In Cache
/// @{

/** @brief <B>IsCacheNamed:\n</B>
	This function will return a True or False determined if the TString sent is\n 
	a named cache element\n

@param[in,out] error Error Status Information
@param[in] Data value cache string to be tested
@param[out] bool True of false based on if the Data in is named in cache file
*/
bool IsCacheNamed(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return false;
	
	bool Test = false;

	if(Data.Length() == 0)
		error->SetError( -1,"IsCacheNamed: No data to process.");
	
	if(!error->Status() && CacheNamed.GetSize() == 0)
		return false;

	if(!error->Status())
	{
		for( int a = 0; !Test && a < CacheNamed.GetSize(); a++)
			Test = stricmp(CacheNamed[a].CStr(), Data.CStr()) == 0;
	}

	return Test;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_Cond_Limits Parse Condition Limits from XML
/// @{

/** @brief <B>Parse_Cond_Limits:\n</B>
	This function will return an array of Condition Limit sections.\n

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TStringArray each elemement is a condition section
*/
TStringArray Parse_Cond_Limits(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TStringArray();

	TStringArray TData;
	TString StrData;
	TData.Initialize("\0",0);
	char *pdest;
	int result = 0;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Cond_Limits - No data to parse.");
		
	if(!error->Status())
	{
		StrData = BTWN_TAGS(error, "<Conditions>", "</Conditions>", Data);
	
		if(!error->Status() && !StrData.IsEmpty())
		do
		{
			TData.Insert(StrData);		// Data found, Store it	
			pdest = strstr(Data.CStr(), StrData.CStr() ); // Look for start of block
		
			if(pdest != NULL) 
			{
				result = pdest - Data.CStr() + StrData.Length();			
				StrData = BTWN_TAGS(error, "<Conditions>", "</Conditions>", Data.Mid(result,-1));
			}
										
		}while(pdest != NULL && !StrData.IsEmpty());

		if(TData.GetSize() == 0)
			error->SetError( -1, "Parse_Cond_Limits- Error Parsing File.");
	}

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_OneID Parse One Requirement ID from XML
/// @{

/** @brief <B>Parse_OneID:\n</B>
	This function will return a Test Parameter selection.

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <ReqID> and </ReqID> tags
*/
TString Parse_OneID(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_OneID - No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<ReqID>", "</ReqID>", Data);
	
	if(TData.IsEmpty())
		error->SetError(-1, "Parse_OneID - Error Parsing File.");

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_ID Parse Requirement ID Section from XML
/// @{

/** @brief <B>Parse_ID:\n</B>
	This function will return a Requirement ID selection from the Vector.

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TStringArray each element is a selection parsed by way of <ReqID> and </ReqID>
*/
TStringArray Parse_ID(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TStringArray();

	TStringArray TData;
	TString StrData;
	TData.Initialize("\0",0);
	char *pdest;
	int result = 0;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_ReqID - No data to parse.");

	TString DataIn = Data;
		
	if(!error->Status())
	{
		StrData = BTWN_TAGS(error, "<ReqID>", "</ReqID>", Data);
	
		if(!error->Status() && !StrData.IsEmpty())
		do
		{
			TData.Insert(StrData);		// Data found, Store it	
			pdest = strstr(DataIn.CStr(), StrData.CStr() ); // Look for start of block
		
			if(pdest != NULL) 
			{
				result = pdest - DataIn.CStr() + StrData.Length();			
				StrData = BTWN_TAGS(error, "<ReqID>", "</ReqID>", DataIn.Mid(result,-1));
				DataIn = DataIn.Mid(result,-1);
			}
										
		}while(!error->Status() && pdest != NULL && !StrData.IsEmpty());

		// Don't return error if none found may be older config or characterization run
		if(TData.GetSize() == 0)
			return TStringArray();
	}

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_ParameterType Parse Parameter Type from XML
/// @{

/** @brief <B>Parse_ParameterType:\n</B>
	This function will return a Parameter Type selection.\n

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <Parameter_Type> and </Parameter_Type> tags
*/
TString Parse_ParameterType(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Parameter - No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<Parameter_Type>", "</Parameter_Type>", Data);
	
	if(TData.IsEmpty())
		error->SetError(-1, "Parse_Parameter - Error Parsing File.");

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_TestParameter Parse Test Parameter from XML
/// @{

/** @brief <B>Parse_TestParameter:\n</B>
	This function will return a Test Parameter selection.

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <Test_Parameter> and </Test_Parameter> tags
*/
TString Parse_TestParameter(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_TestParameter: No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<Test_Parameter>", "</Test_Parameter>", Data);
	
	if(TData.IsEmpty())
		error->SetError(-1, "Parse_TestParameter: Error Parsing File.");

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_Unit Parse Unit from XML
/// @{

/** @brief <B>Parse_Unit:\n</B>
	This function will return a Unit selection.\n

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <units> and </units> tags
*/
TString Parse_Unit(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Unit - No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<units>", "</units>", Data);
	
	if(TData.IsEmpty())
		error->SetError(-1, "Parse_Unit - Error Parsing File.");

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_Condition Parse Condition Section from XML
/// @{

/** @brief <B>Parse_Condition:\n</B>
	This function will return a Condition selection.\n

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <Condition> and </Condition> tags
*/
TString Parse_Condition(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Condition - No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<Condition>", "</Condition>", Data);
	
	if(TData.IsEmpty())
		error->SetError(-1, "Parse_Condition - Error Parsing File.");

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_Max Parse Maximum Limit from XML
/// @{

/** @brief <B>Parse_Max:\n</B>
	This function will return the Max Limit.  It allows for an empty value.

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <max> and </max> tags
*/
TString Parse_Max(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Max - No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<max>", "</max>", Data);
	
	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_Nom Parse Nominal Value from XML
/// @{

/** @brief <B>Parse_Nom:\n</B>
	 This function will return the Nom value.  It will allow for\n
	 the section to be empty to cover absolute values in the Min and\n
	 Max conditions.  If a value is in the section, the Min and Max\n
	 values will be represented with respect of this value.\n\n  

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <nom> and </nom> tags
*/
TString Parse_Nom(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Nom - No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<nom>", "</nom>", Data);

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_Min Parse Minimal Value from XML
/// @{

/** @brief <B>Parse_Min:\n</B>
	 This function will return the Min Limit. It allows for an empty value.  

@param[in,out] error Error Status Information
@param[in] Data to be processed
@param[out] TString data between <min> and </min> tags
*/
TString Parse_Min(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TString();

	TString TData;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_Min - No data to parse.");
		
	if(!error->Status())
		TData = BTWN_TAGS(error, "<min>", "</min>", Data);
	
	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_vSections Parse Vector Sections
/// @{

/** @brief <B>Parse_vSections:\n</B>
	 This function will return an array of Vector sections.  

@param[in,out]  error Error Status Information
@param[in] Data to be processed
@param[out] TStringArray each element is a selection parsed by way of <vector_element> and </vector_element>
*/
TStringArray Parse_vSections(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TStringArray();

	TStringArray TData;
	TString StrData;
	TData.Initialize("\0",0);
	char *pdest;
	int result = 0;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_vSections - No data to parse.");

	TString DataIn = Data;
		
	if(!error->Status())
	{
		StrData = BTWN_TAGS(error, "<vector_element>", "</vector_element>", Data);
		if(!error->Status() && !StrData.IsEmpty())
		do
		{
			TData.Insert(StrData);		// Data found, Store it	
			pdest = strstr(DataIn.CStr(), StrData.CStr() ); // Look for start of block
		
			if(pdest != NULL) 
			{
				result = pdest - DataIn.CStr() + StrData.Length();			
				StrData = BTWN_TAGS(error, "<vector_element>", "</vector_element>", DataIn.Mid(result,-1));
				DataIn = DataIn.Mid(result,-1);
			}
										
		}while(pdest != NULL && pdest != NULL && !StrData.IsEmpty());

		if(TData.GetSize() == 0)
			error->SetError( -1, "Parse_vSections- Error Parsing File.'%s'.", Data.CStr());
	}

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_dSections Parse Data Sections
/// @{

/** @brief <B>Parse_dSections:\n</B>
	 This function will return an array of Data sections.  

@param[in,out]  error Error Status Information
@param[in] Data to be processed
@param[out] TStringArray each element is a selection parsed by way of <result_element> and </result_element>
*/
TStringArray Parse_dSections(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TStringArray();

	TStringArray TData;
	TString StrData;
	TData.Initialize("\0",0);
	char *pdest;
	int result = 0;

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_dSections - No data to parse.");

	TString DataIn = Data;
		
	if(!error->Status())
	{
		StrData = BTWN_TAGS(error, "<result_element>", "</result_element>", Data);

		if(!error->Status() && !StrData.IsEmpty())
		do
		{
			TData.Insert(StrData);		// Data found, Store it	
			pdest = strstr(DataIn.CStr(), StrData.CStr() ); // Look for start of block
		
			if(pdest != NULL) 
			{
				result = pdest - DataIn.CStr() + StrData.Length();			
				StrData = BTWN_TAGS(error, "<result_element>", "</result_element>", DataIn.Mid(result,-1));
				DataIn = DataIn.Mid(result,-1);
			}
										
		}while(pdest != NULL && pdest != NULL && !StrData.IsEmpty());

		if(TData.GetSize() == 0)
			error->SetError( -1, "Parse_dSections- Error Parsing File.");
	}

	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup Parse_vReqID Parse Vector Requirement ID
/// @{

/** @brief <B>Parse_vReqID:\n</B>
	 This function will return an array of Vector Requirement IDs.  It will also\n
	 remove NA and N/A from array of requirements.\n

@param[in,out]  error Error Status Information
@param[in] Data to be processed
@param[out] TStringArray each element is a selection parsed by way of <ReqID> and </ReqID>
*/
TStringArray Parse_vReqID(ErrorCluster *error, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return TStringArray();

	TStringArray TData;
	TString StrData;
	TData.Initialize("\0",0);
	int result = 0;
	TStringArray TempNAReq = TStringArray();	// Used to clean up NA or N/A in ReqID array

	if(Data.IsEmpty())
		error->SetError(-1, "Parse_vReqID - No data to parse.");
		
	if(!error->Status())
		TData = Parse_ID(error, Data); // Parse Vector ReqID

	if (error->Status())
		return TStringArray();
		
	// When vectors are combined a requirement of 'NA' or 'N/A' may be added to vectors
	// with actual requirements.  If this happens the following code should clean it up
	// so a hang up doesn't occur.
	for( int clean = 0; clean < TData.GetSize() && TData.GetSize() > 0; clean++)
	{
		if( stricmp(TData[clean].CStr(), "N/A") != 0 && stricmp(TData[clean].CStr(), "NA") != 0)
			TempNAReq.Insert(TData[clean]);
	}
		
	TData = TempNAReq; // Set to Cleaned NA and N/A Requirement ID array

	return TData;
}
/// @}
/// @}

/// @addtogroup Token_Tools
/// @{
/// @addtogroup IsCompToken Is This a Comparison Token
/// @{

/** @brief <B>IsCompToken:\n</B>
	 This is a basic comparison test and looks for ':', '=', and ' '.\n
	 A space is used with a wide character such as < and >.  '>' was \n
	 changed to ' gt ' and '<' was changed to ' lt '.  This function \n
	 returns a true if the character indicates a comparison.\n

@param[in,out] error Error Status Information
@param[in] Char character
@param[out] bool true or false
*/
bool IsCompToken(ErrorCluster *error, char Char)
{
	if(error->Status())
		return false;

	if( Char == ':' || Char == '=' )
		return true;
	else if( Char == ' ') // To be used with Wide Char such as >, <
		return true;	  //  > is changed to ' gt ', < is ' lt '	
	else
		return false;

}
/// @}
/// @}

/// @addtogroup Token_Tools
/// @{
/// @addtogroup IsMathToken Is This a Math Token
/// @{

/** @brief <B>IsMathToken:\n</B>
	 This is a basic math function test and looks for '(', ')', '^',\n
	 '*', '/', '+', '-', and '|'.  This function returns a true if \n
	 the character indicates a math function.\n

@param[in,out] error Error Status Information
@param[in] Char character
@param[out] bool true or false
*/
bool IsMathToken(ErrorCluster *error, char Char)
{
	if(error->Status())
		return false;

	if( Char == '(' || Char == ')' || Char == '^')
		return true;
	else if( Char == '*' || Char == '/' )
		return true;
	else if( Char == '+' || Char == '-' )
		return true;
	else if( Char == '|')
		return true;
	else
		return false;

}
/// @}
/// @}

/// @addtogroup Token_Tools
/// @{
/// @addtogroup StrIsMathToken TString Is This a Math Token
/// @{

/** @brief <B>StrIsMathToken:\n</B>
	 This is a basic math function test of a TString. It \n
	 will first check the length of the string.  If the\n
	 length is greater than a character then it must not \n
	 be a math token.  If it is only one character long\n
	 IsMathToken will be used to test it.\n		

@param[in,out] error Error Status Information
@param[in] TString Data to Test
@param[out] bool true or false
*/
bool StrIsMathToken(ErrorCluster *error, TString Data)
{
	if(error->Status())
		return false;

	if(Data.Length() > 1 || Data.IsEmpty())
		return false;

	char *Rd = new char[Data.Length()];
	strcpy( Rd, Data );

	bool Test = IsMathToken(error, Rd[0]);

	delete [] Rd;

	return Test;
}
/// @}
/// @}

/// @addtogroup Token_Tools
/// @{
/// @addtogroup IsBadToken Is This a Bad Name or Value Token
/// @{

/** @brief <B>IsBadToken:\n</B>
	 This is a test indicating certain tokens that will need to be removed\n
	 from in 'Values' or 'Names' for math functions work correctly.  The\n
	 tokens that it looks for are: '*', '+' and '^'.  RemoveMathTokens calls\n
	 this function.\n 

@param[in,out] error Error Status Information
@param[in] Char character
@param[out] bool true or false
*/
bool IsBadToken(ErrorCluster *error, char Char)
{
	if(error->Status())
		return false;

	if( Char == '*' )
		return true;
	else if( Char == '+' || Char == '^' )
		return true;
	else
		return false;

}
/// @}
/// @}

/// @addtogroup Token_Tools
/// @{
/// @addtogroup RemoveMathTokens Remove Math Tokens
/// @{

/** @brief <B>RemoveMathTokens:\n</B>
	 This function will replace Math Type Chars in 'Values' or 'Names'\n
	 so math operations will not get confused. The characters removed are\n
	 listed in IsBadToken.\n

@param[in,out] error Error Status Information
@param[in] String to be cleaned up
@param[out] String with characters listed in IsBadToken removed.
*/
TString RemoveMathTokens(ErrorCluster *error, TString String)
{	
	if( error->Status())
		return TString();

	String.Trim();

	char *Buffer = new char[String.Length() + 1];
	strcpy( Buffer, String );
	int i, cnt;

	for( i = cnt = 0; Buffer[i] != '\0'; i++ )	
	{
		char c = Buffer[i];

		if(!IsBadToken(error, c))			// if not a math token
			Buffer[cnt++] = c;				// Add char
	}

	Buffer[cnt] = '\0';						// mark end of string

	TString TData = Buffer;

	delete [] Buffer;
	return TData;
}
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup GetValue Get Column Name Value
/// @{

/** @brief <B>GetValue:\n</B>
	 This function will return the value of the column name selected. 

@param[in,out] error Error Status Information
@param[in] VectorIn Vector XML string
@param[in] DataIn Data XML string
@param[in] ValueWant Column name for the value that is wanted
@param[out] Value_Struct contains Strings 'value' and 'type'
*/
Value_Struct GetValue(ErrorCluster *error, TString VectorIn, TString DataIn, TString ValueWant)
{
	Value_Struct VS;
	VS.Value = TString("0.00");
	VS.Type = TString("Unknown");

	if (error->Status())			// if error in, bail out
		return VS;

	bool In_V = false; // In Vector Test
	bool In_D = false; // In Data Test
	bool In_C = false; // In Cache Test

	TStringArray v_Vector = Parse_vSections(error, RemoveSpace(error, VectorIn));	// Get Vector Sections
	TStringArray v_Data = Parse_dSections(error, RemoveSpace(error, DataIn));	// Get Data Sections

	for( int b = 0; b < v_Vector.GetSize(); b++)	// Check Vector In for Item
	{
		TString Temp = BTWN_TAGS(error, "<name>", "</name>", v_Vector[b]); // Search Data for Test parameter
		if(stricmp(Temp.CStr(), ValueWant.CStr()) == 0)
		{
			VS.Type = BTWN_TAGS(error, "<type>", "</type>", v_Vector[b]);
			VS.Value = BTWN_TAGS(error, "<value>", "</value>", v_Vector[b]);
			In_V = true;
		}
	}
	
	for( b = 0; b < v_Data.GetSize(); b++)	// Check Data In for Item
	{
		TString Temp = BTWN_TAGS(error, "<name>", "</name>", v_Data[b]); // Search Data for Test parameter
		if(stricmp(Temp.CStr(), ValueWant.CStr()) == 0)
		{
			VS.Type = BTWN_TAGS(error, "<type>", "</type>", v_Data[b]);
			VS.Value = BTWN_TAGS(error, "<value>", "</value>", v_Data[b]);
			In_D = true;
		}
	}

	if( In_D && In_V)
		error->SetError( -1, "GetValue: Value of '%s' found in both Vector and Data", ValueWant.CStr());
	
	if( !error->Status() && !In_D && !In_V)	// Check Value Cache
	{
		for( b = 0; b < CacheName.GetSize()  && !In_C; b++)
		{
			if(stricmp(CacheName[b].CStr(), ValueWant.CStr()) == 0)
			{
				VS.Type = TString("float64");
				VS.Value = CacheValue[b];
				In_C = true;
			}
		}
	}
	
	if(!error->Status() && !In_C && !In_D && !In_V) // Let's see if it is a number
	{
		char  *stopstring;
		float64 Number = strtod( ValueWant.CStr(), &stopstring );
		if( *stopstring != NULL)
			error->SetError(-1, "GetValue: Value of '%s' can't be found in cache, vector or data.", ValueWant.CStr());
		else
		{
			VS.Type = TString("float64");
			VS.Value.Format("%.15e",Number);
		}
	}

	return VS;
}
////////////////// Math Functions //////////////////////////////////////////////////////////////////////
/// @}
/// @}

/// @addtogroup Parsers
/// @{
/// @addtogroup GetColumnNames Get Column Name Value or Number
/// @{

/** @brief <B>GetColumnNames:\n</B>
	This function will return the Column Names to be replaced with a number (dbl) value\n
    or it will return a Number.  Basically it returns all but the math operators. Another\n
	function will be used to convert the column name to a dbl.\n

@param[in,out] error Error Status Information
@param[in] String data to be processed
@param[in] All_Math bool indicating a formula is being processed
@param[out] TStringArray with column names change to their value and math operators removed
*/
TStringArray GetColumnNames(ErrorCluster *error, TString String, bool All_Math)
{
	if(error->Status())
		return TStringArray();

	if(String.IsEmpty())
	{
		error->SetError( -1, "GetColumnNames: No data to format.");
		return TStringArray();
	}

	TStringArray Items;
	Items.Initialize("\0",0);

	char *token;

	TString Temp = String;
	char *Rd = new char[Temp.Length() + 1];
	strcpy( Rd, Temp );
	Rd[Temp.Length()]='\0';

	if( All_Math) // This will parse out any math char found
	{
		token = strtok(Rd, SEPMath ); // Get the first token
	
		while( token != NULL )
		 {
		   Items.Insert(token, -1);	// This will be an array of Column names or Numbers
		   token = strtok( NULL, SEPMath ); // Get next token
		 }
	}
	else	// This will allow for the '^' or others to be in the name like 'sin^2 Delay'
	{
		token = strtok(Rd, SEPSome ); // Get the first token
	
		while( token != NULL )
		 {
			Items.Insert(token, -1);	// This will be an array of Column names or Numbers
			token = strtok( NULL, SEPSome ); // Get next token
		  }
	}

	delete [] Rd;
	
	return Items;

}
/// @}
/// @}

/// @addtogroup Conditions_Checking
/// @{
/// @addtogroup ParseConditionString Parse the Condition String
/// @{

/** @brief <B>ParseConditionString:\n</B>
	 This function returns an Array as follows:\n\n
	
	 [Vector Name],[Comp Value],[Data]\n\n

	 Comp Value is ':','<','>','=','<=','>='\n

@param[in,out] error Error Status Information
@param[in] String to be parsed
@param[out] TStringArray parsed data
*/
const TStringArray ParseConditionString( ErrorCluster *error, TString String)
{
	if(error->Status())
		return TStringArray();
	
	if(String.IsEmpty())
	{
		error->SetError(-1, "ParseConditionString - Empty Condition String.");
		return TStringArray();
	}

	TString Input = String;
	TStringArray Names = GetColumnNames( error, String, false ); // Allow ^ ( ) to be in the name
	
	if(error->Status())
		return TStringArray();

	TStringArray ParsedItems = TStringArray();
	int a = 0;
	
	char *Rd = new char[Input.Length() + 1];
	strcpy( Rd, Input );
	Rd[Input.Length()]='\0';

	char c = Rd[0];
	bool Test = false;
	int ff = 0;

	for( int b = 0; Rd != NULL && Rd != '\0' && a < Names.GetSize(); b++)
	 {
					 
		c = Rd[b];
		char *buffy = new char[4];
		
		if(!error->Status() && c == ';')
		{
			b++;
			c = Rd[b];
		}

		Test = IsCompToken(error, c);
	
		if(Test)
		{
			if(isspace(c)) // Must be wide Char
			 {
				 buffy[0] = Rd[b+1]; // jump ' ' get char
				 buffy[1] = Rd[b+2]; // get 2nd char
				 b+=4;				 // jump ' '
				 c = Rd[b];			 // check for =

				 Test = IsCompToken(error, c);
				
				 if(Test)
				 {
					 buffy[2] = c;	// if the first sybol was '<' or '>' then allow for '='
					 buffy[3] ='\0'; // terminate
				 }
				 else
				 {
					 buffy[2] = '\0'; // terminate
					 buffy[3] = '\0'; // terminate
					 b--;
				 }
			
			 }
			 else
			 {
				buffy[0] = c;
				buffy[1] = '\0'; // terminate
				buffy[2] = '\0'; // terminate
				buffy[3] = '\0'; // terminate
			 }
			 
			 ParsedItems.Insert(buffy,-1);
			 delete [] buffy;
			 buffy = NULL;
			 
		}		
		else 
		{
		 	if(stricmp(Names[a].CStr(), "lt") == 0 || stricmp(Names[a].CStr(), "gt") == 0)
				a++;
		
			ParsedItems.Insert(Names[a],-1);
		    b+= (int)(Names[a].Length() - 1);
			a++;
		 }
	
		if( buffy != NULL)
			delete [] buffy;
	 }

	if( ParsedItems.GetSize() == 0)
		error->SetError(-1, "ParseConditionString: Error parsing '%s'",String.CStr());
	
	delete [] Rd;

	return ParsedItems;

}
/// @}
/// @}

/// @addtogroup Conditions_Checking
/// @{
/// @addtogroup ConditionCk Condition Check
/// @{

/** @brief <B>ConditionCk:\n</B>
	This function will determine if the Vector Parameter Conditions Match the Requirement Conditions\n 
	This function expects conditions to be listed as follows:\n\n
			
		[Vector Column Name]PC[Column Value]; [Vector Column Name]PC[Column Value];\n\n

		PC = parse comparisons:\n
		:	this will do a string (stricmp) comparison\n
		=	from this point on the value will be typed to float and will be compared\n
		<\n
		>\n
		<=\n
		>=\n

@param[in,out] error Error Status Information
@param[in] VectorParameters vector or data xml to be compared to condition
@param[in] Condition being looked up
@param[out] bool true if condition matches
*/
CondCk_Struct ConditionCk(ErrorCluster *error, TStringArray VectorParameters, const TString &Condition)
{
	
	CondCk_Struct CC;
	CC.Match = false;
	CC.found = false;
	
	if(error->Status())
		return CC;

	//  The following line shortens the search.  Both formats are checked because most of the code
	//  counts on a ';' to terminate the command.  This could have been done better but it came 
	//  to be during the learning of how this code will have to function.
	if(stricmp(Condition.CStr(), "All") == 0 || stricmp(Condition.CStr(), "All;") == 0 || Condition.IsEmpty())
	{	
		CC.found = true;
		CC.Match = true;
		return CC;
	}

	TStringArray cName = TStringArray();	// Condition Name
	TStringArray cCompare = TStringArray();	// Condition Compare
	TStringArray cValue = TStringArray();	// Condition Value
	TString Value;
	CC.Match = true;
	CC.found = false;

	TStringArray ParsedTemp = ParseConditionString( error, Condition);

	if(error->Status())
	{	
		CC.found = false;
		CC.Match = false;
		return CC;
	}

	if(ParsedTemp.GetSize() < 3)
	{
		error->SetError(-1, "ConditionCk: Error Parsing Conditions of '%s'", Condition.CStr());
		return CC;
	}

	for(int a = 0; a < ParsedTemp.GetSize(); a+=3)
	{
		cName.Insert(ParsedTemp[a]);
		cCompare.Insert(ParsedTemp[a+1]);
		cValue.Insert(ParsedTemp[a+2]);	// ParseConditionString removed any ';' left by ReplaceWide
	}

	//	if !Match then the value was found but did not match
	for( a = 0; !error->Status() && a < cName.GetSize() && CC.Match; a++)
	{
		CC.Name = cName[a];
		CC.found = false;

		for( int b = 0; !error->Status() && b < VectorParameters.GetSize() && !CC.found; b++)
		{
			TString TempStr = BTWN_TAGS(error, "<name>", "</name>", VectorParameters[b]); // Search Vector for cName
	
			if(stricmp(TempStr.CStr(), cName[a].CStr()) == 0)
			{
				CC.found = true;
			
				Value = BTWN_TAGS(error, "<value>", "</value>", VectorParameters[b]);  // Found cName now check value
				
				if(stricmp(cCompare[a].CStr(),":") == 0)	// Then it is a stricmp
				{				
					CC.Match = stricmp( Value.CStr(), cValue[a].CStr()) == 0;
				}
				else	// Convert values to floats and compare them
				{
					char  *stopstring;
					float64 V;
					float64 cV;
	
					V = strtod( Value.CStr(), &stopstring );
				
					if( *stopstring != NULL)
						error->SetError(-1, "ConditionCk: Value of '%s' can't be converted to a number.", Value.CStr());

					cV = strtod( cValue[a].CStr(), &stopstring );

					if( !error->Status() && *stopstring != NULL)
						error->SetError(-1, "ConditionCk: Condition value of '%s' can't be converted to a number.", cValue[a].CStr());

					if(error->Status())
					{	
						CC.found = false;
						CC.Match = false;
						return CC;
					}
					
					if(stricmp(cCompare[a].CStr(),"=") == 0)
					{	
						CC.Match = cV == V;
					}
					else if(stricmp(cCompare[a].CStr(),"lt") == 0)
					{	
						CC.Match = V < cV;
					}
					else if(stricmp(cCompare[a].CStr(),"gt") == 0)
					{	
						CC.Match = V > cV;
					}
					else if(stricmp(cCompare[a].CStr(),"gt=") == 0)
					{	
						CC.Match = (V >= cV);
					}
					else if(stricmp(cCompare[a].CStr(),"lt=") == 0)
					{	
						CC.Match = (V <= cV);
					}
					else
					{
						error->SetError( -1, "ConditionCk: Compare Value of '%s' unKnown.", cCompare[a].CStr());
						CC.Match = false;
					}
									
				}// End Convert values to floats and compare them	
					
			} 
	
		} // End Vector Name Vs. Condition name if statement

		if(!CC.found)
			CC.Match = false;
	
	} // End for loop that searches for the names

	return CC;

}

/// @}
/// @}

/// @addtogroup Conditions_Checking
/// @{
/// @addtogroup ConditionCheck Condition Check Top Level
/// @{

/** @brief <B>ConditionCheck:\n</B>
	This function will determine if the Vector Parameter Conditions Match the Requirement Conditions\n 
	by first Merging the vector and data parameters then testing the data.\n\n


@param[in,out] error Error Status Information
@param[in] Vector xml to be compared to condition
@param[in] Data xml to be compared to condition
@param[in] Condition being looked up
@param[out] bool true if condition matches
*/
bool ConditionCheck(ErrorCluster *error, const TString &Vector, const TString &Data, const TString &Condition)
{
	if(error->Status())
		return false;

	CondCk_Struct Test;
	TStringArray All_Parameters;

	//  Parse the sections
	TStringArray TestVector = Parse_vSections(error, RemoveSpace(error, Vector));
	TStringArray TestData = Parse_dSections(error, RemoveSpace(error, Data));

	All_Parameters.Initialize("\0", TestVector.GetSize()+TestData.GetSize());

	// Merge Vector and Data elements to be searched
	for(int a = 0; a < TestVector.GetSize(); a++)
		All_Parameters[a] = TestVector[a];

	for(int b = 0; b < TestData.GetSize(); b++)
		All_Parameters[a+b] = TestData[b];

	// Test conditions
	Test = ConditionCk(error, All_Parameters, Condition);

	if(!error->Status() && !Test.found)
		error->SetError(-1, "ConditionCheck: Condition Name of '%s' not found.", Test.Name.CStr());
	
	return Test.Match;

}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup GetFormulaString Get Formula String
/// @{

/** @brief <B>GetFormulaString:\n</B>
   This function will return a TStringArray of the formula. \n  

@param[in,out] error Error Status Information
@param[in] String to convert to formula string array
@param[out] TStringArray with values and operations separated
*/
TStringArray GetFormulaString(ErrorCluster *error, TString String)
{
	if(error->Status())
		return TStringArray();

	if(String.IsEmpty())
		error->SetError( -1, "GetFormulaString: No data to format.");
	
	TString Input = String;
	TStringArray Names = GetColumnNames(error, Input, true); // Contains all but the Math Functions

	if(error->Status())
		return TStringArray();
	
	TStringArray Items;
	Items.Initialize("\0",0);
	int a = 0;
	
	char *Rd = new char[Input.Length() + 1];
	strcpy( Rd, Input );
	Rd[Input.Length()]='\0';
	char c = Rd[0];
	bool Test = false;

	for( int b = 0; Rd != NULL && Rd[b] != '\0'; b++)
	 {
	    char *buffy = new char[2];
		c = Rd[b];
	
		Test = IsMathToken(error, c);
		
		if(Test)
		{
			 buffy[0] = c;
			 buffy[1] = '\0';
			 Items.Insert(buffy,-1);
		}
		else if(a < Names.GetSize())
		{
			 Items.Insert(Names[a],-1);
			 b+=(int)Names[a].Length() - 1;
			 a++;
			 
		 }
		
		if(buffy != NULL)	
			delete [] buffy;
		
	  }

	if( Items.GetSize() == 0)
		error->SetError(-1, "GetFormulaString: Error parsing '%s'",String.CStr());

	if(Rd != NULL)
		delete [] Rd;

	return Items;

}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup NamesToNumber Column Names To Numbers
/// @{

/** @brief <B>NamesToNumber:\n</B>
	This function will replace column names with their value.\n

@param[in,out] error Error Status Information
@param[in] String element that is looked-up
@param[in] VectorIn Vector XML String
@param[in] DataIn Data XML String
@param[out] TStringArray containing value of data or vector column
*/
TStringArray NamesToNumber(ErrorCluster *error, TString String, TString VectorIn, TString DataIn)
{
	if(error->Status())
		return TStringArray();

	if(String.IsEmpty())
		error->SetError( -1, "NamesToNumber: No data to convert.");

	TString Input = String;
	TStringArray cNames = GetColumnNames(error, Input, true); // Contains all but the Math Functions
	TStringArray fNames = GetFormulaString(error, Input);	  // Contains formula with Column Names

	bool Match = false;

	if(error->Status())
		return TStringArray();

	for( int a = 0; !error->Status() && a < fNames.GetSize(); a++)
	{
		Match = false;
		for( int b = 0; !error->Status() && b < cNames.GetSize() && !Match; b++)
		{
			if(stricmp(cNames[b].CStr(), fNames[a].CStr()) == 0)
			{
				fNames[a] = GetValue(error, VectorIn, DataIn, cNames[b]).Value;
				Match = true;
			}

		}

	}

	return fNames;

}
/// @}
/// @}


/// @addtogroup Math
/// @{
/// @addtogroup RemoveNull Remove Null from Formula Array
/// @{

/** @brief <B>RemoveNull:\n</B>
	This function will replace NULL formula array values.  When a calculation of a\n
	formula string array is made on a section of that array, the first element of\n
	that array or the math operator is replaced with the value of the calculation \n
	and the rest of the sub-array is wiped out.  This keeps the size of the array \n
	intact until all math operations can be done reducing the number of steps in \n
	performing calculations.\n

@param[in,out] error Error Status Information
@param[in] Formula array with NULL elements
@param[out] TStringArray formula with Null elements removed
*/
TStringArray RemoveNull( ErrorCluster *error, TStringArray Formula)
{
	if(error->Status())
		return TStringArray();

	TStringArray Data;
	Data.Initialize("\0", 0);

	if( Formula.GetSize() < 1)
		return Data;

	for( int a = 0; a < Formula.GetSize(); a++)
	{
		if(stricmp(Formula[a].CStr(), "\0") !=0)
			Data.Insert(Formula[a], -1);

	}

	return Data;

}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Match Match Value
/// @{

/** @brief <B>Match:\n</B>
	This function will return an index array of Matches in the formula array.\n

@param[in,out] error Error Status Information
@param[in] formula TStringArray of the formula
@param[in] Find TString of the value that is being looked up
@param[out] uInt32Array of indices in the formula array that Find has been found.
*/
uInt32Array Match( ErrorCluster *error, TStringArray formula, TString Find)
{
	if(error->Status())
		return uInt32Array();

	if( Find.IsEmpty())
		error->SetError( -1, "Match: nothing to Find.");

	if(formula.GetSize() == 0)
		return uInt32Array();

	uInt32Array Index;
	Index.Initialize(0,0);

	for( int a = 0; a < formula.GetSize(); a++)
		if( stricmp( Find.CStr(), formula[a].CStr()) == 0)
			Index.Insert(a,-1);

	return Index;
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Add Addition
/// @{

/** @brief <B>ADD:\n</B>
  This function will 'ADD' values contained in a TString formula array by first\n
  locating the index of the '+' operation.  It will then convert the index before\n
  this operation to a float64 'a' value and the index of the math operation plus one\n
  to a float64 'b' value.  It will return a structure containing the Value where\n
  (float64) Value = a + b and the TStringArray of the formula.\n

@param[in,out] error Error Status Information
@param[in] formula TStringArray of values being added.
@param[out] Math_Struct (float64) Value and (TStringArray)formula
*/
Math_Struct Add(ErrorCluster *error, TStringArray formula )
{
	Math_Struct MS;
	
	if(error->Status())
		return MS;

	// Size for Add can be two.  This will allow for a formula
	// such as +6.7.  It will be calculated as: 0.0 + 6.7
	if(formula.GetSize() < 2)
		error->SetError( -1, "Add: Formula size error.");

	float64 a;
	float64 b;
	char  *stopstring;
	int Index = Match( error, formula, TString("+"))[0]; // One addition at a time

	if( Index == 0)
		a = 0.00;
	else
	{
		a = strtod( formula[Index-1].CStr(), &stopstring );
			
		if(!error->Status() && *stopstring != NULL)
			error->SetError(-1, "Add: Can't convert '%s'.", formula[Index-1].CStr());

		formula[Index-1] = TString("\0");
	}
			
	if(!StrIsMathToken(error, formula[Index+1]))
		b = strtod( formula[Index+1].CStr(), &stopstring );
	else
		error->SetError( -1, "Add: Formula error. Can't add '%s'.",formula[Index+1].CStr());
	
	if(!error->Status() && *stopstring != NULL)
		error->SetError(-1, "Add: Can't convert '%s'.", formula[Index+1].CStr());
	
	if(!error->Status())
	{
		MS.Value =  a+b;

		formula[Index].Format("%.15e", MS.Value);
		formula[Index+1] = TString("\0");
	}
	
	MS.Formula = RemoveNull(error, formula);
	return MS; 
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Subtract Subtraction
/// @{

/** @brief <B>Subtract:\n</B>
  This function will 'Subtract' values contained in a TString formula array by first\n
  locating the index of the '-' operation.  It will then convert the index before\n
  this operation to a float64 'a' value and the index of the math operation plus one\n
  to a float64 'b' value.  It will return a structure containing the Value where\n
  (float64) Value = a - b and the TStringArray of the formula.\n

@param[in,out] error Error Status Information
@param[in] formula TStringArray of values being subtracted.
@param[out] Math_Struct (float64) Value and (TStringArray)formula
*/
Math_Struct Subtract(ErrorCluster *error, TStringArray formula )
{
	Math_Struct MS;
	
	if(error->Status())
		return MS;

	if(formula.GetSize() == 0)
		error->SetError(-2,"Sub: Formula size is Zero.");

	float64 a;
	float64 b;
	char  *stopstring;

	// Size for Subtract can be two.  This will allow for a formula
	// such as -6.7.  It will be calculated as: 0.0 - 6.7
	if(formula.GetSize() < 2)
	{
		a = strtod( formula[0].CStr(), &stopstring );

		if(!error->Status() && *stopstring != NULL)
			error->SetError(-1, "Subtract: Can't convert '%s'.", formula[0].CStr());
		
		MS.Value =  a;

		formula[0].Format("%.15e", MS.Value);
	
		MS.Formula = RemoveNull(error, formula);
		return MS; 
	}

	int Index = Match( error, formula, TString("-"))[0]; 

	if( Index == 0)
		a = 0.00;
	else
	{
		if(!StrIsMathToken(error, formula[Index-1]))
		{
			a = strtod( formula[Index-1].CStr(), &stopstring );

			if(!error->Status() && *stopstring != NULL)
				error->SetError(-1, "Subtract: Can't convert '%s'.", formula[Index-1].CStr());

			formula[Index-1] = TString("\0");
		}
		else if(!error->Status() && stricmp(formula[Index-1].CStr(), "+") == 0)
		{
			if(!StrIsMathToken(error, formula[Index-2]) )
			{
				a = strtod( formula[Index-2].CStr(), &stopstring );
				
				if(!error->Status() && *stopstring != NULL)
					error->SetError(-1, "Subtract: Can't convert '%s'.", formula[Index-2].CStr());

				formula[Index-2] = TString("\0");
			}
			else
				error->SetError(-1,"Subtract: Formula error. A-B Can't find 'A' Value.");
		}
		else
			error->SetError(-1,"Subtract: Formula error. A-B Can't find 'A' Value.");
	}
		
	if(!error->Status() && !StrIsMathToken(error, formula[Index+1]))
		b = strtod( formula[Index+1].CStr(), &stopstring );
	else
	{
		if(!error->Status() && stricmp(formula[Index+1].CStr(), "-") == 0)
		{
			if(!StrIsMathToken(error, formula[Index+2]) )
			{
				b = strtod( formula[Index+2].CStr(), &stopstring );
				
				if(!error->Status() && *stopstring != NULL)
					error->SetError(-1, "Subtract: Can't convert '%s'.", formula[Index+2].CStr());

				if(!error->Status())
				{
					MS.Value =  a+b;	// - a - equals a plus but the mathorder still needs
										// to be given a value to work with
							
					formula[Index-1] = TString("\0");
					formula[Index].Format("%.15e", MS.Value);
					formula[Index+1] = TString("-");	// GetMathOrder will want to subtract again
					formula[Index+2] = TString("0.00");	// this will give it something to use
				}
				
				MS.Formula = RemoveNull(error, formula);
				return MS;
			}
					
		}
		if(!error->Status())
			error->SetError( -1, "Subtract: Formula error. Can't subtract '%s'.",formula[Index+2].CStr());
	}
	
	if(!error->Status() && *stopstring != NULL)
		error->SetError(-1, "Subtract: Can't convert '%s'.", formula[Index+1].CStr());

	if(!error->Status())
	{
		MS.Value =  a-b;

		formula[Index].Format("%.15e", MS.Value);
		formula[Index+1] = TString("\0");
	}
	
	MS.Formula = RemoveNull(error, formula);
	return MS; 
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Mult Multiplication
/// @{

/** @brief <B>Mult:\n</B>
  This function will 'Multiply' values contained in a TString formula array by first\n
  locating the index of the '*' operation.  It will then convert the index before\n
  this operation to a float64 'a' value and the index of the math operation plus one\n
  to a float64 'b' value.  It will return a structure containing the Value where\n
  (float64) Value = a * b and the TStringArray of the formula.\n

@param[in,out] error Error Status Information
@param[in] formula TStringArray of values being multiplied.
@param[out] Math_Struct (float64) Value and (TStringArray)formula
*/
Math_Struct Mult(ErrorCluster *error, TStringArray formula )
{
	Math_Struct MS;
	
	if(error->Status())
		return MS;

	float64 a;
	float64 b;
	char  *stopstring;

	if(formula.GetSize() < 3)
		error->SetError( -1, "Mult: Formula size error.");

	int Index = Match( error, formula, TString("*"))[0]; 

	if( Index == 0 || (Index == formula.GetSize() - 1))
		error->SetError( -1, "Mult: Formula can't start or end with '*'."); 
	else
	{
		if(!StrIsMathToken(error, formula[Index-1]) && stricmp(formula[Index-1].CStr(),"\0") != 0)
			a = strtod( formula[Index-1].CStr(), &stopstring );
		else
			error->SetError( -1, "Mult: 'A * B' Can't Mult 'A' value of '%s'.",formula[Index-1].CStr());
			
		if(!error->Status() && *stopstring != NULL)
			error->SetError(-1, "Mult: Can't convert '%s'.", formula[Index-1].CStr());
		
		if(!StrIsMathToken(error, formula[Index+1]) && stricmp(formula[Index-1], "\0") != 0)
			b = strtod( formula[Index+1].CStr(), &stopstring );
		else
		{	
			if(!error->Status() && stricmp(formula[Index+1].CStr(), "-") == 0)
				if(!StrIsMathToken(error, formula[Index+2]) && stricmp(formula[Index-1], "\0") != 0)
				{
					b = strtod( formula[Index+2].CStr(), &stopstring );

					if(!error->Status() && *stopstring != NULL)
						error->SetError(-1, "Mult: Can't convert '%s'.", formula[Index+2].CStr());

					if(!error->Status())
					{
						MS.Value =  a*b;
							
						formula[Index-1] = TString("\0");
						formula[Index] = TString("\0");
						formula[Index+2].Format("%.15e", MS.Value);
					}
				
					MS.Formula = RemoveNull(error, formula);
					return MS;
				}
			else
				error->SetError( -1, "Mult: 'A * B' Can't Mult 'B' value of '%s'.",formula[Index+1].CStr());
		}
		
		if(!error->Status() && *stopstring != NULL)
			error->SetError(-1, "Mult: Can't convert '%s'.", formula[Index+1].CStr());

		if(!error->Status())
		{
			MS.Value =  a*b;
					
			formula[Index-1] = TString("\0");
			formula[Index].Format("%.15e", MS.Value);
			formula[Index+1] = TString("\0");
		}
	}
	
	MS.Formula = RemoveNull(error, formula);
	return MS; 
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Divide Division
/// @{

/** @brief <B>Divide:\n</B>
  This function will 'Divide'values contained in a TString formula array by first\n
  locating the index of the '/' operation.  It will then convert the index before\n
  this operation to a float64 'a' value and the index of the math operation plus one\n
  to a float64 'b' value.  It will return a structure containing the Value where\n
  (float64) Value = a / b and the TStringArray of the formula.\n

@param[in,out] error Error Status Information
@param[in] formula TStringArray of values being divided.
@param[out] Math_Struct (float64) Value and (TStringArray)formula
*/
Math_Struct Divide(ErrorCluster *error, TStringArray formula)
{
	Math_Struct MS;

	if(error->Status())
		return MS;

	float64 a;
	float64 b;
	char  *stopstring;

	if(formula.GetSize() < 3)
		error->SetError( -1, "Divide: Formula size error.");

	int Index = Match( error, formula, TString("/"))[0]; 

	if( Index == 0 || (Index == formula.GetSize() - 1))
		error->SetError( -1, "Divide: Formula can't start or end with '/'."); 
	else
	{
		if(!StrIsMathToken(error, formula[Index-1]) && stricmp(formula[Index-1].CStr(), "\0") != 0)
			a = strtod( formula[Index-1].CStr(), &stopstring );
		else
			error->SetError( -1, "Divide: 'A / B' Can't divide 'A' value of '%s'.",formula[Index-1].CStr());

		if(!error->Status() && *stopstring != NULL)
			error->SetError(-1, "Divide: Can't convert '%s'.", formula[Index-1].CStr());
			
		if(!StrIsMathToken(error, formula[Index+1]) && stricmp(formula[Index-1].CStr(), "\0") != 0)
		{
			b = strtod( formula[Index+1].CStr(), &stopstring );
			
			if( b == 0.00)
				error->SetError( -1, "Divide: Divide by zero error.");

			if(!error->Status() && *stopstring != NULL)
				error->SetError(-1, "Divide: Can't convert '%s'.", formula[Index+1].CStr());
		}
		else
		{	
			if(!error->Status() && stricmp(formula[Index+1].CStr(), "-") == 0)
				if(!StrIsMathToken(error, formula[Index+2]) && stricmp(formula[Index-1], "\0") != 0)
				{
					float64 TempB = strtod( formula[Index+2].CStr(), &stopstring );// I know this value was '-'
					b = -1.00 * TempB;
					
					if(!error->Status() && *stopstring != NULL)
						error->SetError(-1, "Divide: Can't convert '%s'.", formula[Index+2].CStr());

					if(!error->Status())
					{
						MS.Value =  a/b;
							
						formula[Index-1] = TString("\0");	// Location of 'a' value
						formula[Index] = TString("\0");		// Location of '/'
						formula[Index+1] = TString("\0");	// Location of '-' 
						formula[Index+2].Format("%.15e", MS.Value);	// Insert new value
					}
				
					MS.Formula = RemoveNull(error, formula);

					return MS;
			}
			else
				error->SetError( -1, "Divide: 'A / B' Can't divide 'B' value of '%s'.",formula[Index+1].CStr());
		}
		if(!error->Status())
		{
			MS.Value =  a/b;
				
			formula[Index-1] = TString("\0");
			formula[Index].Format("%.15e", MS.Value);
			formula[Index+1] = TString("\0");
		}
	}

	MS.Formula = RemoveNull(error, formula);
	return MS; 
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Exp Exponent
/// @{

/** @brief <B>Exp:\n</B>
  This function will raise a value by an exponent contained in a TString formula array \n
  by first locating the index of the '^' operation.  It will then convert the index before\n
  this operation to a float64 'a' value and the index of the math operation plus one\n
  to a float64 'b' value.  It will return a structure containing the Value where\n
  (float64) Value = pow(a, b) {aka a^b} and the TStringArray of the formula.\n

@param[in,out] error Error Status Information
@param[in] formula TStringArray of values being multiplied.
@param[out] Math_Struct (float64) Value and (TStringArray)formula
*/
Math_Struct Exp(ErrorCluster *error, TStringArray formula)
{
	Math_Struct MS;

	if(error->Status())
		return MS;

	float64 a;
	float64 b;
	char  *stopstring;

	if(formula.GetSize() < 3)
		error->SetError( -1, "Exp: Formula size error.");

	int Index = Match( error, formula, TString("^"))[0]; // Exponents Index

	if( Index == 0 || (Index == formula.GetSize() - 1))
		error->SetError( -1, "Exp: Formula can't start or end with '^'."); 

	else
	{
		if(!StrIsMathToken(error, formula[Index-1]) && stricmp(formula[Index-1].CStr(), "\0") != 0)
			a = strtod( formula[Index-1].CStr(), &stopstring );
		else
			error->SetError( -1, "Exp: 'A ^ B' Formula error. 'A' value of '%s' Invalid.",formula[Index-1].CStr());
		
		if(!error->Status() && *stopstring != NULL)
			error->SetError(-1, "Exp: Can't convert '%s'.", formula[Index-1].CStr());
		
		if(!StrIsMathToken(error, formula[Index+1]) && stricmp(formula[Index-1].CStr(), "\0") != 0)
			b = strtod( formula[Index+1].CStr(), &stopstring );
		else
		{	
			if(!error->Status() && stricmp(formula[Index+1].CStr(), "-") == 0)
			{
				if(!StrIsMathToken(error, formula[Index+2]) && stricmp(formula[Index-1], "\0") != 0)
				{
					float64 TempB = strtod( formula[Index+2].CStr(), &stopstring );// I know this value was '-'
					b = -1.00 * TempB;
					
					if(!error->Status() && *stopstring != NULL)
						error->SetError(-1, "Exp: Can't convert '%s'.", formula[Index+2].CStr());

					if(!error->Status())
					{
						MS.Value =  pow( a, b);
							
						formula[Index-1] = TString("\0");	// Location of 'a' value
						formula[Index] = TString("\0");		// Location of '^'
						formula[Index+1] = TString("\0");	// Location of '-' 
						formula[Index+2].Format("%.15e", MS.Value);	// Insert new value
					}
				
					MS.Formula = RemoveNull(error, formula);

					return MS;
				}
				else
					error->SetError( -1, "Exp: 'A ^ B' 'B' value of '%s' Invalid.",formula[Index+1].CStr());
			}
		}

		if(!error->Status() && *stopstring != NULL)
			error->SetError(-1, "Exp: Can't convert '%s'.", formula[Index+1].CStr());
		
		if(!error->Status())
		{

			MS.Value = pow( a, b);
		
			formula[Index-1] = TString("\0");
			formula[Index].Format("%.15e", MS.Value);
			formula[Index+1] = TString("\0");
		}
	}
	
	MS.Formula = RemoveNull(error, formula);
	return MS; 
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup SortABSValue Sort Absolute Value Portion of Formula Array
/// @{

/** @brief <B>SortABSValue:\n</B>
	This function will return an array of abs value starting and ending index values.\n 
	The first value will be the opening index location followed by the closing value \n
	index.  The pattern is repeated.\n\n 

@param[in,out] error Error Status Information
@param[in] Formula TString array of the formula
@param[out] uInt32Array containing indices of absolute values structures
*/
uInt32Array SortABSValue( ErrorCluster *error, TStringArray Formula)
{
	if(error->Status())
		return uInt32Array();

	uInt32Array sABS_Index; // Starting ABS Index
	uInt32Array eABS_Index; // Ending ABS Index


	sABS_Index.Initialize(0,0);
	eABS_Index.Initialize(0,0);

	uInt32Array ABS_Index = Match( error, Formula, TString("|")); // Absolute Value Index

	if(!error->Status() && ABS_Index > 1)
	{
		for(int i = 0; i < ABS_Index.GetSize(); i++)
		{
			sABS_Index.Insert(ABS_Index[i],-1);
			i++;
			if(i < ABS_Index.GetSize())
				eABS_Index.Insert(ABS_Index[i],-1);
		}

	}

	if(sABS_Index.GetSize() != eABS_Index.GetSize())
		error->SetError( -1, "SortABSValue: Start vs. Ending ABS value error. start:'%d' end:'%d'", sABS_Index.GetSize(), eABS_Index.GetSize());

	return ABS_Index;
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup GetMathOrder Get Math Order of Operations
/// @{

/** @brief <B>GetMathOrder:\n</B>
    This function will  return an array containing the order of operations for the\n
	calculations.  This function will not support parentheses or absolute value\n
	math operations which have to be removed before this function is called.  The\n
	order that of the search is done as follows:\n\n

	1. Search for exponentials or '^' and return element of 'EXP'\n
	2. Search for division or '/' and return element of 'DIV'\n
	3. Search for multiplication or '*' and return element of 'MUL'\n
	4. Search for subtraction or '-' and return element of 'SUB'\n
	5. Search for addition or '+' and return element of 'ADD'\n\n

	This search will apply to calculation such as 5-3+2 to ensure they will be \n
	performed correctly. The normal concerns about order of operations should be \n
	upheld.\n  
	
@param[in,out]  error Error Status Information
@param[in] Formula TString array of the formula
@param[out] TStringArray containing the order of operations
*/
TStringArray GetMathOrder(ErrorCluster *error, TStringArray Formula)
{
	
	if(error->Status())
		return TStringArray();

	TStringArray Data;		// Contains Order Array
	Data.Initialize("\0",0);
	int i = 0;

	uInt32Array ex_Index = Match( error, Formula, TString("^")); // Exponents Index
	uInt32Array d_Index = Match( error, Formula, TString("/"));  // division Index
	uInt32Array m_Index = Match( error, Formula, TString("*"));  // multiplication Index
	uInt32Array s_Index = Match( error, Formula, TString("-"));  // subtraction Index
	uInt32Array a_Index = Match( error, Formula, TString("+"));  // addition Index

	if(error->Status())
		return Data;

	if(ex_Index.GetSize() > 0)
		for( i = 0; i < ex_Index.GetSize(); i++)
			Data.Insert( "EXP", -1);
		
	if(d_Index.GetSize() > 0)
		for( i = 0; i < d_Index.GetSize(); i++)
			Data.Insert( "DIV", -1);

	if(m_Index.GetSize() > 0)
		for( i = 0; i < m_Index.GetSize(); i++)
			Data.Insert( "MUL", -1);

	if(s_Index.GetSize() > 0)
		for( i = 0; i < s_Index.GetSize(); i++)
			Data.Insert( "SUB", -1);

	if(a_Index.GetSize() > 0)
		for( i = 0; i < a_Index.GetSize(); i++)
			Data.Insert( "ADD", -1);

	if(Formula.GetSize() == 1 && Data.GetSize() == 0)
	{
		Data.Insert( "No_Calc", -1);
		return Data;
	}
	else if(Formula.GetSize() == 1 && Data.GetSize() > 0)
		error->SetError(-1, "GetMathOrder: Can not do math on the operation of '%s'", Formula[0].CStr());

	return Data;
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup SortParentheses Sort Parentheses Portions of Formula Array
/// @{

/** @brief <B>SortParentheses:\n</B>
   This function will return an array of opening and closing parentheses index values.\n 
	The first value will be the opening index location followed by the closing value\n 
	index.  The pattern is repeated.\n\n
	
	Here is how it works:\n
		When the arrays of '(' and ')' are collected starting from the left of the\n
		expression and moving right, the order of the expression can be determined\n
		by starting from the last '(' Index and look for the first index of the ')'\n
		that is larger than the index of the '('.  For example:\n\n

		0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 = index\n
		( ( A * B ) - C ) /  (  C  +  1  ) = formula\n\n

		opening found at:        0,1,10\n
		closing found at:        5,8,14\n\n

		start opening search at index 2 ( value of 10) and look for the first \n
		closing index that is larger than 10 by starting the closing search at\n
		index 0 (value 5).  This results in the closing being found at index 2 \n
		value of 14.  The second search begins at opening index 1 (value 1).\n  
		Again searching the closing array starting at index 0 (value 5) results\n
		in the correct parenthesis being found.\n\n  

		*As the closing index is found, the closing index value is replaced with\n
		0 so that it will not be considered for the next search.\n\n

		There are two advantages to this method.  One is that it allows for 'over-\n
		use' or embedded parentheses.  Two, provides an order to ensure a value can\n
		be supplied to the next parenthesized portion of the equation.\n\n

@param[in,out] error Error Status Information
@param[in] Formula TString array of the formula
@param[out] uInt32Array containing indices the sorted parentheses structures
*/
uInt32Array SortParentheses( ErrorCluster *error, TStringArray Formula)
{
	
	if(error->Status())
		return uInt32Array();

	uInt32Array Data = uInt32Array();
	uInt32 op_Test = 0;

	uInt32Array op_Index = Match( error, Formula, TString("(")); // Opening parentheses Index
	uInt32Array cp_Index = Match( error, Formula, TString(")")); // Closing parentheses Index

	bool Done;

	if(op_Index.GetSize() == 0 && cp_Index.GetSize() == 0)
		return uInt32Array();

	if( op_Index.GetSize() != cp_Index.GetSize())
	{
		error->SetError( -1, "SortParentheses: Opening vs. Closing parentheses error. op:'%d' cp:'%d'", op_Index.GetSize(), cp_Index.GetSize());
		return uInt32Array();
	}

	for( int i = op_Index.GetSize() - 1; i >= 0; i--) 
	{
		Done = false;
		op_Test = op_Index[i];	// start at last index & work backwards
		Data.Insert(op_Test,-1);

		for(int a = 0; a < cp_Index.GetSize() && !Done; a++)
		{
			if( cp_Index[a] > op_Test)
			{
				Data.Insert(cp_Index[a],-1);
				cp_Index[a] = 0; // Effectively removes element
				Done = true;
			}
		}
	}

	if(!Done)
		error->SetError(-1, "SortParentheses - Order is invalid.  Check formula.");

	return Data;

}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Get_PA_Order Get Parentheses an ABS Order
/// @{

/** @brief <B>Get_PA_Order:\n</B>
	This function will sort the order in which Parentheses and ABS\n
	math operations need to be executed.  It is done by testing the\n
	starting location of the Parentheses against the Starting index of\n
	the ABS value.  The larger of the two index determines the operation\n
	order.  This function returns an array indicating the order by placing\n
	'PAR' for a parentheses operation or 'ABS' for an absolute value\n
	operation.\n\n

@param[in,out] error Error Status Information
@param[in] SP uInt32Array Sorted Parentheses Index
@param[in] SA uInt32Array Sorted ABS Value Index
@param[out] TStringArray of order
*/
TStringArray Get_PA_Order(ErrorCluster *error, uInt32Array SP, uInt32Array SA)
{
	if(error->Status())
		return TStringArray();

	TStringArray Data;
	TString PAR = TString("PAR");
	TString ABS = TString("ABS");

	int P_Size = SP.GetSize()/2;
	int sp = 0;
	int A_Size = SA.GetSize()/2;
	int sa = 0;
	int OrderSize = P_Size + A_Size;
	bool P_Done = false;
	bool A_Done = false;

	if(P_Size == 0)
		P_Done = true;
	if(A_Size == 0)
		A_Done = true;

	Data.Initialize("/0", OrderSize);

	for( int i = 0; !error->Status() && i < OrderSize; i++)
	{
		if(!P_Done && ((SP[sp] > SA[sa]) || A_Done))
		{
			Data[i] = PAR;
			if(sp < (SP.GetSize() - 2))
				sp+=2;
			else
				P_Done = true;
		}
		else if(!A_Done && ((SA[sa] > SP[sp]) || P_Done)) 
		{
			Data[i] = ABS;
			if(sa < (SA.GetSize() - 2))
				sa+=2;
			else
				A_Done = true;
		}
		else // This error would occur if the starting ABS and PAR somehow where equal
			error->SetError(-1, "Get_PA_Order: Internal Error Sorting SA: '%d', SA: '%d'", SA[sa], SP[sp]);

	}

	if(!error->Status() && (!P_Done || !A_Done))
		error->SetError( -1, "Get_PA_Order: Internal Sorting Error.");

	if(!error->Status())
		return Data;

	return TStringArray();

}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup DoMath Do Math Operation
/// @{

/** @brief <B>DoMath:\n</B>
	DoMath will return the value for a given equation that may contain the following\n
	math operations:\n\n
	1 = exponential denoted as ^\n
	2 = division denoted as /\n
	3 = multiplication denoted as *\n
	4 = subtraction denoted as -\n
	5 = addition denoted as +\n\n

	The parentheses and absolute value math operations must be removed prior to calling\n
	this function.  This function will first call SortParentheses and SortABSValue and \n
	will then 'Do Math' based on a call to GetMathOrder.  It will return a float64 of\n
	the result\n\n

@param[in,out] error Error Status Information
@param[in] formula TString array of the formula
@param[out] float64 result of the calculation
*/
float64 DoMath(ErrorCluster *error, TStringArray formula)
{
	Math_Struct MS;
	
	if(error->Status())
		return 0.0;

	char  *stopstring;
	TStringArray Order = TStringArray();
	formula = RemoveNull(error, formula);

	MS.Formula = formula;

	uInt32Array SP = SortParentheses( error, formula);
	uInt32Array SA = SortABSValue( error, formula);

	if(!error->Status() && (SP.GetSize() > 0 || SA.GetSize() > 0))
		error->SetError(-1, "DoMath: ABS Value or Parentheses incorrectly removed.");

	if(formula.GetSize() > 0)
		Order = GetMathOrder(error, formula);

	if(error->Status())
		return 0.0;

	if(!error->Status() && formula.GetSize() == 1)
	{
		MS.Value = strtod( formula[0].CStr(), &stopstring );
		MS.Formula = &TString();

		return MS.Value;
	}
	else
	{
		for(int i = 0; i < Order.GetSize(); i++)
		{
			if(stricmp( Order[i].CStr(), "EXP") == 0)
				MS = Exp(error, MS.Formula);
			else if(stricmp( Order[i].CStr(), "DIV") == 0)
				MS = Divide(error, MS.Formula);
			else if(stricmp( Order[i].CStr(), "MUL") == 0)
				MS = Mult(error, MS.Formula);
			else if(stricmp( Order[i].CStr(), "SUB") == 0)
				MS = Subtract(error, MS.Formula);
			else if(stricmp( Order[i].CStr(), "ADD") == 0)
				MS = Add(error, MS.Formula);
			else
				error->SetError( -1, "DoMath: Math Operator of '%s' unKnown.", Order[i].CStr());
		}	
	
	}

	uInt32Array op_Index = Match( error, MS.Formula, TString("(")); // Opening parentheses Index
	uInt32Array cp_Index = Match( error, MS.Formula, TString(")")); // Closing parentheses Index

	if(MS.Formula.GetSize() > 1 && op_Index.GetSize() == 0 && cp_Index.GetSize() == 0)
		error->SetError( -1, "DoMath: Check formula format. Formula Size: '%d'",MS.Formula.GetSize());

	TString Test;
	Test.Format("%.15e",strtod( MS.Formula[0].CStr(), &stopstring )); // Check stopstring and conversions

	// If the stopstring isn't NULL then there are characters in the formula that
	// can not be converted to a number.  If the formula does not match the value
	// then a conversion generated an error.
	if( *stopstring != NULL || (stricmp(Test.CStr(), MS.Formula[0].CStr()) != 0))
		error->SetError( -1, "DoMath: Error converting MS.Formula: '%s'. Test:'%.15f'", MS.Formula[0].CStr(), Test);

	return MS.Value;
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup InsertData Insert Data into Formula Array
/// @{

/** @brief <B>InsertData:\n</B>
	Insert data into formula will replace formula index with the value handed without\n
	changing the size of the formula array.  The remainder of the formula will be replaced\n
	with "\0".\n

@param[in,out] error Error Status Information
@param[in] formula TString array of the formula
@param[in] Start location of start of the formula being reformatted
@param[in] Stop location of the end of the formula being reformatted
@param[in] Value to be inserted in Start index
@param[out] TStringArray with 'Value' inserted and rest of formula NULLed
*/

TStringArray InsertData(ErrorCluster *error, TStringArray formula, uInt32 Start, uInt32 Stop, float64 Value)
{		
	if(error->Status() || formula.GetSize() == 0)
		return TStringArray();

	TString Data;
	Data.Format("%.15f", Value);

	formula[(int)Start] = Data;
	int a = 0;
	if(formula.GetSize() > 1)
		for( a = (int)Start+1; a < (int)Stop + 1; a++)
			formula[a] = TString("\0");
	
	return formula;
}
/// @}
/// @}

/// @addtogroup Math
/// @{
/// @addtogroup Calculate Perform Calculation
/// @{

/** @brief <B>Calculate:\n</B>
  It will start by determining the number of parentheses, absolute value operations\n
  and number of math operations.  From this information the calculation will be executed\n
  and the result will be returned in the form of a float64.\n
  
@param[in,out] error Error Status Information
@param[in] formula TString array of the formula
@param[out] float64 result of calculation
*/
float64 Calculate(ErrorCluster *error, TStringArray formula)
{

	if(error->Status())
		return 0.0;

	int i = 0;
	int Size = 0;
	float64 Value = 0.00;

	//  The following Sort functions will report an error if a matching stop
	//  math operation is not found.
	uInt32Array SP = SortParentheses( error, formula);
	uInt32Array SA = SortABSValue( error, formula);

	int P_Size = SP.GetSize()/2;
	int sp = 0;
	int A_Size = SA.GetSize()/2;
	int sa = 0;

	bool P_Done = false;
	bool A_Done = false;

	int NumberOfOps = (SP.GetSize() + SA.GetSize())/2;

	if(!error->Status() && NumberOfOps == 0) // No () or ABS
	{
		Value = DoMath(error, formula);
		formula = InsertData(error, formula, 0, formula.GetSize(), Value);
	}

	else if(!error->Status() && SP.GetSize() > 0 || SA.GetSize() > 0) // has ABS or ()
	{	
	
		TStringArray Order = Get_PA_Order(error, SP, SA);

		if(error->Status())
			return 0.0;

		for( i = 0; i < Order.GetSize(); i++)
		{	
			if(!P_Done && stricmp( Order[i].CStr(), "PAR") == 0)
			{	
				Size = SP[sp+1] - SP[sp] - 1; // remove ( and )
						
				Value = DoMath(error, formula.SubArray(SP[sp]+1,Size));
				formula = InsertData(error, formula, SP[sp], SP[sp+1], Value);

				if(sp < (SP.GetSize() - 2))
					sp+=2;
				else
					P_Done = true;
			}

			if(!A_Done && stricmp( Order[i].CStr(), "ABS") == 0)
			{	
				Size = SA[sa+1] - SA[sa] - 1; // remove | and |
						
				Value = fabs(DoMath(error, formula.SubArray(SA[sa]+1,Size)));
				formula = InsertData(error, formula, SA[sa], SA[sa+1], Value);

				if(sa < (SA.GetSize() - 2))
					sa+=2;
				else
					A_Done = true;
			}
			
		} // End for( i = 0; i < Order.GetSize(); i++)

	} // End else  Has () and/or ||

	formula = RemoveNull(error, formula); // Clean the formula up
	if(!error->Status() && formula.GetSize() > 1)
	{
		float64 Value = DoMath(error, formula);
		formula = InsertData(error, formula, 0, formula.GetSize(), Value);
		formula = RemoveNull(error, formula); // Clean the formula up

		if (formula.GetSize() == 1)
			return Value;
	}

	if(!error->Status() && formula.GetSize() > 1)
		error->SetError(-1, "Calculation: Error making calculation. Check format of () and ||.");
	
	return Value;

}
/// @}
/// @}
/// @addtogroup Math
/// @{
/// @addtogroup PEMDAS Top level Call for Calculation
/// @{

/** @brief <B>PEMDAS:\n</B>
  This is the "top level" function for getting a calculation.  It will\n
  convert the Vector and Data elements to a Formula array and call Calculate\n
  to perform the calculations.  The result will be returned in the form of\n
  a float64.\n
  
@param[in,out] error Error Status Information
@param[in] String element that is looked-up
@param[in] VectorIn Vector XML String
@param[in] DataIn Data XML String
@param[out] float64 containing value of the calculation
*/
float64 PEMDAS(ErrorCluster *error, TString String, TString VectorIn, TString DataIn)
{
	float64 Data = 0.00;

	if(error->Status())
		return Data;

	TString Input = String;

	TStringArray Formula = NamesToNumber(error, Input, VectorIn, DataIn ); // Contains the formula with Names replaced with numbers

	Data = Calculate(error, Formula);

	return Data;
}

/// @}
/// @}

/// @addtogroup ValueCache
/// @{
/// @addtogroup CacheValueCk Value Cache Check
/// @{

/** @brief <B>CacheValueCk:\n</B>
	A value cache parameter can be added to the XML configuration file to\n
	allow for limited vector to vector comparisons.  The most typical use\n
	of this would be when a 'nominal' value needs to be captured and a series\n
	of vectors need to be compared with respect to that nominal.  The Value_Cache\n
	contains Cache_Elements comprised of a name and a condition.  The name contains\n
	the name used within the Requirement configurations section followed by the column\n
	name that relates to the value you want to store.\n\n
	
	This function will parse out and present the cache values for a given\n
	vector and data field.\n
  
@param[in,out] error Error Status Information
@param[in] V_In Vector Data In
@param[in] Parsed_ValueCache Parsed Value Cache from XML
@param[in] Data Parsed Data In
@param[out] void	Values are State Cached
*/
  
void CacheValueCk(ErrorCluster *error, const TString &V_In, TStringArray Parsed_ValueCache, const TString &Data)
{
	if (error->Status())			// if error in, bail out
		return;

	bool Test = false;
	
	TStringArray Name = TStringArray();		// Cache Name
	TStringArray CacheCondition = TStringArray();

	for( int a = 0; a < Parsed_ValueCache.GetSize(); a+=2)
	{
		Name.Insert(Parsed_ValueCache[a]);
		CacheCondition.Insert(Parsed_ValueCache[a+1]);
	}

	for( a = 0; !error->Status() && a < Name.GetSize(); a++) // Check to see if Cache Condition Matches V In
	{	
		bool In_C = false;
		Test = ConditionCheck(error, V_In, Data, CacheCondition[a]); // Should change this
		
		if(!error->Status() && Test)
		{
			TStringArray TempData = ParseString( error, Name[a] );

			if(!IsCacheNamed(error, TempData[0]))
				error->SetError(-1, "CacheValueCk: Name of '%s' not found in Vector, Data, or Value Cache.",TempData[0].CStr());

			// Check if value has already been cached
			if( !error->Status() && CacheName.GetSize() > 0)
				for( int b = 0; b < CacheName.GetSize()  && !In_C; b++)
					In_C = stricmp(TempData[0].CStr(), CacheName[b].CStr()) == 0;

			if( !error->Status() && !In_C) // If not in cache already, store it
			{
				TString SoData;
				float64 SData = PEMDAS(error, TempData[1], V_In, Data);
				if(!error->Status())
				{
					CacheName.Insert(TempData[0], -1);
					SoData.Format("%.15e", SData);
					CacheValue.Insert(SoData);
				}
		
			}
	
		}
	}
	
}
/// @}
/// @}

/// @addtogroup Main
/// @{
/// @addtogroup CINRun Main Function definition
/// @{

/** @brief <B>CINRun:\n</B>
	This is the entry point.\n\n
	Before this code will execute on/for a test the requirements need to be generated in a specific\n
	form as follows:\n\n

<Requirements>\n
________<Chapter_Info>\n
________<name>Chapter Name</name>\n
________<Version>Requirement Version</Version>\n
____</Chapter_Info>\n
____<Requirement_element>\n
________<ReqID>Requirement ID or Number</ReqID>\n
________<Parameter_Type>name or definition for the given ReqID</Parameter_Type>\n
________<units>unit of measurement for Test_Parameter</units>\n
________<Test_Parameter>Name of Control and/or Calculation compared against Limits</Test_Parameter>\n
________<Conditions>\n
____________<Condition>Vector settings required for limit test</Condition>\n
____________<max>Upper Limit</max>\n
____________<nom>Nominal Value with respect to limits</nom>\n
____________<min>Minimum Limit</min>\n
________</Conditions>\n
____</Requirement_element>\n
</Requirements>\n\n
	A valid path provided to the 'Req_DataPath' control will load these requirements in to memory.\n
	If no values are presents on the 'VectorIn' or 'DataIn' control, the requirements will be\n
	formatted and handed back to Labview by way of the 'All_Requirements' structure.  If values\n
	are available in both 'VectorIn' and 'DataIn' then the vector will be searched for a <ReqID>\n
	If it is not found or the value is 'N/A' then the results will be returned without any additional\n
	information applied.  This is also true if an error is handed into this function.  If a value\n
	is found in the vector then the Requirements will be compared to the vector <ReqID>.  If a match\n
	is found then the condition portion of the Requirements is tested against the Vector.  When a match\n
	is found math will be performed and the results will get the requirement information appended to it\n
	and the data will be handed back to Labview by way of 'ResultsOut'.  The format of this appended\n
	structure is as follows:\n\n

<req_element>\n
____________<req_name>Requirement ID</req_name>\n
____________<result_name>Test parameter name; this may be a calculation</result_name>\n
____________<req_upperlim>Upper Limit</req_upperlim>\n
____________<req_value>Test Parameter Value</req_value>\n
____________<req_lowerlim>Lower Limit</req_lowerlim>\n
____________<req_status>Pass or Review</req_status>\n
</req_element>\n\n

	In the case that the vector requirement or condition of the vector can not be match against the\n
	requirements provided, req_status will indicate a mis-match.  If TBD is placed in the limits\n
	req_status will indicate Review.\n\n
  
@param[in,out] error Error Status Information
@param[in] Req_DataPath	XML Requirement Data Path
@param[in] VectorIn TString Vector in Data
@param[in] DataIn TString Data in Information
@param[out] All_Requirements	Structure containing all requirements from XML
@param[out] ResultsOut			XML formatted TString Data
*/ 

extern "C" CIN MgErr CINRun(All_ReqsHdl All_Requirements, LStrHandle Req_DataPath, LStrHandle VectorIn, LStrHandle DataIn, LStrHandle ResultsOut, LV_ErrorCluster *error )
{
	
	ErrorCluster Err(error);			// Declare C++ 'ErrorCluster' Class
	ErrorCluster TempErr(error);		// Declare C++ 'ErrorCluster' Class
	TString R_Path = Req_DataPath;		// Requirement Path

	LStrHandle nCondition = NULL;
	LStrHandle nMax = NULL;
	LStrHandle nNom = NULL;
	LStrHandle nMin = NULL;

	LStrHandle nID = NULL;
	LStrHandle nParameter_Type = NULL;
	LStrHandle nUnits = NULL;
	LStrHandle nTest_Parameter = NULL;
	
	TString V_IN = RemoveMathTokens(&Err, VectorIn);			// Vector Data IN
	TString D_IN = RemoveMathTokens(&Err, DataIn);			// Results Data IN
	int32 SizeReqmt = 0;

	if (error->status != LVBooleanFalse)	// Check for Error In
	{
		D_IN = DataIn;
		D_IN.SetLV_String(&TempErr, &ResultsOut);
		return noErr;
	}
	else
	{	
		if(!R_Path.IsEmpty())
		{
			R_IN = LoadFile(&Err, R_Path);
		
			// Re-Init Value_Cache Items when the config is loaded
			CacheName = TStringArray();
			CacheNamed = TStringArray();  // Cache Names from File
			CacheValue = TStringArray();
			Parsed_ValueCache = TStringArray();
		}

		TStringArray Parsed_Conditions; 

		// Get all of the Requirement Blocks
		TStringArray Parsed_Reqmts =  Parse_Reqmts(&Err, R_IN);
		SizeReqmt = Parsed_Reqmts.GetSize();
		Parsed_ValueCache =  Parse_ValueCache(&Err, R_IN);  // Used for Cross Vector Testing

		// Get Array Containing Size(s) of Condition sections
		uInt32Array CondSize;
		CondSize.Initialize(0,0);
		int32 tCondSize = 0;

		for( int a = 0; !Err.Status() && a < SizeReqmt; a++)
		{
			Parsed_Conditions = Parse_Cond_Limits(&Err, Parsed_Reqmts[a]);
			tCondSize += Parsed_Conditions.GetSize();
			CondSize.Insert(Parsed_Conditions.GetSize(), -1);
		}

		if(SetCINArraySize((UHandle)All_Requirements, ParamNumber, tCondSize) != 0)  // Set Requirements array size
			Err.SetError(-1, "GetRequirements: Unable to set Requirements array size.");

		if(Err.Status())
			return Err.LVErr(error);

		(*All_Requirements)->dimSize = tCondSize;
		
		int c = 0;

		for( a = 0; !Err.Status() && a < SizeReqmt; a++ ) 
		{	
					
			Parsed_Conditions = Parse_Cond_Limits(&Err, Parsed_Reqmts[a]);

			if(Err.Status())
				return Err.LVErr(error);
					
			for(int b = 0; b < Parsed_Conditions.GetSize(); b++)
			{	
				nCondition = NULL;
				nMax = NULL;
				nNom = NULL;
				nMin = NULL;

				nID = NULL;
				nParameter_Type = NULL;
				nUnits = NULL;
				nTest_Parameter = NULL;
				
				Parse_OneID(&Err, Parsed_Reqmts[a]).SetLV_String(&Err, &nID);
				Parse_ParameterType(&Err, Parsed_Reqmts[a]).SetLV_String(&Err, &nParameter_Type);
				Parse_Unit(&Err, Parsed_Reqmts[a]).SetLV_String(&Err, &nUnits);
				Parse_TestParameter(&Err, Parsed_Reqmts[a]).SetLV_String(&Err, &nTest_Parameter);
				
				(*All_Requirements)->Requirements[c].ID = nID;
				(*All_Requirements)->Requirements[c].Parameter_Type = nParameter_Type;
				(*All_Requirements)->Requirements[c].units = nUnits;
				(*All_Requirements)->Requirements[c].Test_Parameter = nTest_Parameter;
				
				Parse_Condition(&Err, Parsed_Conditions[b]).SetLV_String(&Err, &nCondition);
				Parse_Max(&Err, Parsed_Conditions[b]).SetLV_String(&Err, &nMax);
				Parse_Nom(&Err, Parsed_Conditions[b]).SetLV_String(&Err, &nNom);
				Parse_Min(&Err, Parsed_Conditions[b]).SetLV_String(&Err, &nMin);
				
				if(Err.Status())
					return Err.LVErr(error);
				
				(*All_Requirements)->Requirements[c].Condition = nCondition;
				(*All_Requirements)->Requirements[c].max = nMax;
				(*All_Requirements)->Requirements[c].nom = nNom;
				(*All_Requirements)->Requirements[c].min = nMin;
				
				c+=1;
			}
		
			if(Err.Status())
				return Err.LVErr(error);
		
		}
	
	}

	if(!Err.Status() && !V_IN.IsEmpty() && !D_IN.IsEmpty())
	{
		TString DataOut = TString();				// This will store Out req format results
		int ID_Match = -1;							// This will contain the req table index match vector ID
		TStringArray v_Type = TStringArray();		// Vector data Type
		TStringArray v_MisMatch = TStringArray();	// Vector Vector Req vs Req Table Data Log
		TString RMax;		// Requirement Table MAX Limit String
		TString RNom;		// Requirement Table Nominal String
		TString RMin;		// Requirement Table MIN Limit String
		TStringArray RNomArray;	// Requirement Table Nominal TStringArray
		TString nv_ID;		// Current Vector ReqID being used
		TString dStatus;	// Out Results Data Status
		float64Array dMax = float64Array();
		float64Array dNom = float64Array();
		float64Array dMin = float64Array();
		float64 dvalue = 0.00;
		int tpIndex = 0;	// Test Parameter Index
		
		// Check to see if there are Cached Values to Log for cross vector comparisons
		if(Parsed_ValueCache.GetSize() > 0)
			CacheValueCk(&Err, RemoveSpace(&Err, V_IN), Parsed_ValueCache, RemoveSpace(&Err, D_IN));
			
		TStringArray v_Vector = Parse_vSections(&Err, RemoveSpace(&Err, V_IN));	// Get Vector Sections
		TStringArray v_ReqID = Parse_vReqID(&Err, RemoveSpace(&Err, V_IN));		// Get Vector ReqIDs
		TStringArray v_Data = Parse_dSections(&Err, RemoveSpace(&Err, D_IN));	// Get Data Sections

		// First verify the ReqID in the vector and against the requirements and get the limits
		if(!Err.Status() && v_ReqID.GetSize() > 0 && v_Vector.GetSize() > 0)
			for(int ReqID_Index = 0; ReqID_Index < v_ReqID.GetSize(); ReqID_Index++)
			{
				dMax = float64Array();	// Data is appended, so have to clean it out each time
				dNom = float64Array();	// Data is appended, so have to clean it out each time
				dMin = float64Array();	// Data is appended, so have to clean it out each time
				nv_ID = v_ReqID[ReqID_Index];
				bool found = false;		// Indicate if ReqID has been found in Vector
				bool NoLimit = false;	// If TBD found in limits, Allows set to Review
				bool done = false;	// Flag for a calculation check
				ID_Match = -1;	// Reset Requirement ID index

				if(!nv_ID.IsEmpty())
				{
					found = false;	// Start looking for Req_ID
					for(int a = 0; !found && a < ((*All_Requirements)->dimSize); a++)
					{
						//  This section will check the ReqID name and verify the vector Conditions
						//  match to determine which specific Req is being tested
						TString R_ID = (*All_Requirements)->Requirements[a].ID;

						if((stricmp(nv_ID.CStr(), R_ID.CStr()) == 0) && ConditionCheck(&Err, V_IN, D_IN,(*All_Requirements)->Requirements[a].Condition))
						{
						
							//  Requirement ID and Condition matched
							TString Units = (*All_Requirements)->Requirements[a].units;
							
							// Get the Limit Strings
							RMax = (*All_Requirements)->Requirements[a].max; // Only one value allowed
							RNom = (*All_Requirements)->Requirements[a].nom; 
							RMin = (*All_Requirements)->Requirements[a].min; // Only one value allowed

							// An Array of Nominal Values will be allowed with respect to a single Max and Min.
							// This will allow a requirement, such as with pacing, to have a Max and Min limit
							// with respect to a given chamber.  It should be noted that the parameters to be
							// tested must be in the proper order within the requirement configuration
							RNomArray = ParseString( &Err, RNom);

							//  Any limit marked with TBD causes a 'Review' to be reported
							if((stricmp(RMax.CStr(),"TBD") == 0) || (stricmp(RNom.CStr(),"TBD") == 0) || (stricmp(RMin.CStr(),"TBD") == 0))
								NoLimit = true;

							if( !Err.Status() && !NoLimit)
							{
								if(RNomArray.GetSize() != 0)
								{
									for( int RNomIndex = 0; RNomIndex < RNomArray.GetSize();  RNomIndex++)
									{
										dMax.Insert(PEMDAS(&Err, RMax, V_IN, D_IN), -1); // Calculated Max value (string to float)
										dNom.Insert(PEMDAS(&Err, RNomArray[RNomIndex], V_IN, D_IN), -1); // Calculated Nominal value (string to float)
										dMin.Insert(PEMDAS(&Err, RMin, V_IN, D_IN), -1); // Calculated Min value (string to float)
									}
														
								}
								else 
								{ 
									dMax.Insert(PEMDAS(&Err, RMax, V_IN, D_IN), -1); // Calculated Max value (string to float)
									dMin.Insert(PEMDAS(&Err, RMin, V_IN, D_IN), -1); // Calculated Min value (string to float)
								}
														
							}
							
							// Currently the only Units parameter that causes the code to perform differently is the '%'
							// character.  This allows for a precentage calculation without having to add any formulas
							// to the limits portion of the configuration.
							if( !Err.Status() && stricmp( Units.CStr(), "%") == 0 && !NoLimit) // Re-Calc Based on Nominal
							{
								// If RNom isn't empty then Max and Min are calculated with respect to the nominal
								if(!RNom.IsEmpty())
								{
									for( int RNomIndex = 0; RNomIndex < RNomArray.GetSize();  RNomIndex++)
									{
										dMax[RNomIndex] = (dMax[RNomIndex]*0.01)*dNom[RNomIndex] + dNom[RNomIndex];	// Calculate with respect to Nom
										dMin[RNomIndex] = (dMin[RNomIndex]*0.01)*dNom[RNomIndex] + dNom[RNomIndex];	// dMin must have the proper sign '+' or '-'
									}
								}
							}
							else if(!Err.Status() && !NoLimit) // Re-Calc Based on Nominal
							{
								// If RNom is empty then Max and Min are absolute values and
								// no further math is required.
								if(!RNom.IsEmpty())
								{
									for( int RNomIndex = 0; RNomIndex < RNomArray.GetSize();  RNomIndex++)
									{
										dMax[RNomIndex] = dNom[RNomIndex] + dMax[RNomIndex];	// Calculate with respect to Nom
										dMin[RNomIndex] = dNom[RNomIndex] + dMin[RNomIndex];	// dMin must have the proper sign '+' or '-'
									}
								}
							}

							ID_Match = a;	// This is the Requirements[a].ID index
							found = true;	// Indicates the Requirement ID was found
						}
					}
					
					// If the Req_ID was not found it could be because the ID in the vector didn't match
					// or the conditions did not match.  The following will capture this.
					if(!found)
						v_MisMatch.Insert(nv_ID, -1);
				
				} // end if(!nv_ID.IsEmpty() && ( stricmp( nv_ID.CStr(), "N/A") != 0 || stricmp( nv_ID.CStr(), "NA") != 0)
					
				
				//  Next grab the data (Test Parameter) and compare it against the limits
				if(!Err.Status() && ID_Match >= 0  && found) 
				{
						TString Parameter = (*All_Requirements)->Requirements[ID_Match].Test_Parameter;
						TStringArray tParameter = ParseString( &Err, Parameter); // Get Test Parameter

						if(tParameter.GetSize() > 0)
						{
							if(tParameter.GetSize() > dMax.GetSize())
							{
								for( int dM = 0; dM < tParameter.GetSize() - 1; dM++)
									dMax.Insert(dMax[0], -1);
							}

							if(tParameter.GetSize() > dMin.GetSize())
							{
								for( int dM = 0; dM < tParameter.GetSize() - 1; dM++)
									dMin.Insert(dMin[0], -1);
							}
						
							for( tpIndex = 0; tpIndex < tParameter.GetSize(); tpIndex++)
							{
								for( int b = 0; b < v_Data.GetSize(); b++)
								{
									
									TString Temp = BTWN_TAGS(&Err, "<name>", "</name>", v_Data[b]); // Search Data for Test parameter
									if(stricmp(Temp.CStr(), tParameter[tpIndex].CStr()) == 0)
									{
										TString dType = BTWN_TAGS(&Err, "<type>", "</type>", v_Data[b]);
										dvalue = PEMDAS(&Err, tParameter[tpIndex], V_IN, D_IN); // Calculated value

										// Check if Value is within limits
										if(!Err.Status() )
										{
											if(dvalue <= dMax[tpIndex] && dvalue >= dMin[tpIndex] && !NoLimit)
												dStatus = TString("Pass");
											else
												dStatus = TString("Review");
										}
									
										done = true;

										b = v_Data.GetSize(); // Got info so get out
									}
								}
				
								if(!done) // Must be a calc
								{
									dvalue = PEMDAS(&Err, tParameter[tpIndex], V_IN, D_IN); // Calculated value

									// Check if Value is within limits
									if(!Err.Status())
									{
										if(dvalue <= dMax[tpIndex] && dvalue >= dMin[tpIndex] && !NoLimit)
											dStatus = TString("Pass");
										else
											dStatus = TString("Review");
									}
								
								}

								if(!Err.Status())	// Build output
								{
									TString dReqID_Out = TString("<req_element>\r\n<req_name>");
									TString dTestParam_Out = TString("</req_name>\r\n<result_name>");
									TString dMax_Out = TString("</result_name>\r\n<req_upperlim>");
									TString dMeas_Out = TString("</req_upperlim>\r\n<req_value>");
									TString dMin_Out = TString("</req_value>\r\n<req_lowerlim>");
									TString dStatus_Out = TString("</req_lowerlim>\r\n<req_status>");
									
									DataOut += dReqID_Out;
									DataOut += nv_ID;
									DataOut += dTestParam_Out;
									DataOut += tParameter[tpIndex];
									DataOut += dMax_Out;
									TString G;
									if(!NoLimit)
										G.Format("%.12e", dMax[tpIndex]);
									else
										G = TString("TBD");

									DataOut += G;
									DataOut += dMeas_Out;
									G.Format("%.12e",dvalue);
									DataOut += G;
									DataOut += dMin_Out;
									if(!NoLimit)
										G.Format("%.12e",dMin[tpIndex]);
									else
										G = TString("TBD");

									DataOut += G;
									DataOut += dStatus_Out;
									DataOut += dStatus;
									DataOut += TString("</req_status>\r\n</req_element>\r\n\r\n");
								}
						
							}	// End Test Parameter For Loop
						}	// End Test Parameter	
					}	// End ID Match
			}	// End ReqID_Index for loop

			if(!Err.Status())
				D_IN += DataOut;	// Append Req Data to Provided In data

			if( v_MisMatch.GetSize() > 0)
			{
				DataOut = TString();
				for( int a = 0; a < v_MisMatch.GetSize(); a++)
				{
					
					TString dReqID_Out = TString("<req_element>\r\n<req_name>");
					TString dTestParam_Out = TString("</req_name>\r\n<result_name>");
					TString dMax_Out = TString("</result_name>\r\n<req_upperlim>");
					TString dMeas_Out = TString("</req_upperlim>\r\n<req_value>");
					TString dMin_Out = TString("</req_value>\r\n<req_lowerlim>");
					TString dStatus_Out = TString("</req_lowerlim>\r\n<req_status>");
									
					DataOut += dReqID_Out;
					DataOut += v_MisMatch[a];
					DataOut += dTestParam_Out;
					TString G = TString("NaN");
					DataOut += G;
					DataOut += dMax_Out;
					DataOut += G;
					DataOut += dMeas_Out;
					DataOut += G;
					DataOut += dMin_Out;
					DataOut += G;
					DataOut += dStatus_Out;
					DataOut += TString("Req or Condition MisMatch");
					DataOut += TString("</req_status>\r\n</req_element>\r\n\r\n");

				}
				
				D_IN += DataOut;
			}

			D_IN.SetLV_String(&TempErr, &ResultsOut);
	}	// End !V_IN.IsEmpty() && !D_IN.IsEmpty()
	else
		ReqmtString(&Err, R_IN).SetLV_String(&Err, &ResultsOut);

	return Err.LVErr(error);

}

#pragma warning( pop )	// Restore Warning ('identifier' : has bad storage class)
/// @}
/// @}
