
typedef struct {
	LStrHandle ID;
	LStrHandle Parameter_Type;
	LStrHandle units;
	LStrHandle Test_Parameter;
	LStrHandle Condition;
	LStrHandle max;
	LStrHandle nom;
	LStrHandle min;
	} Reqs_Struct;

typedef struct {
	int32 dimSize;
	Reqs_Struct Requirements[1];
	} All_Reqs;
typedef All_Reqs **All_ReqsHdl;

typedef struct {
	TString Value;
	TString Type;
	} Value_Struct;

typedef struct {
	TString Name;
	bool found;
	bool Match;
	} CondCk_Struct;

typedef struct {
	float64 Value;
	TStringArray Formula;
	} Math_Struct;

typedef struct {
	TString Type;
	TStringArray Formula;
	} PABS_Struct;

	TStringArray CacheName = TStringArray();	// Cache Names With Conditions Met
	TStringArray CacheNamed = TStringArray();  // Cache Names from File
	TStringArray CacheValue = TStringArray();
	TStringArray Parsed_ValueCache = TStringArray();

	TString R_IN;					// Requirement Data
	#define SEPMath "(+-*)/:= ;^|"	// For Math, This allows for 'funky' Chars if needed later
	#define SEPSome "+*/:= ;|"		// MathSeps removed that can be used in column names
	#define SEPCHARS ":=;,"		
	#define ParamNumber 0			// The return parameter is parameter 1
	
