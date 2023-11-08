
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "JSONParser.h"




JSONLexicalResult lexical_analysis(JSONToken** outTokens, FILE* readStream) {
	//Read JSON to end of file
	JSONToken* ptr;

	double total = NAN;
	int sign = 1;
	int division = 10;
	char c = 0;
	bool isReadingString = false;
	bool isReadingValue = false;
	bool isReadingDecimal = false;
	int tokenCount = 0;
	int stringCount = 0;
	size_t tokenMemory = 0;
	size_t charSize = 1;
	char* field = NULL;

	while ((c = fgetc(readStream)) != EOF) {
		if (c == '\n') {
			if (isReadingValue == true || isReadingDecimal == true) {
				isReadingDecimal = isReadingValue = false;
			}
		}
		if ((c == ' ') && !isReadingString) {
			continue;
		}
		JSONToken token = { 0 };
		//if current character is not eof
		if ((c < '0' || c > '9') && (c != '.')) {
			//c is not number and no decimal
			isReadingValue = false;
		}
		if (c == '-' && isReadingString == false) {
			//must be a number
			sign = -1;
		}
		else if (c == '.') {
			isReadingDecimal = true;
		}
		if (c == JSON_OPEN_OBJ || c == JSON_CLOSE_OBJ) {
			//store in array
			token.LEX_TYPE = JSON_LEX_TOKEN_SYMBOL;
			token.symbol = c;


		}
		else if (c == JSON_NEWLINE_CHAR) {
			token.LEX_TYPE = JSON_LEX_TOKEN_CONTROLCHARACTER;
			token.symbol = c;
		}
		else if (c == JSON_FIELD_OPEN_CLOSE) {
			token.LEX_TYPE = JSON_LEX_TOKEN_SYMBOL;
			token.symbol = c;
			isReadingString = !isReadingString;
			if (field != NULL) {
				field[stringCount] = '\0';
			}





		}
		else if (c == JSON_FIELD_END) {
			token.LEX_TYPE = JSON_LEX_TOKEN_SYMBOL;
			token.symbol = c;




		}
		else if (c == '[' || c == ']') {
			token.LEX_TYPE = JSON_LEX_TOKEN_SYMBOL;
			token.symbol = c;
		}
		else if ((c == ',' || c == ';') && isReadingString == false) {
			token.LEX_TYPE = JSON_LEX_TOKEN_SYMBOL;
			token.symbol = c;
			//isReadingValue = isReadingValue == true ? false : isReadingValue;

		}
		else if ((c >= '0' && c <= '9') && isReadingString == false) {
			token.LEX_TYPE = JSON_LEX_TOKEN_NUMBER;
			isReadingValue = true;

		}
		else if (isReadingString) {
			stringCount++;
			field = (field == NULL) ? calloc(2, 1) : field;
			char* ptr = realloc(field, stringCount + 1);
			if (ptr != NULL) {
				ptr[stringCount - 1] = c;
				field = ptr;
			}
			else {
				free(field);
				ptr = NULL;
				field = NULL;
				JSONLexicalResult err = { 0 };
				err.error_type = JSON_LEXICAL_ERROR;
				err.value = "Lexical Error: Reallocation Ptr is NULL!";
				return err;
			}
		}
		else if (c == '.' && isReadingValue == true) {
			isReadingDecimal = true;
		}
		else {
			isReadingValue = isReadingValue == true ? false : isReadingValue;
			isReadingDecimal == isReadingValue ? isReadingValue : false;
		}
		if (isReadingValue && c != '.') {
			//parse c
			total = isnan(total) == 1 ? 0 : total;
			double val = c - '0';
			if (isReadingDecimal) {
				total = total + val / division;
				division *= 10;
			}
			else {
				total = total * 10 + val;
			}

		}
		else if (!isReadingValue && isnan(total) != 1) {
			//Do stuff
			token.LEX_TYPE = JSON_LEX_TOKEN_NUMBER;
			token.value = sign * total;
			size_t size = sizeof(token);
			tokenMemory += size;
			tokenCount++;
			ptr = realloc((*outTokens), tokenMemory);
			if (ptr == NULL) {
				free(ptr);
				JSONLexicalResult err = { 0 };
				err.error_type = JSON_LEXICAL_ERROR;
				err.value = "Lexical Error: Reallocation Ptr is NULL!";
				return err;
			}
			else {
				//allocate memory
				*outTokens = ptr;

				(*outTokens)[tokenCount - 1] = token;





			}
			total = NAN;
		}
		else if (!isReadingString && !isReadingValue) {
			//Check if there is string
			if (field != NULL) { // C should be "
				token.LEX_TYPE = JSON_LEX_TOKEN_STRINGS;
				token.stringValue = calloc(stringCount + 2, 1);
				if (token.stringValue != NULL) {
					errno_t  stringCpy = strcpy_s(token.stringValue, stringCount + 2, field);
					if (stringCpy == 0) {
						free(field);
						field = NULL;
						stringCount = 0;
					}
					else {
						printf("Error");
						free(field);
						free(token.stringValue);
					}
				}
			}

			size_t size = sizeof(token);
			tokenMemory += size;
			tokenCount++;
			ptr = realloc((*outTokens), tokenMemory);
			if (ptr == NULL) {
				free(ptr);
				JSONLexicalResult err = { 0 };
				err.error_type = JSON_LEXICAL_ERROR;
				err.value = "Lexical Error: Reallocation Ptr is NULL!";
				return err;
			}
			else {
				//allocate memory
				*outTokens = ptr;

				(*outTokens)[tokenCount - 1] = token;
				//printf("%c", (*outTokens)[tokenCount - 1].symbol);

			}
		}




	}




	JSONLexicalResult success = { 0 };
	success.error_type = 0;
	success.value = "Good!";
	success.totalTokens = tokenCount;
	return success;
}





