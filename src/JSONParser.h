#pragma once
#include <stdio.h>

#define JSON_OPEN_OBJ '{'
#define JSON_CLOSE_OBJ '}'
#define JSON_FIELD_OPEN_CLOSE '"'
#define JSON_FIELD_END ':'
#define JSON_NEWLINE_CHAR '\n'



enum LEX_TOKEN {
	JSON_LEX_TOKEN_SYMBOL,
	JSON_LEX_TOKEN_STRINGS,
	JSON_LEX_TOKEN_NUMBER,
	JSON_LEX_TOKEN_CONTROLCHARACTER
};

enum json_error {
	JSON_LEXICAL_OK,
	JSON_LEXICAL_ERROR
};


typedef struct JSONToken {
	int LEX_TYPE;
	union {
		char* stringValue; //8bit
		char symbol;
		double value;
		


	};
} JSONToken;

typedef struct JSONLexicalResult {
	int error_type;
	char * value;
	int totalTokens;
} JSONLexicalResult;



enum JSON_TYPE {
	JSON_Number,
	JSON_String,
	JSON_BOOLEAN,
	JSON_OBJECT,
	JSON_ARRAY
};


typedef enum JSON_RESULT {
	RESULT_FIELD,
	RESULT_OBJECT
} JSON_RESULT;


typedef struct JSONField {


	int status;
	int FIELD_TYPE;
	
	char* name;


	double Field_NumberValue;
	char* Field_StringValue;
	int Field_BoolValue;


	struct JSONObject * Field_JSONObject;
	struct JSONField* Field_Array;



} JSONField;


typedef struct JSONObject {
	int status;
	char* objectName;
	int objectCount;
	size_t FIELD_SIZE;
	JSONField* fields;
} JSONObject;

typedef struct JSONManager {
	int status;
	int JSONIndex;
	JSONObject* prev;
	JSONObject* next;
	JSONObject current;

} JSONManager;

extern JSONManager manager;


typedef struct JSONStatus {
	int status;
	char* error_message;
} JSONStatus;


typedef struct JSONResult {
	int status;
	JSON_RESULT fieldType;
	union {
		JSONField* fieldVal;
		JSONObject* objectVal;
	};
}JSONResult;


JSONManager Parse_JSON(char filename[]);

void processTokens(JSONToken token, int c);



JSONObject* parseToken(JSONToken* tokens, int tokenCount);
JSONObject processObject(char* name, JSONField* fields, int fieldCount, int totalSize);

//Creation of Objects
JSONObject createNewObject(char** name);
JSONObject processObjectRecursive(JSONToken* tokens, int* count, char* name, char** stringField_Name, char** field_value, double* v, int* maxToken);

// Creation of fields
JSONField newStringField( char** name, char** value);
JSONField newValueField( char** name, double* value);
JSONField newArrayField(char** name);
JSONField parseArray(JSONToken* tokens, int* currentCount, int* count_limit, char** stringName, char** stringValue);
JSONField newObjectField(JSONObject* value, size_t objectSize);


//Allocation
JSONStatus allocateToObject(JSONObject* object, JSONField* field, int* allocationSize);
JSONStatus reallocateAndAssignArray(JSONField** FieldPointer, JSONField* field, int* fieldCount, int field_size);

//Search Tools
JSONResult findByNameRecursively(char* name, JSONObject* object);
JSONResult* findJSONData(JSONResult* inJSONResult,char* jsonString, const JSONObject* root);


//DEBUG
void DEBUG_PRINTFIELD(JSONField* field);


//MEMORY
JSONStatus cleanup(JSONObject* topLevel);
JSONStatus cleanupToken(JSONToken* tokens, int count);


//Retrieval of Data
double getResultNumberValue(JSONResult searchResult);
char* getResultStringValue(JSONResult searchResult);

//Writing of Data
char* BuildJSON(JSONObject obj);