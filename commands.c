#include "commands.h"

static char *BUFFER;
static size_t buffer_volume;
static size_t total_volume;

extern long int cursor_pos;
extern long int start_pos;
extern int moveCursor(size_t pos);

void initBuffer(size_t pos_volume)
{
    buffer_volume = (pos_volume <= MAX_VOLUME - 2)? pos_volume : MAX_VOLUME;
    ++buffer_volume;
    free(BUFFER);
    BUFFER = (char *)calloc(buffer_volume + 1, sizeof (char));
    BUFFER[0] = ' ';
    total_volume = 1;
    start_pos = 1;
}

size_t getVolume()
{
    return buffer_volume;
}

void freeBuffer()
{
    buffer_volume = 0;
    total_volume = 0;
    free(BUFFER);
    BUFFER = NULL;
}

static int insert(size_t pos, const char *word)
{
    size_t length = strlen(word);
    if (buffer_volume - total_volume < length)
        return OVERFLOW;

    if (pos < 1 || pos > total_volume)
        return OUT_OF_THE_RANGE;

    int left = 0;
    int right = 0;

    if (BUFFER[pos - 1] != ' ' && length + total_volume <= buffer_volume)
        left = 1;

    if (BUFFER[pos]!= ' ' && BUFFER[pos]!= '\0' && length + left + total_volume <= buffer_volume)
        right = 1;

    long int cur_pos = 0;
    for (long int i = 0; i < total_volume - pos; ++i){
        cur_pos  = total_volume - 1 - i;
        BUFFER [cur_pos + length + left + right] = BUFFER[cur_pos];
    }

    for (size_t i = 0; i < length; ++i){
        BUFFER[pos + i + left] = word[i];
    }

    if (left)
        BUFFER[pos] = ' ';
    if (right)
        BUFFER[pos + length + left] =' ';

    total_volume += length + left + right;
    if (cursor_pos >= pos)
        moveCursor(cursor_pos + length + left + right);

    return SUCCESS;
}

int addText(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    return insert(total_volume, context->args[0]);
}

int insertText (Context *context)
{
    if (context->arg_num != 2)
        return WRONG_ARGC;

    size_t pos = atol(context->args[0]);
    if (!pos)
        return WRONG_ARGS;

    return insert(pos, context->args[1]);
}

int putText(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    return insert(cursor_pos, context->args[0]);
}

static int delete(size_t start, size_t end)
{
    if (start <= 0 || end >= total_volume || start >= end)
        return OUT_OF_THE_RANGE;

    size_t length = end - start;
    for (long int i = start; i < total_volume - length; i++) {
        BUFFER[i] = BUFFER[i + length];
    }
    for (long int i = total_volume - length; i < total_volume; i++) {
        BUFFER[i] = 0;
    }
    total_volume -= length;

    if (cursor_pos >= start)
        moveCursor(cursor_pos - length);

    return SUCCESS;
}

int removeText(Context *context) {
    if (context->arg_num != 2)
        return WRONG_ARGC;
    size_t start = atol(context->args[0]);
    size_t end = atol(context->args[1]);
    if (start == 0 || end == 0 || start >= end)
        return WRONG_ARGS;

    return delete(start, end);
}

int deleteText(Context *context)
{
    unsigned int arg_num = context->arg_num;
    if (arg_num > 1)
        return WRONG_ARGC;

    int num_of_sym = 1;
    if (arg_num > 0 && (num_of_sym = atoi(context->args[0])) <= 0){
        return WRONG_ARGS;
    }

    return delete(cursor_pos - num_of_sym, cursor_pos);
}

unsigned int num_of_letters = 26;
enum CASE{
    LOW,
    UP
};