JSONManager Parse_JSON(char filename[]) {
	FILE* jsonFile;
	JSONObject obj = { 0 };
	JSONToken* tokens = calloc(1, sizeof(JSONToken));
	JSONManager manager = { 0 };
	fopen_s(&jsonFile, filename, "r");
	if (jsonFile != 0) {
		if (tokens != NULL) {
			JSONLexicalResult result = lexical_analysis(&tokens, jsonFile);
			


			if (result.error_type == 0) {
				printf("\n=========== RAW JSON (DEBUG) =========================\n");
				for (int i = 0; i < result.totalTokens; i++) {
					processTokens(tokens[i], i);
					
				}
				obj = *(parseToken(tokens, result.totalTokens));
				
				//JSONResult res = findByNameRecursively("para",  &obj);
				/*JSONResult res = findJSONData("Player.Gold", &obj);
				
				if (res.status == 1) {
					if (res.fieldType == RESULT_FIELD) {
						DEBUG_PRINTFIELD(res.fieldVal);
					}
				}*/

				//cleanup(&obj);
				

				
				

			}

			cleanupToken(tokens, result.totalTokens);
		}
		fclose(jsonFile);
		manager.status = 1;
		manager.current = obj;
		tokens = NULL;
		return manager;
		

	}

	
	tokens = NULL;
	return manager;





}

JSONResult* findJSONData(JSONResult *inJSONResult,char* jsonString, const JSONObject* root) {
	char* delimiter = ".";
	char* token = NULL;
	char* next_token = NULL;
	size_t s = strnlen_s(jsonString, CHAR_MAX * 3);
	char writable[500] = { '\0' };

	for (int i = 0; i < s+1; i++) {
		writable[i] = *(jsonString + i);
	}



	token = strtok_s(writable, delimiter, &next_token);
	while (token[0] != '\0') {
		int found = 0;
		for (int i = 0; i < root->objectCount; i++) {
			JSONField field = *(root->fields + i);
			if (strcmp(field.name,token) == 0 && next_token != NULL) {
				if (next_token != NULL && field.FIELD_TYPE == JSON_OBJECT) {
					found = 1;
					root = field.Field_JSONObject;
					token = strtok_s(NULL, delimiter, &next_token);
					break;
				}
				else if (next_token[0] != '\0' && field.FIELD_TYPE != JSON_OBJECT) {
					//err
					inJSONResult->status = 2;
					inJSONResult->fieldType = RESULT_FIELD;
					inJSONResult->fieldVal = (root->fields + i);
					return inJSONResult;
				}
				else if (next_token[0] == '\0' && field.FIELD_TYPE == JSON_OBJECT) {
					inJSONResult->status = 1;
					inJSONResult->fieldType = RESULT_OBJECT;
					inJSONResult->fieldVal = (root->fields + i);
					return inJSONResult;
				}
				else if (next_token[0] == '\0' && (field.FIELD_TYPE <= JSON_BOOLEAN)) {
					inJSONResult->status++;
					inJSONResult->fieldType = RESULT_FIELD;
					inJSONResult->fieldVal = (root->fields + i);
					return inJSONResult;
				}
			}
		}
		if (!found) {
			break;
		}
		
	}
	printf("Find: No result\n");
	inJSONResult->status = 0;
	return inJSONResult;
	
}

