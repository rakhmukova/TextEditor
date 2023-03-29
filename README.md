# TextEditor

This is a simple command-line text editor.

## Usage

In order to use the text editor complete the following steps. 
1. Clone the repository and change to the project directory:
``` 
git clone https://github.com/rakhmukova/TextEditor/
cd TextEditor
```
2. Compile the program using 'make' and run it with './editor':
```
make
./editor
```
Use the commands listed below to modify text. Type 'h' to see the list of commands and 'q' to exit.

## Commands

Here is the list of available commands:

- a [text]: Add text to the end.
- i [position] [text]: Insert text at a specific position.
- put [text]: Put text under the cursor.
- d [number of symbols]: Delete a specified number of symbols before the cursor.
- c [centre | reach | cross]: Switch the cursor strategy.
- f [text]: Find text.
- rm [start] [end]: Remove symbols from a specified starting position to an ending position.
- rp [text1] [text2]: Replace each occurrence of text1 with text2.
- m [position]: Move the cursor to a specific position.
- lw [number of words]: Move the cursor left by a specified number of words.
- rw [number of words]: Move the cursor right by a specified number of words.
- p [-h [height]] [-w [width]] [-v [volume]]: Change editor parameters, such as height, width, or volume.
- up: Transform the text under the cursor to uppercase.
- low: Transform the text under the cursor to lowercase.
- s [filename]: Save the text to a file.
- l [filename]: Load a text from a file.
- h: Display a help message.
- q: Quit the text editor.