int tryChange(size_t pos, enum CASE case_param)
{
    int diff = BUFFER[pos] -'a';
    if (diff >= 0 && diff <= num_of_letters){
        if (case_param == UP)
            BUFFER[pos] = 'A' + diff;
        return 1;
    }

    diff = BUFFER[pos] - 'A';
    if (diff >= 0 && diff <= num_of_letters){
        if (case_param == LOW)
            BUFFER[pos] = 'a' + diff;
        return 1;
    }

    return 0;
}

int changeCase(enum CASE case_param)
{
    size_t cur_pos = cursor_pos;
    if (BUFFER[cur_pos] == ' ')
        return SUCCESS;

    while(tryChange(cur_pos, case_param)){
        if (--cur_pos < 1)
            break;
    }

    cur_pos = cursor_pos;
    while(tryChange(cur_pos, case_param)){
        if (++cur_pos >= total_volume)
            break;
    }

    return SUCCESS;
}

int upcase(Context *context)
{
    if (context->arg_num > 0)
        return WRONG_ARGC;

    enum CASE case_param = UP;
    return changeCase(case_param);
}

int lowcase(Context *context)
{
    if (context->arg_num > 0)
        return WRONG_ARGC;

    enum CASE case_param = LOW;
    return changeCase(case_param);
}

int contains(const char *first_str, const char *second_str, size_t length)
{
    for (size_t i = 0; i < length; ++i){
        if (first_str[i] != second_str[i])
            return 0;
    }
    return 1;
}

long int findNextPos(const char *pattern, const char *text, size_t length)
{
    long int cur_pos = 0;
    while(text[cur_pos + length - 1] != '\0'){
        if (contains(pattern, &text[cur_pos], length))
            return cur_pos;
        ++cur_pos;
    }

    return -1;
}

int findPatternPositions(const char *pattern, const char *text, long int **positions)
{
    size_t length = strlen(pattern);
    if (length > total_volume)
        return 0;

    long int amount = 5; 
    *positions = (long int *)calloc(amount, sizeof (long int));

    long int cur_amount = 0;
    long int cur_pos = 0;
    long int cur_start = 1;
    long int *realloc_buffer =NULL;
    int realloc_success = 1;

    while(cur_start + length < total_volume){
        cur_pos = findNextPos(pattern, &text[cur_start], length);
        if (cur_pos == -1)
            break;

        if (++cur_amount == amount){
            amount *= 2;
            realloc_buffer = (long int *)realloc(*positions, amount * sizeof(long int));
            if (!realloc_buffer){
                realloc_success = 0;
                break;
            }
            *positions = realloc_buffer;
        }
        (*positions)[cur_amount] = cur_start + cur_pos;
        cur_start += cur_pos + length;
    }

    if (!cur_amount || !realloc_success){
        free(*positions);
        return 0;
    }
        
    (*positions)[0] = cur_amount;
    if (amount != cur_amount + 2){
        realloc_buffer = (long int *)realloc(*positions, (cur_amount + 2) * sizeof(long int));
        if (!realloc_buffer){
            free(*positions);
            return 0;
        }
        *positions = realloc_buffer;    
    }
    (*positions)[cur_amount + 1] = length;
    
    return 1;
}

extern void typeHighlightedText(const long int *highlighted);

int findText(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    long int **pattern_positions = (long int **)calloc(1, sizeof (long int *));
    if (!findPatternPositions(context->args[0], BUFFER, pattern_positions))
        return NO_PATTERN;

    typeHighlightedText(*pattern_positions);

    free(*pattern_positions);
    free(pattern_positions);
    return SUCCESS;
}