JSONResult findByNameRecursively(char* name, const JSONObject* object) {
	JSONResult result = { 0 };
	JSONField* fields = object->fields;
	int count = object->objectCount;
	if (fields != NULL) {
		for (int i = 0; i < count; i++) {
			JSONField field = object->fields[i];
			if (field.name != NULL) {
				if (strcmp(field.name, name) == 0) {
					result.status = 1;
					if (field.FIELD_TYPE != JSON_OBJECT) {
						result.fieldType = RESULT_FIELD;
						result.fieldVal = &field;
						return result;
					}
					else {
						result.fieldType = RESULT_OBJECT;
						result.objectVal = &field;
						return result;
					}

				}
				if (field.FIELD_TYPE == JSON_OBJECT) {
					//recursion
					JSONResult subResult = findByNameRecursively(name, field.Field_JSONObject);
					if (subResult.status == 1) {
						return subResult;
					}
					else {
						continue;
					}

				}
			}
		}
	}
	result.status = 0;
	return result;
}

JSONStatus cleanupToken(JSONToken * tokens, int count) {
	for (int i = 0; i < count; i++) {
		if (tokens[i].LEX_TYPE == JSON_LEX_TOKEN_STRINGS) {
			tokens[i].stringValue = NULL;
		}
		else {
			tokens[i].symbol = '\0';
			tokens[i].value = 0.0;
		}
	}

	free(tokens);
	tokens = NULL;
}


JSONStatus cleanup(JSONObject* topLevel) {

	if (topLevel->objectName != NULL) {
		const char* root = "ROOT";
		printf("\nCleaning %s\n", topLevel->objectName);
		if (strcmp(topLevel->objectName, root) != 0) {
			free(topLevel->objectName);
			topLevel->objectName = NULL;
		}
	}

	if (topLevel->fields != NULL) {
		for (int i = 0; i < topLevel->objectCount; i++) {
			JSONField* fd = topLevel->fields + i;
			if (fd->name != NULL) {
				if (fd->FIELD_TYPE != JSON_OBJECT) {
					printf("\n\tCleaning Field %s\n", fd->name);
					free(fd->name);
					fd->name = NULL;
				}
				else {
					printf("\n\tCleaning Field %s\n", fd->name);
				}
				
			}
			if (fd->Field_StringValue != NULL) {
				free(fd->Field_StringValue);
				fd->Field_StringValue = NULL;
			}
			if (fd->Field_JSONObject != NULL) {
				cleanup(fd->Field_JSONObject);
				free(fd->Field_JSONObject);
				fd->Field_JSONObject = NULL;

			}
		}
		free(topLevel->fields);
		topLevel->fields = NULL;
		topLevel->FIELD_SIZE = 0;
		topLevel->objectCount = 0;

	}
}

void processTokens(JSONToken token, int c) {
	switch (token.LEX_TYPE) {
	case JSON_LEX_TOKEN_STRINGS:
		printf("Token %d, \"%s\"(-)\n", c, token.stringValue);
		break;
	case JSON_LEX_TOKEN_SYMBOL:
		printf("Token %d, \"%c\"(%d)\n", c, token.symbol, (int)token.symbol);
		break;
	case JSON_LEX_TOKEN_NUMBER:
		printf("Token %d, %.2lf\n", c, token.value);
		break;
	case JSON_LEX_TOKEN_CONTROLCHARACTER:
		printf("Token %d, %c(%d)\n", c, token.symbol, (int)token.symbol);

	}
}


