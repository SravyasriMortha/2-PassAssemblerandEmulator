/*****************************************************************************
TITLE: Assembler C Program																																
AUTHOR:   		SRAVYASRI MORTHA
ROLL NUMBER:	2101CS74
Declaration of AuthorshipCS
This .c file, asm.c, is part of the assignment of COMPUTER ARCHITECTURE at the 
department of Computer Science and Engineering, IIT Patna . 
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

/* --------------- MNEMONIC DEFINITIONS --------------- */

// Mnemonic Struct
// operand_req = 0 : No Operand
// operand_req = 1 : Value Operand
// operand_req = 2 : 'data'
// operand_req = 3 : 'set'
// operand_req = 4 : Offset Operand
typedef struct Mnemonic {
	char* mnemonic;
	char* opcode;
	int operand_req;
} Mnemonic;

Mnemonic mnemonics[] = {
	{"ldc", "00000000", 1},
	{"adc", "00000001", 1},
	{"ldl", "00000010", 4},
	{"stl", "00000011", 4},
	{"ldnl", "00000100", 4},
	{"stnl", "00000101", 4},
	{"add", "00000110", 0},
	{"sub", "00000111", 0},
	{"shl", "00001000", 0},
	{"shr", "00001001", 0},
	{"adj", "00001010", 1},
	{"a2sp", "00001011", 0},
	{"sp2a", "00001100", 0},
	{"call", "00001101", 4},
	{"return", "00001110", 0},
	{"brz", "00001111", 4},
	{"brlz", "00010000", 4},
	{"br", "00010001", 4},
	{"HALT", "00010010", 0},
	{"data", NULL, 2},
	{"SET", NULL, 3}
};

/* --------------- END OF MNEMONIC DEFINITIONS --------------- */

/* --------------- ERROR DEFINITIONS --------------- */

char* errors[] = {
	"No Error in Pass One",
	"Duplicate Label Defined",
	"Invalid Label",
	"Invalid Mnemonic",
	"Missing Operand",
	"Not A Number",
	"Unexpected Operand",
	"Extra on End of Line",
	"No Such Label",
	"Undefined Error"
};

/* --------------- END OF ERROR DEFINITIONS --------------- */

/* --------------- SYMBOL TABLE DEFINITIONS --------------- */

// Symbol Table Node
typedef struct SymbolTableNode {
	char* symbol;
	int address;
	struct SymbolTableNode* next;
} SymbolTableNode;

// The Symbol Table
typedef struct SymbolTable {
	SymbolTableNode* head;
	int (*insert)(struct SymbolTable*, char*);
	int (*assign)(struct SymbolTable*, char*, int);
	int (*getAddress)(const struct SymbolTable*, char*);
	bool (*hasUnassigned)(const struct SymbolTable*);
	bool (*update)(struct SymbolTable*, char*, int);
} SymbolTable;

// Function to create and return a new Symbol Table Node
SymbolTableNode* createSymbolTableNode(char* label, int address) {
	
	SymbolTableNode* newNode = (SymbolTableNode*) malloc(sizeof(SymbolTableNode));
	
	newNode -> symbol = (char*) malloc(1024);
	newNode -> address = address;
	newNode -> next = NULL;
	strcpy(newNode -> symbol, label);
	
	return newNode;
}

// Will insert a label occuring as an operand in case the label does not
// already exist in the table. 
int insertSymbolTable(SymbolTable* symbol_table, 
	char* label) {

	if (symbol_table -> head == NULL) {
		symbol_table -> head = createSymbolTableNode(label, -1);;
		return 1;
	}

	SymbolTableNode* head = symbol_table -> head;

	if (strcmp(label, head -> symbol) == 0) return 0;
	
	while (head -> next != NULL) {
		if (strcmp(label, (head -> next) -> symbol) == 0) return 0;
		head = head -> next;
	}
	
	head -> next = createSymbolTableNode(label, -1);
	return 1;
}

