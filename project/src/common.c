#include "common.h"



// initializes the program termination signals
void initSignal(void *handlerSucc, void *handlerUn){
	struct sigaction actSucc, actUn;

	// prepare the success sigaction set
	if (sigemptyset(&actSucc.sa_mask))
		_FATAL("Unable to initialize the success termination signal set!\n\t - Error Code: %d", errno);
	if (sigaddset(&actSucc.sa_mask, SIGINT))        // user interactive termination 
		_FATAL("Unable to add the SIGINT signal to the success termination set!\n\t - Error code: %d", errno);
	if (sigaddset(&actSucc.sa_mask, SIGTERM))       // standart termination 
		_FATAL("Unable to add the SIGTERM signal to the success termination set!\n\t - Error code: %d", errno);
	
	// prepare the unsuccess sigaction set
	if (sigemptyset(&actUn.sa_mask))
		_FATAL("Unable to initialize the unsuccess signal set!\n\t - Error Code: %d", errno);
	if (sigaddset(&actUn.sa_mask, SIGABRT))         // fatal termination 
		_FATAL("Unable to add the SIGABRT signal to the unsucess termination set!\n\t - Error code: %d", errno);
	if (sigaddset(&actUn.sa_mask, SIGFPE))          // erroneous arithmetic operation
		_FATAL("Unable to add the SIGFPE signal to the unsucess termination set!\n\t - Error code: %d", errno);
	if (sigaddset(&actUn.sa_mask, SIGILL))          // invalid function image
		_FATAL("Unable to add the SIGILL signal to the unsucess termination set!\n\t - Error code: %d", errno);
	if (sigaddset(&actUn.sa_mask, SIGSEGV))         // segementation fault
		_FATAL("Unable to add the SIGSEGV signal to the unsucess termination set!\n\t - Error code: %d", errno);

	actSucc.sa_flags     = SA_SIGINFO;
	actSucc.sa_sigaction = handlerSucc;
	actUn.sa_flags       = SA_SIGINFO;
	actUn.sa_sigaction   = handlerUn;

	// change the actions of the signals to the one specified by the sets
	if (sigaction(SIGINT, &actSucc, NULL)) 
		_FATAL("Unable to change ht SIGINT action!\n\t - Error Code: %d", errno);
	if (sigaction(SIGTERM, &actSucc, NULL)) 
		_FATAL("Unable to change ht SIGTERM action!\n\t - Error Code: %d", errno);
	if (sigaction(SIGABRT, &actUn, NULL)) 
		_FATAL("Unable to change ht SIGABRT action!\n\t - Error Code: %d", errno);
	if (sigaction(SIGFPE, &actUn, NULL)) 
		_FATAL("Unable to change ht SIGFPE action!\n\t - Error Code: %d", errno);
	if (sigaction(SIGILL, &actUn, NULL)) 
		_FATAL("Unable to change ht SIGILL action!\n\t - Error Code: %d", errno);
	if (sigaction(SIGSEGV, &actUn, NULL)) 
		_FATAL("Unable to change ht SIGSEGV action!\n\t - Error Code: %d", errno);
}



// checks if the string is valid using to the match() function and size
size_t isStringValid(const char* buffer, int (*matcher)(int), int forceLen) {
	if (!buffer)            // invalid string
		return FALSE;  

	int i = 0;
	while (buffer[i] != '\0')
		if (!matcher((int)buffer[i++]))
			return FALSE;

	return (forceLen == 0 || forceLen == i ? i : FALSE);
}



/*! \brief Checks if the IP address block's format is valid.
 *
 *  Verifies if the block is a number between IP_BLOCK_MIN and IP_BLOCK_MAX.
 *
 * \param  buffer   the buffer containing the ip address block.
 * \return TRUE if the IP address block's format is valid, FALSE otherwise.
 */
static bool_t _isIPBlockValid(const char *buffer) {
	if (isStringValid(buffer, STR_DIGIT, STR_ALLLEN)) {
		int val = atoi(buffer);
		return val >= IP_BLOCK_MIN && val <= IP_BLOCK_MAX;
	}

	return FALSE;
}


// checks if the ip address format is valid (in dot notaion)
bool_t isIPValid(const char *buffer) {
	static const char *delimChars = "...\0";
	char ipBlock[strlen(buffer) + 1];
	int i = 0, j = 0;

	while (buffer[i] != '\0') {
		if (buffer[i] == *delimChars) {
			ipBlock[j++] = '\0';
			if (!_isIPBlockValid(ipBlock)) return FALSE;
			delimChars++;
			j = 0;
			i++;
			continue;
		}            
		else
			ipBlock[j++] = buffer[i++];
	}

	ipBlock[j++] = '\0';
	return *delimChars == '\0' && _isIPBlockValid(ipBlock);
}


// checks if the port number is valid
bool_t isPortValid(const char *buffer) {
	if (isStringValid(buffer, STR_DIGIT, STR_ALLLEN)) {
		int val = atoi(buffer);
		return val >= PORT_MIN && val <= PORT_MAX;
	}

	return FALSE;
}


// checks if the UID is valid
bool_t isValidUID(const char *buffer) {
	    return isStringValid(buffer, STR_DIGIT, UID_SIZE);
}


// checks if the password is valid
bool_t isValidPassword(const char *buffer) {
    return isStringValid(buffer, STR_ALPHANUM, PASS_SIZE);
}



// reads the user input
char* getUserInput(char *buffer, size_t size) {
	buffer[size - 2] = '\n';
	if (fgets(buffer, size, stdin) == NULL)
		FATAL("Unable to read the user input!");

	if (buffer[size - 2] != '\n' && buffer[size - 2] != '\0') {
		_WARN("The input line is too large! Type again\n\t - Maximum size: %lu characters", size - 2);
		return NULL;
	}

	return buffer;
}



// outputs a string character by character.
void putStr(const char *buffer, bool_t flush) {
	int i = 0;
	while (buffer[i] != '\0')
		putchar(buffer[i++]);
		
	if (flush)
		fflush(stdout);
}



// return te operation string correspondent to the file op
const char* getFileOp(const char op) {
    switch (op) {
	case FOP_L: return "list";
	case FOP_U: return "upload";
	case FOP_R: return "retrieve";
	case FOP_D: return "delete";
	case FOP_X: return "remove";
	default: return "\0";
    }
}


// generates a random number
int randomNumber(int min, int max) {
	return rand() % (max - min) + min;
}