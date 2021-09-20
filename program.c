#include "program.h"

static int exit_editor = 0;

int toExitEditor()
{
    return exit_editor;
}

int exitEditor(Context *context)
{
    exit_editor = 1;
    return SUCCESS;
}

char *error_messages[] = {
        "",
        "No command. Print \" h \" to see the list of commands.",
        "The number of arguments is wrong.",
        "Wrong arguments.",
        "Overflow. \a ",
        "Out of the range. \a ",
        "No file.",
        "The size of the file exceeds buffer volume. Try printing\
        p -v [volume] to enlarge the volume of buffer and loading a file again.",
        "No pattern.\a ",

        "\ta [text] add\n\
        i [position] [text] insert text to the position\n\
        put [text] put text under cursor\n\
        d [number of symbols] delete number of symbols before cursor\n\
        put [text] put text under cursor\n\
        c [centre | reach | cross] switch cursor strategy\n\
        f [text] find \n\
        rm [start] [end] remove symbols from start to end \n\
        rp [text1] [text2] replace each occurence of the text1 by text2 \n\
        m [position] move cursor to the position\n\
        lw [number of words] move cursor left to the number of words\n\
        rw [number of words] move cursor right to the number of words\n\
        p [-h [height]] [-w [width]] [-v [volume]] change parametres\n\
        up transform the text under cursor to an upper case\n\
        low transform the text under cursor to a lower case\n\
        s [filename] save text to a file\n\
        l [filename] load a file\n\
        h help\n\
        q quit"
};

void operError(unsigned int error_code)
{
    printf("%s\n", error_messages[error_code % 20]);
}

void setDefParams()
{
    if (!getVolume())
        initBuffer(DEF_VOLUME);
    if (!getWidth())
        setWidth(DEF_WIDTH);
    if (!getHeight())
        setHeight(DEF_HEIGHT);
}

int setParams(int arg_num, char *args[]){
    int exit_code = 0;
    size_t param = 0;

    while (--arg_num > 0){
        if ((*args)[0] == '-'){
            switch((*args)[1]){
                case('h'):
                    param = atoi(*++args);
                    if (param){
                        setHeight(param);
                    }
                    --arg_num;
                    break;
                case('w'):
                    param = atoi(*++args);
                    if (param){
                        setWidth(param);
                    }
                    --arg_num;
                    break;
                case('v'):
                    param = atol(*++args);
                    if (param){
                        initBuffer(param);
                    }
                    --arg_num;
                    break;
                default:
                    exit_code = WRONG_ARGS;
            }
        }
        else
            exit_code = WRONG_ARGS;
        ++args;
    }

    setDefParams();
    return exit_code;
}

int changeParams(Context *context)
{
    return setParams(context->arg_num, context->args);
}

void initEditor(int arg_num, char *args[])
{
    unsigned int exit_code = 0;
    if (arg_num > 0)
        exit_code = setParams(arg_num - 1, &args[1]);

    operError(exit_code);
}

void deinitEditor()
{
    freeBuffer();
}

typedef int (*Command)(Context *context);

int execute(Command command, Context *context)
{
    int exit_code = command(context);
    if (exit_code == SUCCESS && command != exitEditor){
        updateCursor();
        typeText();
    }
    return exit_code;
}

int findCommand (const char *com_name, Command *cur_command)
{
    if (!com_name)
        return -1;

    char f_char = com_name[0];
    switch (f_char) {
        case 'a':
            if (!strcmp(com_name, "a"))
                *cur_command = addText;
            break;
        case 'i':
            if (!strcmp(com_name, "i"))
                *cur_command = insertText;
            break;
        case 'd':
            if (!strcmp(com_name, "d"))
                *cur_command = deleteText;
            break;
        case 'r':
            if (!strcmp(com_name, "rm"))
                *cur_command = removeText;
            else if (!strcmp(com_name, "rp"))
                *cur_command = replaceText;
            else if (!strcmp(com_name, "rw"))
                *cur_command = moverw;
            break;
        case 'm':
            if (!strcmp(com_name, "m"))
                *cur_command = moveCurPos;
            break;
        case 'l':
            if (!strcmp(com_name, "l"))
                *cur_command = loadFile;
            if (!strcmp(com_name, "lw"))
                *cur_command = movelw;
            if (!strcmp(com_name, "low"))
                *cur_command = lowcase;
            break;
        case 'u':
            if (!strcmp(com_name, "up"))
                *cur_command = upcase;
            break;
        case 'c':
            if (!strcmp(com_name, "c"))
                *cur_command = switchStrategy;
            break;
        case 'f':
            if (!strcmp(com_name, "f"))
                *cur_command = findText;
            break;
        case 's':
            if (!strcmp(com_name, "s"))
                *cur_command = saveFile;
            break;
        case 'p':
            if (!strcmp(com_name, "p"))
                *cur_command = changeParams;
            else if (!strcmp(com_name, "put"))
                *cur_command = putText;
            break;
        case 'q':
            if (!strcmp(com_name, "q"))
                *cur_command = exitEditor;
            break;
        case 'h':
            if (!strcmp(com_name, "h"))
                *cur_command = help;
            break;
    }
    if (!(*cur_command))
        return -1;

    return 0;
}

int buildContext(char *input, char **cur_com_name, Context *cur_context)
{
    int def_arg_num = 5;
    char sep[] = " \n";
    *cur_com_name = strtok(input, sep);
    unsigned int size = def_arg_num;
    cur_context->args = (char **)calloc(size, sizeof (char *));
    unsigned int arg_num = 0;

    int realloc_success = 1;
    char **realloc_buffer = NULL;

    while((cur_context->args[arg_num] = strtok(NULL, sep)) != NULL){
        if (arg_num++ == size){
            size *= 2;
            realloc_buffer = (char **)realloc(cur_context->args, size * sizeof(char *));
            if (!realloc_buffer){
                realloc_success = 0;
                break;
            }
            cur_context->args = realloc_buffer;
        }
    }

    if (realloc_success && arg_num && ((cur_context->arg_num = arg_num) != size)){
        realloc_buffer = (char **)realloc(cur_context->args, arg_num * sizeof(char *));
        if (!realloc_buffer){
            realloc_success = 0;
        }
        cur_context->args = realloc_buffer;
    }

    if (!realloc_success)
        return -1;

    return 0;
}

void controlInput(char *input)
{
    Context *context = (Context *)calloc(1, sizeof (Context));
    char **cur_comm_name = (char **)calloc(1, sizeof(char *));
    Command *cur_command = (Command *)calloc(1, sizeof (Command));

    int cur_error_code = 0;
    if (buildContext(input, cur_comm_name, context) == 0){
        int findCom = findCommand(*cur_comm_name, cur_command);
        if (findCom == -1)
            cur_error_code = NO_COMMAND;
        else
            cur_error_code = execute(*cur_command, context);
    }
    else
        cur_error_code = WRONG_ARGC;

    operError(cur_error_code);

    free(cur_comm_name);
    free(cur_command);
    free(context->args);
    free(context);
}

static size_t def_input_size = 25;

void runEditor()
{
    int input_code = 0;
    char *input = (char *)calloc(def_input_size, sizeof(char));
    while(!toExitEditor()){
        printf("\nCommand: ");
        if ((input_code = getline(&input, &def_input_size, stdin)) != -1)
            controlInput(input);
    }
    free(input);
}
