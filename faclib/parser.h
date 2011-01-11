#ifndef _PARSER_H_
#define _PARSER_H_ 1

char StrTrim(char *s, char c);
int QuotedStrSplit(char *s, char sep, char qb, char qe);
int StrSplit(char *s, char sep);
int SetParserQuote(char *qbegin, char *qend);
int SetParserBreak(char *brkch);
int SetParserEscape(char escape);
int SetParserWhite(char *white);
int Parse(char *token, int tokmax, char *line, 
	  int *brkpos, int *next, int *quoted);

#endif