void DEBUG_PRINTFIELD(JSONField* field) {
	int type = field->FIELD_TYPE;

	switch (type) {
	case JSON_String:
		printf("{%s : %s}", field->name, field->Field_StringValue);
		break;
	case JSON_Number:
		printf("{%s : %.2f}", field->name, field->Field_NumberValue);
		break;
	}

}




JSONObject* parseToken(JSONToken* tokens, int tokenCount) {
	//set syntax flag
	printf("\n=================Parse Started ======================\n");
	JSONManager manager = { 0 };
	int fieldAllocation = 0;
	JSONObject currentObject = { 0 };
	currentObject.objectName = "ROOT";
	currentObject.fields = NULL;
	double Number = -NAN;
	bool inJSON = false;
	bool InArray = 0;
	bool InObject = 0;
	bool isField = 0;
	bool isValue = 0;
	bool inArray = false;
	char* stringField = NULL;
	char* value = NULL;
	JSONToken* token;

	size_t fieldSize = 0;
	for (int i = 0; i < tokenCount; i++) {
		JSONField jsonField = { 0 };
		token = &tokens[i];
		int type = token->LEX_TYPE;

		if (type == JSON_LEX_TOKEN_SYMBOL) {
			if (tokens[i].symbol == JSON_OPEN_OBJ || tokens[i].symbol == JSON_CLOSE_OBJ) {
				if (inJSON == false) {
					inJSON = true;
					continue;
				}
				if (tokens[i].symbol == JSON_OPEN_OBJ) {
					InObject = true;
				}
				else if (tokens[i].symbol == JSON_CLOSE_OBJ) {
					InObject = false;
				}

			}
			else if (tokens[i].symbol == '[' || tokens[i].symbol == ']') {
				inArray = !inArray;
			}
		}
		else if (type == JSON_LEX_TOKEN_STRINGS) {
			if (!inArray) {
				if (stringField == NULL) {
					isField = true;
					isValue = false;
				}
				else if (value == NULL) {
					isValue = true;
					isField = false;

				}
			}




		}
		else if (type == JSON_LEX_TOKEN_CONTROLCHARACTER) {
			continue;
		}
		else if (type == JSON_LEX_TOKEN_NUMBER && inArray == false) {
			if (stringField != NULL) {
				isValue = true;
				Number = token->value;

				if (stringField != NULL && !isnan(Number)) {
					//If there is a field, numberValue but no string value

					jsonField = newValueField(&stringField, &Number);
					jsonField.FIELD_TYPE = JSON_Number;
					fieldSize = sizeof(jsonField);
					JSONStatus status = allocateToObject(&currentObject, &jsonField, &fieldSize);
					if (status.status == 0) {
						//do stuff after allocation
						printf("Allocation %d fields to (%s) \n", currentObject.objectCount, currentObject.objectName);
					}
					else {
						printf("Error");
						return;
					}



				}

			}
			else isValue = false;
		}




		if (type == JSON_LEX_TOKEN_STRINGS) {
			if (isField) {
				if (stringField == NULL) {
					stringField = token->stringValue;
					token->stringValue = NULL;

				}
			}

			if (isValue) {
				if (value == NULL) {
					value = token->stringValue;
					token->stringValue = NULL;
				}
			}


			if (value != NULL && stringField != NULL) {
				jsonField = newStringField(&stringField, &value);
				fieldSize = sizeof(jsonField);
				JSONStatus status = allocateToObject(&currentObject, &jsonField, &fieldSize);
				if (status.status == 0) {
					//do stuff after allocation
					printf("Allocated %d fields (%s)\n", currentObject.objectCount, currentObject.objectName);
				}
				else {
					printf("Error");
					return;
				}






			}





		}
		else if (inArray && stringField != NULL) {
			printf("=========ENTER ARRAY ==============\n");
			JSONField arrayField = parseArray(tokens, &i, &tokenCount, &stringField, &value);
			printf("=========EXIT ARRAY ==============\n");
			inArray = false;
		}
		else if (InObject == true && stringField != NULL) {

			//We're in an object and there a name for it
			printf("============== START Creation of SubObjects {%s} =================\n", currentObject.objectName);
			JSONObject subObject = processObjectRecursive(tokens, &i, stringField, &stringField, &value, &Number, &tokenCount);
			JSONField  oField = newObjectField(&subObject, 40);
			size_t stField = sizeof oField;
			JSONStatus aStatus = allocateToObject(&currentObject, &oField, &stField);
			if (aStatus.status == 0) {
				printf("Allocated %d field to object (%s)\n", currentObject.objectCount, oField.Field_JSONObject->objectName);
			}
			else {
				printf("%s", aStatus.error_message);
			}

			InObject = false;





			printf("============== STOP Creation of SubObjects {%s} =================\n", currentObject.objectName);
			continue;
		}






	}
	currentObject.status = 0;
	printf("\n=================Parse ENDED ======================\n");
	return &currentObject;
}


