#include <sys/vga.h>
#include <sys/ps2.h>
#include <ramfs.h>
#include <alloc.h>

//
//Editor by Marco Maissan
//0949830
//TI2B
//

bool exit = false;
struct file *f;
char *buffer;
int curx, cury;
int amountOfLines;
int currentline;
int strpos; //position in string

void editor(const char *name)
{
    //Reset all variables for when editor is reloaded
    currentline = strpos = amountOfLines = 0;
    vga_clear();

    //loadfile and go to line 0
    loadfile(name);
    gotoline(0);

    while(exit == false){
        //keep arsing chars untill exit 
        parsechar();
    }
    //after exited, reset exit variable
    exit=false;
}

void loadfile(const char *name){
    //Move to beginning of screen
    //if file exists, load its buffer
    //if file doesn't exist yet, create a new one with given name
    move(0,0,false);
    f = ramfs_seek(name);
    if(f == NULL){
        touch(name);
        f = ramfs_seek(name);
        ramfs_write(f, "\0", 2);
    }
    buffer = malloc(strlen(f->data)+1);//----
    ramfs_read(f, &buffer);
    //Print loaded buffer
    printf("%s",buffer);
}

void parsechar(){
    char c = ps2_getch();
    

    //Check for backspace, enter, arrow, CTRL or char.
    if(c == '\b'){
        back(); 
    }else if(c == '\n'){
        parseenter(c);
    }
    else if(c == '\e'){
        ps2_getch();
        char c = ps2_getch();
        parsearrows(c);
    }else if(c == '^'){
        parsectrl(ps2_getch());
    }else{
        insertcharinstring(c);
    }
}

void parseenter(char c){
    //insert the enter and go to the next line by finding position
    insertcharinstring(c);
    gotostrpos();
}

void insertcharinstring(char c){
    //Before we insert a char, we resize the buffer
    //Strlen + 1 (\0) + 1 (new char) = strlen+2
    //

    char* ptr = realloc(buffer, strlen(buffer)+2);

    buffer = ptr;
    f->size = strlen(buffer)+2;


    //Switch from right side of array everything to the right 
    //one position and insert new char
    for(int i = f->size-1; i >= strpos; i--){
        buffer[i+1] = buffer[i];
    } 
    buffer[strpos] = c; 
    strpos++;


    redraw();
    gotostrpos(); //go to new string position
}

void parsearrows(char x){
    //Check if we have to go left or right with the arrow key
    if(x == 'D' && curx > 0){
        //move left and change stringposition -1
        move(-1, 0, true);
        strpos--;
    }else if(x == 'C' && buffer[strpos] != '\n' && buffer[strpos] != '\0'){
        //if there isnt a newline or end of string, move right
        move(1, 0, true);
        strpos++;
    }else if(x == 'A'){
        if(currentline > 0) currentline--;
        gotoline(currentline);
        //up
    }else if(x == 'B') {
        //move to currentline++
        currentline++;
        gotoline(currentline);
    }else if(x == 'P'){
        //Delete character
        del();
    }
}

void back(){
    if(strpos > 0){
        //Find the current string position
        //switch all characters one position to left of array
        //untill the NULL terminator is found
        int i = strpos;
        char c = buffer[i];
        while(c != '\0'){
            buffer[i-1] = buffer[i];
            i++;
            c = buffer[i];
        }
        //Add null character again at end of string
        buffer[i-1] = '\0';
        //change size :) 
        f->size--;
        strpos--;
        vga_clear();
        redraw();
        gotostrpos();
    }
}

void del(){
    //Do the same as backspace, but instead just switch all characters from cursor position to the left and append null terminator.
    if(strlen(buffer) > 0){
        int i = strpos;
        char c = buffer[i];
        while(c != '\0'){
            buffer[i] = buffer[i+1];
            i++;
            c = buffer[i];
        }
        buffer[i-1] = '\0';
        f->size--;
        vga_clear();
        redraw();
        gotostrpos();
    }
}

void gotostrpos(){
    //Go to the right cursor position. 
    //If newline char is found, go to newline. Else just keep moving right.
    move(0,0,false);
    for(int i = 0; i < strpos; i++){
        if(buffer[i] == '\n'){
            printf("%c", '\n');
        }else{
            move(1,0,true);
        }
    }
}

void gotoline(int n){
    //This function goes to line n.
    //It makes sure that lines > 1 row are parsed correctly.

    move(0,0,false); //go to beginning of screen
    int lineindex = 0;
    strpos = 0;
    for(int i = 0; i < f->size; i++){
        //if at the right line, break out of loop.
        if(lineindex == n){
            return;
        }
        else{
            move(1, 0, true); 
            if(buffer[i] == '\n'){
                //if newline, get current cursor, go back to start at currentline+1
                printf("\n");
                lineindex++;
            } if(buffer[i] == '\0'){
                //reset current line
                currentline--;
                move(-1, 0, true);
                return;
            }
        }
        strpos++;
    }
}

void move(int x, int y, bool r){
    //A simple method that makes vga_curset easier
    struct vga_cursor cursor = {x, y};
    vga_curset(cursor, r);
    if(r == true){
        curx += x;
        cury += y;}
    else{
        curx = x;
        cury = y;
    }
}

void redraw(){
    //Redraw entire buffer
    vga_clear();
    move(0,0,false);
    printf("%s", buffer);
}

void parsectrl(char c){
    //Swap mem location of buffer. This safes the file
    if(c == 'S'){
        ramfs_write(f, buffer, (strlen(buffer)+1));
        move(0,24,false);
        printf("Saved! ");
    }else if(c == 'Q'){
        exit = true;
    }
}