// Will assign the PC to a label in case it exists in the Symbol Table
// otherwise insert it and assign it the value. (RETURN 0)
// Will give error if address is already assigned (RETURN 1).
int assignSymbolTable(SymbolTable* symbol_table, 
	char* label, int PC) {

	(*symbol_table).insert(symbol_table, label);
	SymbolTableNode* head = symbol_table -> head;

	while (strcmp(head -> symbol, label) != 0) {
		head = head -> next;
	}
	
	if (head -> address != -1) return 1;

	head -> address = PC;
	return 0;
}

// Return the address of the given label.
// Return -1 if the address does not exist in the table.
int getAddress(const SymbolTable* symbol_table, char* label) {
	
	SymbolTableNode* head = symbol_table -> head;
	
	while (head != NULL && strcmp(head -> symbol, label) != 0) {
		head = head -> next;
	}

	if (head == NULL) return -1;
	return head -> address;
}

bool hasUnassigned(const SymbolTable* symbol_table) {

	SymbolTableNode* head = symbol_table -> head;
	while (head != NULL) {
		if (head -> address == -1) return 1;
		head = head -> next;
	}
	return 0;
}

bool update(SymbolTable* symbol_table, char* label, int address) {

	SymbolTableNode* head = symbol_table -> head;
	
	while (head != NULL && strcmp(head -> symbol, label) != 0) {
		head = head -> next;
	}

	if (head == NULL) return 0;
	head -> address = address;
	return 1;
}

/* --------------- END OF SYMBOL TABLE DEFINITIONS --------------- */

/* --------------- LITERAL TABLE DEFINITIONS --------------- */

// Literal Table Node
typedef struct LiteralTableNode {
	int literal;
	int address;
	struct LiteralTableNode* next;
} LiteralTableNode;

// The Literal Table
typedef struct LiteralTable {
	LiteralTableNode* head;
	int (*insert)(struct LiteralTable*, int);
	int (*assign)(struct LiteralTable*, int);
} LiteralTable;

int addressCtr;

// Function to create and return a new Literal Table Node
LiteralTableNode* createLiteralTableNode(int literal, int address) {
	
	LiteralTableNode* newNode = (LiteralTableNode*) malloc(sizeof(LiteralTableNode));
	
	newNode -> literal = literal;
	newNode -> address = address;
	newNode -> next = NULL;
	
	return newNode;
}

// Inserts a literal in the Literal Table at the end.
int insertLiteralTable(LiteralTable* literal_table, 
	int literal) {

	if (literal_table -> head == NULL) {
		literal_table -> head = createLiteralTableNode(literal, -1);
		return 1;
	}

	LiteralTableNode* head = literal_table -> head;
	while (head -> next != NULL) {
		head = head -> next;
	}

	head -> next = createLiteralTableNode(literal, -1);
	return 1;
}

// Assigns an address to all the literals in the literal table
// starting from the given address. (RETURN 1 on success)
int assignLiteralTable(LiteralTable* literal_table, int address) {

	addressCtr = address - 1;
	LiteralTableNode* head = literal_table -> head;
	while (head != NULL) {
		head -> address = address++;
		head = head -> next;
	}
	return 1;
}

int getNextLiteralAddress() {
	addressCtr++;
	return addressCtr;
}

/* --------------- END OF LITERAL TABLE DEFINITIONS --------------- */


// Make Symbol Table and Literal Table
SymbolTable symbol_table = {NULL, insertSymbolTable, assignSymbolTable, getAddress, hasUnassigned, update};
LiteralTable literal_table = {NULL, insertLiteralTable, assignLiteralTable};

/* --------------- UTILITY FUNCTIONS --------------- */

void printLiteralTable(LiteralTable* l_table, FILE* log) {
	fprintf(log, "LITERAL TABLE\n");
	LiteralTableNode* head = l_table -> head;
	while (head != NULL) {
		fprintf(log, "%d\t%d\n", head -> literal, head -> address);
		head = head -> next;
	}
	return;
}