// Process sub objects
JSONObject processObject(char* name, JSONField* fields, int fieldCount, int totalSize) {
	JSONObject obj = { 0 };
	obj.fields = NULL;
	for (int i = 0; i < fieldCount; i++) {
		JSONField field = fields[i];
		if (obj.fields = NULL) {
			obj.fields = calloc(1, sizeof totalSize);
			if (obj.fields != NULL) {
				obj.objectName = name;
				obj.fields[i] = fields[i];
			}
			else {
				printf("ERROR");
			}
		}
	}
	return obj;
}


JSONField newStringField(char** name, char** value) {
	JSONField tField = { 0 };
	size_t nameLen = strnlen_s(*name, INT_MAX);
	size_t valueLen = strnlen_s(*value, INT_MAX);

	tField.name = calloc(nameLen + 1, 1);
	tField.Field_StringValue = calloc(valueLen + 1, 1);

	if (tField.name != NULL && tField.Field_StringValue != NULL) {
		tField.FIELD_TYPE = JSON_String;
		errno_t nameStatus = strncpy_s(tField.name, nameLen + 1, *name, nameLen + 10);
		errno_t valueStatus = strncpy_s(tField.Field_StringValue, valueLen + 1, *value, valueLen + 10);
		if (nameStatus + valueStatus == 0) {
			printf("Created New Field : { %s : %s }\n", tField.name, tField.Field_StringValue);
			free(*name);
			free(*value);
			*name = NULL;
			*value = NULL;
			return tField;
		}
		else {
			printf("Error.");
			tField.status = 1;
			free(*name);
			free(*value);
			free(tField.Field_StringValue);
			free(tField.name);
			tField.name = NULL;
			tField.Field_StringValue = NULL;
			*name = NULL;
			*value = NULL;
			return tField;
		}
	}
}



JSONField newObjectField(JSONObject* value, size_t size) {
	JSONField tField = { 0 };
	tField.name = value->objectName;
	tField.FIELD_TYPE = JSON_OBJECT;
	JSONObject* val = malloc(size);
	if (val != NULL) {

		errno_t fieldPtr = memcpy_s(val, size, value, size);
		if (fieldPtr == 1) {
			free(val);
			free(fieldPtr);
			fieldPtr = NULL;
			val = NULL;

		}
		else {
			tField.Field_JSONObject = val;

		}
	}
	//tField.Field_JSONObject = value;
	printf("Created New Field : { %s : (%s) }\n", tField.name, tField.Field_JSONObject->objectName);
	return tField;
}





