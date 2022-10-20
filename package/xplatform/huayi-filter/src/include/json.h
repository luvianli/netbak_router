#ifndef VSF_JSON_H
#define VSF_JSON_H

typedef struct cJSON {
	struct cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==cJSON_String */
	int valueint;				/* The item's number, if type==cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJSON;

extern void cJSON_Delete(cJSON *c);
extern cJSON *cJSON_Parse(const char *value);
extern int cJSON_GetArraySize(cJSON *array);
extern cJSON *cJSON_GetArrayItem(cJSON *array,int item);
extern char *cJSON_GetStrVar(cJSON *object, char *var, char *defaultGetValue);
extern cJSON *cJSON_GetObjectItem(cJSON *object,const char *string);
#endif