void printSymbolTable(SymbolTable* s_table, FILE* log) {
	fprintf(log, "SYMBOL TABLE\n");
	SymbolTableNode* head = s_table -> head;
	while (head != NULL) {
		fprintf(log, "%s\t%d\n", head -> symbol, head -> address);
		head = head -> next;
	}
	return;
}

// Removes comment from a line
void removeComment(char* line) {
	char* comment = strchr(line, ';');
	if (comment != NULL) *comment = '\0';
	return;
}

// Removes ':'. RETURN 1 if ':' is present else 0
bool hasLabel(char* line) {
	char* colon = strchr(line, ':');
	if (colon != NULL) {
		*colon = ' ';
		return 1;
	}
	return 0;
}

// To check if label is valid
bool isValidLabel(char* label) {
	int len = strlen(label);
	if (len == 0 || (!isalpha(label[0]) && label[0] != '_')) return 0;
	for (int i = 0; i < len; i++) {
		if (!isalnum(label[i]) && label[i] != '_') return 0;
	}
	return 1;
}

// RETURN 1 if valid number else 0
bool isValidNumber(char* word) {
	char* endptr = NULL;
	errno = 0;
	int number = strtol(word, &endptr, 0);
	if (number == 0 && (word == endptr || errno != 0 
		|| (errno == 0 && word && *endptr != 0))) {
		return 0;
	}
	return 1;
}

// RETURN number conversion of given valid string
int toNumber(char* word) {
	char* endptr = NULL;
	return strtol(word, &endptr, 0);
}

// Check if mnemonic is valid
// RETURN 0: Invalid
// RETURN 1: Valid, does not require operand
// RETURN 2: Valid, requires operand
// RETURN 3: Valid, 'data'
// RETURN 4: Valid, 'set'
int isValidMnemonic(char* mnemonic) {
	for (int i = 0; i < 21; i++) {
		if (strcmp(mnemonics[i].mnemonic, mnemonic) == 0) 
			return mnemonics[i].operand_req + 1;
	}
	return 0;
}

// RETURN 1 if mnemonic type requires operand
bool requiresOperand(int type) {
	return type > 1;
}

// Scan a word from instruction string. RETURN sscanf error code.
int scanWord(char** line, char* word) {
	int lineptr;
	int scan_err = sscanf((*line), "%s%n", word, &lineptr);
	if (scan_err != -1) (*line) += lineptr;
	return scan_err;
}

int raiseError(int errcode, int line_num, FILE* log) {
	if (errcode && errcode < 8) fprintf(log, "ERROR: Line %d: %s\n", line_num, errors[errcode]);
	else if (errcode) fprintf(log, "ERROR: %s\n", errors[errcode]);
	else fprintf(log, "%s\n", errors[errcode]);
	return errcode;
}

/* --------------- END OF UTILITY FUNCTIONS --------------- */

/* --------------- FIRST PASS --------------- */