JSONField newStringFieldArray(char** name, char** value) {
	JSONField tField = { 0 };

	tField.name = NULL;
	tField.Field_StringValue = NULL;
	size_t nameLen = strnlen_s(*name, INT_MAX);
	size_t valueLen = strnlen_s(*value, INT_MAX);

	tField.name = calloc(nameLen + 1, 1);
	tField.Field_StringValue = calloc(valueLen + 1, 1);

	if (tField.name != NULL && tField.Field_StringValue != NULL) {
		tField.FIELD_TYPE = JSON_String;
		errno_t nameStatus = strncpy_s(tField.name, nameLen + 1, *name, nameLen + 10);
		errno_t valueStatus = strncpy_s(tField.Field_StringValue, valueLen + 1, *value, valueLen + 10);
		if (nameStatus + valueStatus == 0) {
			printf("(ARRAY) Created New Array Field : { %s : %s }\n", tField.name, tField.Field_StringValue);
			free(*value);
			*value = NULL;
			return tField;
		}
		else {
			printf("Error.");
			tField.status = 1;
			free(*name);
			free(*value);
			free(tField.Field_StringValue);
			free(tField.name);
			tField.name = NULL;
			tField.Field_StringValue = NULL;
			*name = NULL;
			*value = NULL;
			return tField;
		}
	}
}




JSONField newValueField(char** name, double* value) {
	//currentObject->objectCount++;
	JSONField field = { 0 };
	size_t nameLen = strnlen_s(*name, INT_MAX);

	char* namePtr = calloc(nameLen + 1, 1);



	if (namePtr != NULL) {
		field.name = namePtr;
		namePtr = NULL;
		field.FIELD_TYPE = JSON_Number;
		errno_t nameStatus = strncpy_s(field.name, nameLen + 1, *name, nameLen + 1);
		if (nameStatus == 0) {

			field.Field_NumberValue = *value;
			*value = -NAN;
			free(*name);
			*name = NULL;
			return field;
		}
		else {
			printf("Error.");
			free(*name);
			free(namePtr);
			*name = NULL;
			*value = -NAN;
			return field;
		}
	}
	else if (namePtr == NULL) {
		printf("ERROR!");
		free(*name);
		*name = NULL;

	}
}

//Allocate a field to a object
JSONStatus allocateToObject(JSONObject* object, JSONField* field, int* allocationSize) {
	object->FIELD_SIZE = *allocationSize * (object->objectCount + 1);
	printf("Added size %d\n", *allocationSize);
	if (object->fields == NULL) {
		JSONField* fieldPtr = calloc(1, *allocationSize + 10);
		if (fieldPtr != NULL) {
			fieldPtr[object->objectCount] = *field;
			object->fields = fieldPtr;
			object->objectCount++;
			fieldPtr = NULL;
			JSONStatus stats = { 0 };
			stats.status = 0;
			return stats;
		}
		else {
			free(fieldPtr);
			JSONStatus status = { 0 };
			status.status = 1;
			status.error_message = "Unable to allocate the object";

		}


	}
	else {
		JSONField* fieldPtr = realloc(object->fields, object->FIELD_SIZE);
		if (fieldPtr != NULL) {
			fieldPtr[object->objectCount] = *field;
			object->fields = fieldPtr;
			fieldPtr = NULL;
			object->objectCount++;
			JSONStatus stats = { 0 };
			stats.status = 0;
			return stats;
		}
		else {
			JSONStatus status = { 0 };
			status.status = 1;
			status.error_message = "Something went wrong during allocation.\n";
			free(object->fields);
			return status;

		}
	}



}

JSONObject createNewObject(char** name) {
	JSONObject newObject = { 0 };

	size_t nameLen = strnlen_s(*name, INT_MAX);

	if (nameLen != 0) {
		char* ptr = calloc(nameLen + 1, 1);

		if (ptr != NULL) {

			errno_t err = strncpy_s(ptr, nameLen + 1, *name, nameLen + 10);
			if (err != 0) {
				printf("ERROR");
				free(ptr);
				free(*name);
				*name = NULL;
				ptr = NULL;
			}
			else {
				free(*name);
				*name = NULL;
				newObject.objectName = ptr;
				ptr = NULL;

				return  newObject;
			}

		}
		else {
			free(ptr);
			free(newObject.objectName != NULL ? newObject.objectName : NULL);
		}
	}
}


