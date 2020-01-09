// Lines of text. Free and open source. See LICENSE.

// Store information about the lines in the text. These are tracked purely from
// reported text insertions and deletions. There is one line for each newline. A
// "row" means a "line number counting from zero".
struct lines;
typedef struct lines lines;

// Create or free a lines object.
lines *newLines();
void freeLines(lines *ls);

// The number of lines in the text, equal to the number of newlines.
int countLines(lines *ls);

// Insert extra lines when string s is inserted in the text at a given position.
void insertLines(lines *ls, int at, int n, const char s[n]);

// Delete lines when n bytes are deleted from the text before a given position.
void deleteLines(lines *ls, int at, int n, const char s[n]);

// Find the start position of a line.
int startLine(lines *ls, int row);

// Find the end position of a line, after the newline.
int endLine(lines *ls, int row);

// Find the length of a given line, including the newline.
int lengthLine(lines *ls, int row);

// Find the row number of the line containing a position.
int findRow(lines *ls, int at);
