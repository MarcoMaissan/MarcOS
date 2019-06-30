#include <sys/vga.h>
#include <sys/ps2.h>
#include <ramfs.h>
#include <alloc.h>

bool exit = false;
struct file *f;
char *buffer;
int curx, cury;
int amountOfLines;
int currentline;
int strpos; //position in string

void editor(const char *name)
{
    currentline = strpos = amountOfLines = 0;
    vga_clear();
    loadfile(name);
    gotoline(0);

    while(exit == false){
        parsechar();
    }
    exit=false;
}

void loadfile(const char *name){
    move(0,0,false);
    f = ramfs_seek(name);
    if(f == NULL){
        touch(name);
        f = ramfs_seek(name);
        ramfs_write(f, "\0", 2);
    }
    buffer = malloc(strlen(f->data)+1);//----
    ramfs_read(f, &buffer);
    printf("%s",buffer);
}

void parsechar(){
    char c = ps2_getch();

    //Check for backspace, enter, arrow or char.
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
    insertcharinstring(c);
    gotostrpos();
}

void insertcharinstring(char c){
    //realloc data


    char* ptr = realloc(buffer, strlen(buffer)+2);
    buffer = ptr;
    f->size = strlen(buffer)+2;

    for(int i = f->size-1; i >= strpos; i--){
        buffer[i+1] = buffer[i];
    } 
    buffer[strpos] = c; 
    strpos++;

    redraw();
    gotostrpos();
}



void parsearrows(char x){
    //Check if we have to go left or right with the arrow key
    if(x == 'D' && curx > 0){
        //left
        move(-1, 0, true);
        strpos--;
    }else if(x == 'C' && buffer[strpos] != '\n' && buffer[strpos] != '\0'){
        //right
        move(1, 0, true);
        strpos++;
    }else if(x == 'A'){
        if(currentline > 0) currentline--;
        gotoline(currentline);
        //up
    }else if(x == 'B') {
        currentline++;
        gotoline(currentline);
    }else if(x == 'P'){
        del();
    }
}

void back(){
    if(strpos > 0){
        int i = strpos;
        char c = buffer[i];
        while(c != '\0'){
            buffer[i-1] = buffer[i];
            i++;
            c = buffer[i];
        }
        buffer[i-1] = '\0';
        f->size--;
        strpos--;
        vga_clear();
        redraw();
        gotostrpos();
    }
}

void del(){
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
    vga_clear();
    move(0,0,false);
    printf("%s", buffer);
}

void parsectrl(char c){
    if(c == 'S'){
        ramfs_write(f, buffer, (strlen(buffer)+1));
        move(0,24,false);
        printf("Saved! ");
    }else if(c == 'Q'){
        exit = true;
    }
}