int replace(char *to_replace, char *replace_by, long int **pattern_positions)
{
    long int amount = (*pattern_positions)[0];
    size_t f_length = (*pattern_positions)[amount + 1];
    size_t s_length = strlen(replace_by);
    
    long int diff = s_length - f_length;
    long int overall_shift = amount * diff;
    if (total_volume + overall_shift >= buffer_volume)
        return OUT_OF_THE_RANGE;

    total_volume += overall_shift;
    long int new_cur_start = 0;
    long int new_cur_pos = 0;
    long int new_cur_end = 0;

    if (diff >= 0){
        new_cur_end = total_volume;
        for (long int i = 0; i < amount; ++i){
            new_cur_start = (*pattern_positions)[amount - i] + f_length + overall_shift;
            for (long int j = 0; j < new_cur_end - new_cur_start; ++j){
                new_cur_pos = new_cur_end - j - 1;
                BUFFER[new_cur_pos] = BUFFER[new_cur_pos - overall_shift];
            }
            overall_shift -= diff;
            (*pattern_positions)[amount - i] += overall_shift;
            new_cur_end = new_cur_start - s_length;
        }
    }
    else{
        overall_shift = 0;
        diff=-diff;
        for (long int i = 0; i < amount; ++i){
            new_cur_start = (*pattern_positions)[i + 1] - overall_shift + s_length;
            new_cur_end = i + 1 < amount ? (*pattern_positions)[i + 2] - overall_shift : total_volume;
            overall_shift += diff;
            for (long int j = new_cur_start; j < new_cur_end; ++j){
                BUFFER[j] = BUFFER[j + overall_shift];
            }
            (*pattern_positions)[i + 1] -= (overall_shift - diff);
        }

        for (long int i = 0; i < overall_shift; ++i){
            BUFFER[total_volume + i] = '\0';
        }
    }

    for (long int i = 0; i < amount; ++i){
        new_cur_start = (*pattern_positions)[i+1];
        for (size_t j = 0; j < s_length; ++j){
            BUFFER[new_cur_start + j] = replace_by[j];
        }
    }

    (*pattern_positions)[amount + 1] = s_length;
    return SUCCESS;
}

int replaceText(Context *context)
{
    if (context->arg_num != 2)
        return WRONG_ARGC;

    char *to_replace = context->args[0];
    char *replace_by = context->args[1];

    long int **pattern_positions = (long int **)calloc(1, sizeof (long int *));
    if (!findPatternPositions(to_replace, BUFFER, pattern_positions))
        return NO_PATTERN;

    int exit_code = 0;
    if ((exit_code = replace(to_replace, replace_by, pattern_positions)) == SUCCESS)
        typeHighlightedText(*pattern_positions);

    free(*pattern_positions);
    free(pattern_positions);
    return exit_code;
}

static FILE *cur_file = NULL;

int isSpecialChar(char c)
{
    return (c == '\n' || c == '\t');
}

int fillBuffer(char *file_name)
{
    cur_file = fopen(file_name, "r");
    int c = 0;
    long int pos = 1;
    for (; pos < buffer_volume - 1; ++pos){
        if ((c = fgetc(cur_file)) == EOF)
            break;
        else if (isSpecialChar(c))
            c =' ';
        BUFFER[pos] = c;
    }
    fclose(cur_file);

    total_volume = pos;
    moveCursor(1);
    start_pos = 1;
    if (c != EOF)
        return TOO_LARGE;

    return SUCCESS;
}

int fileExists(char *file_name)
{
    int exist = 1;
    cur_file = fopen(file_name, "r +");
    if (cur_file == NULL)
        exist = 0;
    else
        fclose(cur_file);
    return exist;
}

int loadFile(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    char *cur_file_name = context->args[0];
    if (!fileExists(cur_file_name))
        return NO_FILE;

    initBuffer(buffer_volume);
    if (fillBuffer(cur_file_name) == TOO_LARGE){
        return TOO_LARGE;
    }
    return SUCCESS;
}

int printToFile(char *file_name)
{
    cur_file = fopen(file_name, "w+");
    for (size_t i = 1; i < total_volume; ++i){
        fputc(BUFFER[i], cur_file);
    }
    fclose(cur_file);
    return SUCCESS;
}

int saveFile(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    return printToFile(context->args[0]);
}

int help(Context *context)
{
    return HELP_MESSAGE;
}