JSONObject processObjectRecursive(JSONToken* tokens, int* count, char* name, char** stringField_Name, char** field_value, double* v, int* maxToken) {
	JSONToken token = tokens[*count];



	//--Flags

	bool isNumber = false;
	bool isStringValue = false;
	bool isStringField = false;

	bool isArray = false;
	JSONObject subObject = createNewObject(stringField_Name);
	char* d = subObject.objectName;
	int allocSize = 0;



	for (;;) {
		token = tokens[*count];
		if (token.LEX_TYPE == JSON_LEX_TOKEN_SYMBOL) {
			if (token.symbol == JSON_CLOSE_OBJ) {
				(*count) += 1;
				return subObject;
			}
			else if (token.symbol == JSON_OPEN_OBJ) {
				if (*stringField_Name != NULL) {
					(*count) += 1;
					printf("====================={ENTER FROM : %s to (%s)}================\n", subObject.objectName, *stringField_Name);
					JSONObject  subresult = processObjectRecursive(tokens, count, subObject.objectName, stringField_Name, field_value, v, maxToken);
					JSONField objectField = newObjectField(&subresult, sizeof subresult);
					size_t fieldSz = sizeof objectField;
					JSONStatus status = allocateToObject(&subObject, &objectField, &fieldSz);
					if (status.status == 0) {
						JSONObject o = subObject;
						printf("Allocated %d fields to object (%s)\n", subObject.objectCount, subObject.objectName);

					}

					printf("====================={START EXITING: %s to (%s)}================\n", subresult.objectName, subObject.objectName);
					continue;
				}
			}
			else if (token.symbol == '[' || token.symbol == ']') {
				isArray = !isArray;

			}
		}
		else if (token.LEX_TYPE == JSON_LEX_TOKEN_STRINGS && !isArray) {
			if (*stringField_Name != NULL) {
				isStringValue = true;
				isStringField = false;
				*field_value = token.stringValue;
			}
			else {
				isStringField = true;
				isStringValue = false;
				*stringField_Name = token.stringValue;
			}
		}
		else if (token.LEX_TYPE == JSON_LEX_TOKEN_NUMBER) {
			isNumber = true;
			if (isnan(*v)) {
				*v = token.value;

			}
		}
		if (isArray == true) {
			parseArray(tokens, count, maxToken, stringField_Name, field_value);
		}
		if (*stringField_Name != NULL && isNumber == true && !isnan(*v)) {
			JSONField numField = newValueField(stringField_Name, v);
			if (numField.status == 0) {
				printf("Created new field {%s : %lf } in %s (Object)", numField.name, numField.Field_NumberValue, subObject.objectName);
			}
			allocSize = sizeof(numField);
			JSONStatus status = allocateToObject(&subObject, &numField, &allocSize);

			if (status.status == 0) {
				isNumber = false;
				printf("Allocated %d fields to Object (%s)\n", subObject.objectCount, subObject.objectName);
			}
		}
		else if (*stringField_Name != NULL && *field_value != NULL && isNumber == false) {
			JSONField stringField = newStringField(stringField_Name, field_value);
			allocSize = sizeof(stringField);
			if (stringField.status == 0) {
				printf("Created new field {%s : %s } in %s (Object)", stringField.name, stringField.Field_StringValue, subObject.objectName);
			}
			JSONStatus status = allocateToObject(&subObject, &stringField, &allocSize);
			if (status.status == 0) {
				JSONObject obk = subObject;
				printf("Allocated %d fields to Object (%s)\n", subObject.objectCount, subObject.objectName);
			}
		}
		if (*count < *maxToken) {
			(*count)++;
		}
		else return subObject;
	}
	return subObject;
}



