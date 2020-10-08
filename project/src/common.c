#include "common.h"


/*! \brief Checks if the specified string is a number
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int isNumber(const char c) {
        return c >= '0' && c <= '9';
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int isChar(const char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); 
}







/* reads the user input */
char *getUserInput(char *buffer) {
    if (fgets(buffer, BUFSIZ, stdin) == NULL)
        FATAL("Failed to get user input!");
        
	return buffer;
}





/*! \brief Checks if the string is valid using to the match() function and size.
 *
 *  Compares each character of the string with the specified match() functiion,
 *  while also verifying if the string length. 
 *
 * \param  buffer   the buffer containing the string.
 * \param  matcher  the function to check the characters.
 * \param  forceLen the required size of the string.
 * \return the string's length if the string is valid, FALSE otherwise.
 */
int isStringValid(const char* buffer, int (*matcher)(int), int forceLen) {
    if (str == NULL)    // invalid string
        return FALSE;  

    int i = 0;
    while (str[i] != '\0' && matcher(str[i]))
        i++;

    return (forceLen == 0 || forceLen == i ? i : FALSE );
}




/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
/**
*   Function that checks all chars of a string for alfanumeric chars
*   and the string length is the same as the argument `forceLen`
*   \return String length if evalued to True
*           else False
*/
int checkAlfaNum(const char *str, int forceLen) {
        int i=0;
        if (str == NULL) return FALSE;
    
        while (*str!='\0') {
            if (!isNumber(*str) || !isChar(*str))
                return FALSE;
            ++str;
            ++i;
        }
        if (forceLen != 0 && forceLen != i)
            return FALSE;
        return i;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */

/**
*   Function that checks all chars of a string for only numbers
*   and the string length is the same as the argument `forceLen`
*   \return String length if evalued to True
*           else False
  */
int checkOnlyNum(const char *str, int forceLen) {
        int i=0;
        if (str == NULL) return FALSE;

        while (*str!='\0') {
            if (!isNumber(*str))
                return FALSE;
            ++str;
            ++i;
        }
        if (forceLen != 0 && forceLen != i)
            return FALSE;
        return i;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
/**
*   Function that checks all chars of a string for only char
*   and the string length is the same as the argument `forceLen`
*   \return String length if evalued to True
*           else False
  */
int checkOnlyChar(const char *str, int forceLen) {
        int i=0;
        if (str == NULL) return FALSE;

        while (*str!='\0') {
            if (!isChar(*str))
            return FALSE;
            ++str;
            ++i;
        }
        if (forceLen != 0 && forceLen != i)
            return FALSE;
        return i;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
/**
*   Function that checks the string for the IPv4 notation
*   "xxx.xxx.xxx.xxx" and xxx <= 255
*   \return True if valid IPv4 notation
*           else False
  */
int checkValidIp(const char *str) {
        int delim_counter=0, size, num;
        char *ptr;
        char ip_str[strlen(str)+1];

        // backup orignal string
        strcpy(ip_str, str);
    
        if (ip_str == NULL) return FALSE;
    
        ptr = strtok(ip_str, IP_DELIM);
        if (ptr == NULL) return FALSE;
        while(ptr) {
                /* between delim must contain only numbers */
                size = checkOnlyNum(ptr, 0);
                if (size < 0 && size > 3) return FALSE;

                /* check for valid IP parcel number */
                num=atoi(ptr);
                if (num >= 0 && num <= 255) {
                    /* continue spliting string */
                        ptr = strtok(NULL, IP_DELIM);
                if (ptr != NULL) ++delim_counter;
                } else
                    return FALSE;
        }
    
            if (delim_counter != 3) return FALSE;
    
        return TRUE;
}

/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
/**
*   Function that checks all chars of a string for only char
*   and the string length is the same as the argument `forceLen`
*   \return String length if evalued to True
*           else False
*/
int checkValidPORT(const char *str) {
    int val;
    if(checkOnlyNum((const char*) str, 0)){
        val = atoi(str);
        return val >= 1000 && val <= 65535;
    }

    return FALSE;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int isValidUID(const char *input) {
	    return checkOnlyNum(input, UID_SIZE);
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int isValidPassword(char *input) {
        return checkAlfaNum(input, PASS_SIZE);
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
/*int checkFlag(const char *flag, const char *input) {
	if (!strcmp(PDPORTARG, flag)) {
		
	} else if (!strcmp(ASIPARG, flag)) {
		
	} else if (!strcmp(ASPORTARG, flag)) {
		
	} else if (!strcmp(FSIPARG)) {
		
	} else if (!strcmp(FSPORTARG)) {
		
	} else if (!strcmp(PDPORTARG)) {
		
	} else if (!strcmp(VERBOSE)) {
		
	} else {
		
	}
}*/






/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
void fatal(const char *message) {
        fprintf(stderr, "\033[1;31m[FATAL]: \33[0m%s\n", message);
        exit(EXIT_FAILURE);
}