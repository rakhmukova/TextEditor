#ifndef TEXT_EDITOR_COMMANDS_H
#define TEXT_EDITOR_COMMANDS_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SUCCESS 0
#define NO_COMMAND 21
#define WRONG_ARGC 22
#define WRONG_ARGS 23
#define OVERFLOW 24
#define OUT_OF_THE_RANGE 25
#define NO_FILE 26
#define TOO_LARGE 27
#define NO_PATTERN 28
#define HELP_MESSAGE 29

#define MAX_WIDTH 100
#define DEF_WIDTH 80
#define MAX_HEIGHT 40
#define DEF_HEIGHT 25

#define MAX_VOLUME 1000000
#define DEF_VOLUME 10000

typedef struct {
    unsigned int arg_num;
    char **args;
} Context;

enum Strategies{
    CENTRE = 35,
    CROSS,
    REACH
};

int addText (Context *context);
int insertText (Context *context);
int putText (Context *context);
int removeText (Context *context);
int deleteText (Context *context);
int upcase(Context *context);
int lowcase(Context *context);
int findText(Context *context);
int replaceText(Context *context);
int saveFile(Context *context);
int loadFile(Context *context);
int help(Context *context);
int switchStrategy(Context *context);
int movelw(Context *context);
int moverw(Context *context);
int moveCurPos(Context *context);

void typeText();
void updateCursor();

void initBuffer(size_t pos_volume);
void setWidth(unsigned int pos_width);
void setHeight(unsigned int pos_height);

size_t getVolume();
unsigned int getWidth();
unsigned int getHeight();

void freeBuffer();

#endif