// Error Codes:
// RETURN 1: Duplicate Label
// RETURN 2: Invalid Label
// RETURN 3: Invalid Mneumonic
// RETURN 4: Missing Operand
// RETURN 5: Not a number
// RETURN 6: Unexpected Operand
// RETURN 7: Extra on end of line
// RETURN 8: No such label
int firstPass(FILE *assembly, FILE* log) {
	int PC = 0, err = 0, line_num = 0;
	char* line = (char*) malloc(1024);

	// While lines are available, take entire line as input
	// in 'line' variable.
	while (fgets(line, 1024, assembly)) {
		line_num += 1;
		// Remove comment if any
		removeComment(line);

		char* word = (char*) malloc(1024);
		char* label = (char*) malloc(1024);

		// Check if it is labelled. If label exists, 
		// scan it. If empty RETURN 2. If invalid RETURN 2.
		// If duplicate label detected, RETURN 1
		// Else insert it into the symbol table.
		bool labelled = hasLabel(line);
		if (labelled) {
			if (scanWord(&line, label) == -1) {
				err = raiseError(2, line_num, log);
				continue;
			}
			if (isValidLabel(label)) {
				if (symbol_table.assign(&symbol_table, label, PC)) {
					err = raiseError(1, line_num, log);
					continue;
				}
			}
			else {
				err = raiseError(2, line_num, log);
				continue;
			}
		}

		// Scan for next word. If nothing after label, continue loop.
		// Else, the next word should be mnemonic/'data'/'set'.
		// If not, RETURN 3.
		if (scanWord(&line, word) == -1) continue;
		int mnemonic_type = isValidMnemonic(word);
		if (!mnemonic_type) {
			err = raiseError(3, line_num, log);
			continue;
		}

		// Scan for the next word. 
		// If nothing after mnemonic and operand required, throw error
		// If something after mnemonic and operand not required, throw error
		if (requiresOperand(mnemonic_type)) {

			if (scanWord(&line, word) == -1) {
				err = raiseError(4, line_num, log);
				continue;
			}
			if (!isValidLabel(word) && !isValidNumber(word)) {
				err = raiseError(5, line_num, log);
				continue;
			}

			if (isValidLabel(word)) 
				symbol_table.insert(&symbol_table, word);

			else if (isValidNumber(word)) {
				// If 'set' update value for the given label
				if (mnemonic_type == 4) {
					symbol_table.update(&symbol_table, label, toNumber(word));
					continue;
				}

				// If not 'data' or 'set', then insert number in literal table
				if (mnemonic_type < 3) 
					literal_table.insert(&literal_table, toNumber(word));
			}
		} 

		else if (scanWord(&line, word) != -1) {
			err = raiseError(6, line_num, log);
			continue;
		}
		
		// Find if extra words are there
		if (scanWord(&line, word) != -1) {
			err = raiseError(7, line_num, log);
			continue;
		}
		PC += 1;
	}
	// Scan the Symbol Table for unassigned labels
	if(symbol_table.hasUnassigned(&symbol_table)) {
		err = raiseError(8, line_num, log);
	}

	// Fill literal table with address
	literal_table.assign(&literal_table, PC);

	if (err) return -1;
	err = raiseError(0, line_num, log);
	return 0;
}

/* --------------- END OF FIRST PASS --------------- */

/* --------------- UTILITY FUNCTIONS FOR SECOND PASS --------------- */

char* convert_24bit(int x) {
	char chr[] = "000000000000000000000000";
	for (int i = 23; i >= 0; i--) { 
		int k = x >> i; 
		if (k & 1) 
			chr[23 - i] = '1'; 
		else
			chr[23 - i] = '0';  
	} 
    char* ans = (char*) malloc(40);
    strcpy(ans, chr);
    return ans;
}

char* convert_32bit(int x) {
	char chr[] = "00000000000000000000000000000000";
	for (int i = 31; i >= 0; i--) { 
		int k = x >> i; 
		if (k & 1) 
			chr[31 - i] = '1'; 
		else
			chr[31 - i] = '0';  
	}
    char* ans = (char*) malloc(40);
    strcpy(ans, chr);
    return ans;
}

// RETURN 1 if Offset Required else 0
bool requiresOffset(char* mnemonic) {
	char* offsetMnemonics[] = {"ldl", "stl", "ldnl", "stnl", "call", "brz", "brlz", "br"};
	for (int i = 0; i < 8; i++) {
		if (strcmp(offsetMnemonics[i], mnemonic) == 0) return 1;
	}	
	return 0;
}

// Returns the 8-bit opcode of the given mnemonic
int getOpcode(char** opcode, char* mnemonic) {
	for (int i = 0; i < 20; i++) {
		if (strcmp(mnemonics[i].mnemonic, mnemonic) == 0) { 
			*opcode = mnemonics[i].opcode;
			return 1;
		}
	}
	return 0;
}