#define YELLOW "\033[1;43m"
#define RESET "\033[1;0m"
#define DEF_SHIFT 3
#define DEF_INDENT 2

static int def_shift;
static int enough_for_shift;
static unsigned int width;
static unsigned int height;

long int start_pos = 1;
long int cursor_pos = 1;

unsigned int getWidth()
{
    return width;
}

unsigned int getHeight()
{
    return height;
}

void setWidth(unsigned int pos_width)
{
    if (!pos_width)
        return;

    width = (pos_width > MAX_WIDTH)? MAX_WIDTH: pos_width;
    def_shift = DEF_SHIFT >= width ? width - 1: DEF_SHIFT;
    enough_for_shift = DEF_INDENT >= width ? width - 1: DEF_INDENT;
}

void setHeight(unsigned int pos_height)
{
    if (!pos_height)
        return;

    height = (pos_height > MAX_HEIGHT)? MAX_HEIGHT: pos_height;
}

void typeText()
{
    long int pos = start_pos;
    long int end_pos = start_pos + height * width;
    if (end_pos >= total_volume)
        end_pos = total_volume;
    int row_number = (end_pos - start_pos) / width + 1;
    for (int i = 0; i < row_number; ++i){
        printf("\n");
        for (int j = 0; j < width; ++j){
            if (pos == end_pos)
                break;
            if (pos == cursor_pos){
                printf("%s%c%s", YELLOW, BUFFER[pos], RESET);
            }
            else
                printf("%c", BUFFER[pos]);
            ++pos;
        }
    }
    printf("\n\nStart position: %li \n\n", start_pos);
}

enum COLOURS{
    Y,
    W
};

void switchColor(enum COLOURS *colour)
{
    if (*colour == Y){
        *colour = W;
        printf("%s", RESET);
    }
    else{
        *colour = Y;
        printf("%s", YELLOW);
    }
}

long int firstOccur(const long int *highlighted)
{
    size_t first_border_pos = 0;
    size_t amount = highlighted[0];
    size_t occur = 1;
    for (; occur < amount + 1; ++occur){
        first_border_pos = highlighted[occur];
        if (first_border_pos >= start_pos)
            break;
    }
    return occur;
}

void typeHighlightedText(const long int *highlighted)
{
    long int amount = highlighted[0];
    size_t length = highlighted[amount + 1];
    long int next_border_pos = 0;
    long int occur = firstOccur(highlighted);
    if (occur <= amount){
        next_border_pos = highlighted[occur];
        if (occur > 1 && highlighted[occur - 1] + length >= start_pos)
            next_border_pos = highlighted[--occur] + length;
    }

    enum COLOURS *cur_colour = (enum COLOURS *)calloc(1, sizeof(enum COLOURS));
    *cur_colour = W;

    long int end_pos = start_pos + height * width;
    if (end_pos >= total_volume)
        end_pos = total_volume;
    int row_number = (end_pos - start_pos) / width + 1;

    long int pos = start_pos;
    for (int i = 0; i < row_number; ++i) {
        for (int j = 0; j < width; ++j) {
            if (pos == end_pos && *cur_colour == Y){
                switchColor(cur_colour);
                break;
            }
            if (pos == next_border_pos) {
                switchColor(cur_colour);
                if (*cur_colour == Y)
                    next_border_pos += length;
                else if (++occur <= amount){
                    // подряд идут два образца
                    if (next_border_pos == highlighted[occur]){
                        switchColor(cur_colour);
                        next_border_pos += length;
                    }
                    else
                        next_border_pos = highlighted[occur];
                }
            }
            printf("%c", BUFFER[pos]);
            ++pos;
        }

        if (*cur_colour == Y)
            printf("%s\n%s", RESET, YELLOW);
        else
            printf("\n");
    }
    free(cur_colour);
}

static enum Strategies cursor_strategy = CROSS;

void setStrategy(unsigned int strategy)
{
    cursor_strategy = strategy;
}

