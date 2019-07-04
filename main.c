#include <alloc.h>
#include <sys/print.h>
#include <sys/vga.h>
#include <sys/ps2.h>
#include <ramfs.h>
#include <editor.h>

//
//
//Marco Maissan
//0949830
//TI2B
//
//


int stringlength = 0;
int cursorposition = 0;

//Writable memory space start and end, NOT THE BLOCKS
char* string = NULL;
void* head = NULL;
int allocsize = 10;
int buffersize = 0;
struct vga_cursor cursor = {};

bool halted = false;
bool debugkernel =false;
    
void main(void)
{
    //Make example file
    touch("lorem");
    struct file *f = ramfs_seek("lorem");
    char *str = "Dit is een test met een hele erge onwijs lange string maar echt eentje die heeeel erg lang is.\nHallo\nDag";
    ramfs_write(f, str, strlen(str)+1); 


    vga_clear();
    prompt();
    //Allocate terminal string size
    string = head = malloc(sizeof(char) * allocsize);
    buffersize = allocsize;

    //Pointer to start of sentence.
    //Use variable memory allocation since sentence length can vary
    //eallocate memory after each input of character
    while(!halted){
        char c = ps2_getch();

        //Check for backspace, enter, arrow or char.
        if(c == '\b'){
            backspace();
        }
        else if(c == '\n'){
            enter();
        }else if(c == '\e'){
            ps2_getch();
            char x = ps2_getch();
            parseansi(x);
        }
        else{
            insertchar(c);
        }
        if(debugkernel == true) debug_kernel();
    }
    printf("\nHalted!");
}

void insertchar(char c){
    //Reallocate to fit string
    if(stringlength == buffersize-1){
        buffersize = buffersize+allocsize;
        string = (char*)realloc(string, buffersize);
    }

    int difference = stringlength-cursorposition; 
    
    //If at end of string
    if(difference == 0){
        string[stringlength] = c;
    }else{
        //Otherwise insert inbetween and switch to right till end of aray
        for(int i = stringlength; i >= cursorposition; i--){
            string[i+1] = string[i];
        }
        string[cursorposition] = c;
    }
    //Keep going till null character, and set cursor at right position...
    string[stringlength+1] = '\0';
    cursor = (struct vga_cursor){(-1*(cursorposition)), 0};
    vga_curset(cursor, true);
    stringlength++;
    printbuffer();
    cursor = (struct vga_cursor){-1*difference, 0};
    vga_curset(cursor, true);
    cursorposition++;
}

void backspace(){
    if(cursorposition > 0){
        //Only backspace if there are chars at all...
        //Move all characters from cursorposition to the left one place
        for(int i = cursorposition-1; i < stringlength; i++){
            string[i] = string[i+1]; 
        }    
        string[stringlength-1] = NULL;

        //Make sure to reposition the cursor correctly
        cursor = (struct vga_cursor){(-1*cursorposition) + stringlength,0};
        vga_curset(cursor, true);
        printf("\b");

        cursor = (struct vga_cursor){(-1*stringlength)+1, 0};
        vga_curset(cursor, true);

        //Redraw buffer
        printbuffer();

        //Change variables accordingly
        stringlength--;
        cursorposition--;

        cursor = (struct vga_cursor){(-1*(stringlength-cursorposition)), 0};
        vga_curset(cursor, true);
    }
}

void printbuffer(){
    printf("%s", string);
}

void prompt(){
    //Prompt is the line to write
    printf("\nM> ");
    string[0] = '\0';
    cursorposition = 0;
    stringlength = 0;
}

void clear(){
    //Clear screen and write prompt
    vga_clear();
    prompt();
}

bool checkcommand(char* command, int size){
    //Read the buffer in a for loop and see if it matches the char* command array
    for(int i = 0; i < size; i++){
        if(string[i] != command[i]) 
        {   
            return false;
        }
    }
    return true;
}

void enter(){
    //Check if a certain command is written.
    //If command is written, execute its function
    //and print another prompt.
    if(stringlength > 0){
        if(checkcommand("halt", 4) == true){
            halted = true;
        }else if(checkcommand("clear", 5) == true){
            clear();
        }
        else if(checkcommand("echo ",5) == true){
            echo();
            prompt(); 
        }else if(checkcommand("touch ",6) == true){
            touch(&string[6]);
            prompt();
        }else if(checkcommand("ls", 2) == true){
            ls();
            prompt();
        }else if(checkcommand("rm ", 3) == true){
            rm(&string[3]);
            prompt();
        }else if(checkcommand("cat ", 4) == true){
            cat(&string[4]);
            prompt();
        
        }else if(checkcommand("td", 2) == true){
            testdata();
            prompt();
        }
        else if(checkcommand("vim ",4) == true){
            editor(&string[4]);
            clear();
        }
        else{
            printf("\nCommand ");
            printbuffer();
            printf(" not found!");
            prompt();
        }
    }else{
        prompt();
    }
    stringlength = 0;
    cursorposition = 0;
}

void echo()
{
    //Print each character which comes after "echo " till end of string
    printf("\n");
    for(int i = 0; i <= stringlength; i++){
        if(i >= 5) printf("%c", string[i]);
    }

}

void parseansi(char x){
    //Check if we have to go left or right with the arrow key
    if(x == 'D' && cursorposition > 0){
        struct vga_cursor cursor = {-1, 0};
        vga_curset(cursor, true);
        cursorposition-=1;
    }else if(x == 'C' && cursorposition < stringlength){
        struct vga_cursor cursor = {1, 0};
        vga_curset(cursor, true);
        cursorposition+=1;
    }
}

void debug_kernel(){
    //DEBUG KERNEL WILL SHOW VARIABLES.
    //To debug kernel, show all allocated memory addresses and values
    struct block *temp = (void*)head - sizeof(struct block);
    printf("Address: %p, used: %d, size: %d\n", temp, temp->used, temp->size);
    while(temp->next != NULL){
        temp = temp->next;
        printf("Address: %p used: %d, size: %d\n", temp, temp->used, temp->size);

    }
}