// Sets the 24-bit operand for the machine code. For data it sets 32-bits
int setOperandCode(char** operand, char* word, char* mnemonic, 
	int mnemonic_type, int PC) {
	int num = 0;
	// For 'data'
	if (mnemonic_type == 3) {
		*operand = convert_32bit(toNumber(word));
		return 1;
	}

	if (isValidNumber(word)) num = toNumber(word);
	else if (isValidLabel(word)) num = symbol_table.getAddress(&symbol_table, word);
	else return 0;

	if (requiresOffset(mnemonic) && isValidLabel(word)) num -= PC + 1;
	*operand = convert_24bit(num);

	return 1;
}

// Binary to Hexadecimal
char* binToHex(char* bin, int mode) {
	char* a = bin;
	int num = 0;
	do {
	    int b = *a == '1'? 1 : 0;
	    num = (num << 1) | b;
	    a++;
	} while (*a);
	char* hex = (char*) malloc(10);
	if (mode) sprintf(hex, "%08x", num);
	else sprintf(hex, "%x", num);
	return hex;
}

/* --------------- END OF UTILITY FUNCTIONS FOR SECOND PASS --------------- */

/* --------------- SECOND PASS --------------- */

int secondPass(FILE *assembly, FILE *object, FILE *listing, FILE* log) {
	int PC = 0, err = 0, line_num = 0;
	char* line = (char*) malloc(1024);
	fseek(assembly, 0, SEEK_SET);

	// While lines are available, take entire line as input
	// in 'line' variable.
	while (fgets(line, 1024, assembly)) {
		// Remove comment if any
		removeComment(line);
		//fprintf(log, "%s", line);

		char* label = (char*) malloc(1024);
		char* mnemonic = (char*) malloc(10);
		char* word = (char*) malloc(1024);
		char* opcode = (char*) malloc(10);
		char* operand = (char*) malloc(26);
		char* machine = (char*) malloc(36);
		machine[0] = '\0';

		bool labelled = hasLabel(line);
		if (labelled) {
			scanWord(&line, label);
			fprintf(listing, "%08x\t\t\t\t%s:\n", PC, label);
		}

		if (scanWord(&line, mnemonic) == -1) continue;
		int mnemonic_type = isValidMnemonic(mnemonic);
		getOpcode(&opcode, mnemonic);

		// For 'set'
		if (mnemonic_type == 4) continue;

		if (requiresOperand(mnemonic_type)) {
			scanWord(&line, word);			
			setOperandCode(&operand, word, mnemonic, mnemonic_type, PC);
		}
		else setOperandCode(&operand, "0", mnemonic, mnemonic_type, PC);

		strcat(machine, operand);
		if (mnemonic_type != 3) strcat(machine, opcode);

		fprintf(object, "%s\n", machine);

		fprintf(listing, "%08x\t%s\t%s\t0x%s\n", PC, binToHex(machine, 1), mnemonic, binToHex(operand, 0));
		
		PC += 1;
	}
	fprintf(log, "No Error in Pass Two\n");
	return 1;
}

/* --------------- END OF SECOND PASS --------------- */

// Driver Program
int main(int argc, char* argv[]) {
	// Open the Assembly File
	FILE *assembly, *object, *listing, *log;
	
	char* file_name = (char*) malloc(1024);
	strcpy(file_name, argv[1]);
	char* ptr = strchr(file_name, '.');
	if (ptr != NULL) *ptr = '\0';

	char* object_name = (char*) malloc(1024);
	strcat(object_name, file_name);
	strcat(object_name, ".o");
	
	char* listing_name = (char*) malloc(1024);
	strcat(listing_name, file_name);
	strcat(listing_name, ".lst");
	
	char* log_name = (char*) malloc(1024);
	strcat(log_name, file_name);
	strcat(log_name, ".log");
	
	assembly = fopen(argv[1], "r");
	object = fopen(object_name, "wb");
	listing = fopen(listing_name, "w");
	log = fopen(log_name, "w");

	// First Pass
	int fPassErr = firstPass(assembly, log);
	if (fPassErr) return 0;

	// Second Pass
	int sPassErr = secondPass(assembly, object, listing, log);

	// Close the Assembly File
	fclose(log);
	fclose(listing);
	fclose(object);
	fclose(assembly);
}