void updateCentre()
{
    if (total_volume <= width * height){
        return;
    }

    long int centre_pos = height / 2 * width + width / 2;
    if (cursor_pos < 1 + centre_pos / 2 || cursor_pos > total_volume - centre_pos)
        moveCursor(start_pos + centre_pos);
    else{
        centre_pos += start_pos;
        start_pos += cursor_pos - centre_pos;
    }
    
    if (start_pos < 1)
        start_pos = 1;
}

void updateCrossEdge()
{
    long int end_pos = start_pos + width * height;
    if (cursor_pos >= start_pos && cursor_pos < end_pos)
        return;

    size_t diff = 0;
    if (cursor_pos >= end_pos){
        diff = cursor_pos - end_pos;
        if (end_pos + def_shift < total_volume)
            diff += def_shift;
    }
    else{
        diff = cursor_pos - start_pos;
        if (start_pos + diff >= 1 + def_shift)
            diff -= def_shift;
    }

    start_pos += diff;
}

void updateReachEdge()
{
    long int end_pos = start_pos + width * height;
    if (cursor_pos < end_pos - enough_for_shift &&
        cursor_pos >= start_pos + enough_for_shift)
        return;

    long int diff = 0;
    if (cursor_pos >= end_pos - enough_for_shift){
        diff = cursor_pos - end_pos;
        if (end_pos + def_shift < total_volume)
            diff += def_shift;
    }
    else{
        diff = cursor_pos - start_pos;
        if (start_pos + diff >= 1 + def_shift)
            diff -= def_shift;
    }

    start_pos += diff;
}

void updateCursor()
{
    switch(cursor_strategy){
        case(CENTRE):
            updateCentre();
            break;
        case(CROSS):
            updateCrossEdge();
            break;
        case(REACH):
            updateReachEdge();
            break;
        default:
            cursor_strategy = REACH;
            updateCursor();
    }
}

int switchStrategy(Context *context) {
    if (context->arg_num != 1)
        return WRONG_ARGC;

    enum Strategies cur_strategy = 0;
    char *strategy_name = context->args[0];
    if (strategy_name[0] == 'c') {
        if (!strcmp(strategy_name, "cross"))
            cur_strategy = CROSS;

        else if (!strcmp(strategy_name, "centre"))
            cur_strategy = CENTRE;
    }
	 else if (strategy_name[0] == 'r' && !strcmp(strategy_name, "reach"))
        cur_strategy = REACH;

    if (!cur_strategy)
        return WRONG_ARGS;

    setStrategy(cur_strategy);
    return SUCCESS;
}

int moveCursor(size_t pos)
{
    if (pos <= 0 || pos >= total_volume)
        return OUT_OF_THE_RANGE;

    cursor_pos = pos;
    return SUCCESS;
}

int moveCurPos(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    size_t pos = atol(context->args[0]);
    if (!pos)
        return WRONG_ARGS;

    return moveCursor(pos);
}

int movelw(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    unsigned int words_num = atoi(context->args[0]);
    if (!words_num)
        return WRONG_ARGS;

    long int pos = cursor_pos;
    for (int i = 0; i < words_num; ++i){
        while (pos >= 1 && !isalpha(BUFFER[pos])){
            --pos;
        }
        while(pos >= 1 && isalpha(BUFFER[pos])){
            --pos;
        }
    }
    return moveCursor(pos);
}

int moverw(Context *context)
{
    if (context->arg_num != 1)
        return WRONG_ARGC;

    unsigned int words_num = atoi(context->args[0]);
    if (!words_num)
        return WRONG_ARGS;

    long int pos = cursor_pos;
    for (int i = 0; i < words_num; ++i){
        while (pos < total_volume && !isalpha(BUFFER[pos])){
            ++pos;
        }
        while(pos < total_volume && isalpha(BUFFER[pos])){
            ++pos;
        }
    }
    return moveCursor(pos);
}