JSONField parseArray(JSONToken* tokens, int* currentCount, int* count_limit, char** stringName, char** stringValue) {
	JSONToken token = tokens[*currentCount];
	bool isValue = false;
	bool isString = false;
	int arrayCount = 0;

	char* counterName = NULL;
	if (counterName == NULL) {
		size_t lenS = strnlen_s(*stringName, INT_MAX);
		if (lenS > 0) {
			char* nPtr = calloc(lenS + 1, 1);
			if (nPtr != NULL) {
				errno_t error = strcpy_s(nPtr, lenS + 1, *stringName);
				if (error == 0) {
					counterName = nPtr;
					nPtr = NULL;
				}
				else {
					free(nPtr);
					nPtr = NULL;
					counterName = NULL;
				}
			}
		}
	}
	JSONField arrayField = newArrayField(stringName);
	JSONField stringField = { 0 };
	bool isEmpty = true;
	if (arrayField.status == 0) {
		while (*currentCount < *count_limit) {
			token = tokens[*currentCount];
			if (token.LEX_TYPE == JSON_LEX_TOKEN_SYMBOL) {
				if (token.symbol == ']') {

					free(counterName);
					counterName = NULL;
					arrayField.status = 0;
					return arrayField;
				}
			}
			else if (token.LEX_TYPE == JSON_LEX_TOKEN_STRINGS) {
				if (isEmpty) {
					*stringValue = token.stringValue;
					JSONField stringField = newStringFieldArray(&counterName, stringValue);
					arrayField.Field_Array[arrayCount++] = stringField;
					isEmpty = false;
				}
				else {
					*stringValue = token.stringValue;
					size_t fieldSize = sizeof stringField;
					JSONField stringField = newStringFieldArray(&counterName, stringValue);
					JSONStatus stats = reallocateAndAssignArray(&(arrayField.Field_Array), &stringField, &arrayCount, fieldSize);
				}



			}


			(*currentCount)++;
		}

		free(counterName);
		counterName = NULL;
	}
	free(counterName);
	counterName = NULL;




}

JSONField newArrayField(char** name) {
	//currentObject->objectCount++;
	JSONField field = { 0 };
	size_t nameLen = strnlen_s(*name, INT_MAX);


	field.name = calloc(nameLen + 1, 1);


	if (field.name != NULL) {
		field.FIELD_TYPE = JSON_String;
		errno_t nameStatus = strncpy_s(field.name, nameLen + 1, *name, nameLen + 10);
		if (nameStatus == 0) {



			free(*name);
			*name = NULL;
			JSONField* ptr = calloc(1, sizeof(field));
			size_t fieldSizes = sizeof field;
			if (ptr != NULL) {
				field.Field_Array = ptr;
				ptr = NULL;
				field.status = 0;
				return field;
			}
			else {
				free(ptr);
				free(field.Field_Array);
				field.status = 1;
				return field;
			}
		}
		else {
			printf("Error.");
			free(*name);
			*name = NULL;
			field.status = 1;
			return field;
		}
	}
}


JSONStatus reallocateAndAssignArray(JSONField** FieldPointer, JSONField* field, int* fieldCount, int field_size) {
	if (FieldPointer != NULL) {
		JSONField* FieldPtr = realloc(*FieldPointer, (*fieldCount) * field_size);
		if (FieldPtr != NULL) {
			FieldPtr[*fieldCount] = *field;
			*FieldPointer = FieldPtr;
			FieldPtr = NULL;
			JSONStatus newStatus = { 0 };
			newStatus.status = 0;
			(*fieldCount)++;
			return newStatus;
		}
		else {
			free(*FieldPointer);
			free(FieldPtr);
			*FieldPointer = NULL;
			FieldPtr = NULL;
			JSONStatus newStatus = { 0 };
			newStatus.status = 1;
			return newStatus;
		}
	}
}


double getResultNumberValue(JSONResult* searchResult) {
	if (searchResult->status != 0) {
		if (searchResult->fieldType == RESULT_FIELD) {
			JSONField* field = searchResult->fieldVal;
			if (field->FIELD_TYPE == JSON_Number) {
				return field->Field_NumberValue;
			}
		}
	}
	return 0.00;
}

char * getResultStringValue(JSONResult* searchResult) {
	if (searchResult->status != 0) {
		if (searchResult->fieldType == RESULT_FIELD) {
			JSONField* field = searchResult->fieldVal;
			if (field->FIELD_TYPE == JSON_String) {
				return field->Field_StringValue;
			}
		}
	}
	return NULL;
}




char* buildJSON(JSONObject obj) {
	return;
}