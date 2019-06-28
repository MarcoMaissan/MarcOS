#include <alloc.h>
#include <sys/print.h>
#include <sys/vga.h>
#include <sys/ps2.h>
#include <ramfs.h>
/*
 *
 * MarcOS
 * src/main.c
 *
 * Copyright (C) 2019 Marco Maissan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 */
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
    vga_clear();
    prompt();
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
    if(stringlength == buffersize-1){
        buffersize = buffersize+allocsize;
        string = (char*)realloc(string, buffersize);
    }

    int difference = stringlength-cursorposition; 

    if(difference == 0){
        string[stringlength] = c;
    }else{
        for(int i = stringlength; i >= cursorposition; i--){
            string[i+1] = string[i];
        }
        string[cursorposition] = c;
    }
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
        for(int i = cursorposition-1; i < stringlength; i++){
            string[i] = string[i+1]; 
        }    
        string[stringlength-1] = NULL;

        cursor = (struct vga_cursor){(-1*cursorposition) + stringlength,0};
        vga_curset(cursor, true);
        printf("\b");

        cursor = (struct vga_cursor){(-1*stringlength)+1, 0};
        vga_curset(cursor, true);

        printbuffer();

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
    printf("\nM> ");
    string[0] = '\0';
    cursorposition = 0;
    stringlength = 0;
}

void clear(){
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
    //To debug kernel, show all allocated memory addresses and values
    struct block *temp = (void*)head - sizeof(struct block);
    printf("Address: %p, used: %d, size: %d\n", temp, temp->used, temp->size);
    while(temp->next != NULL){
        temp = temp->next;
        printf("Address: %p used: %d, size: %d\n", temp, temp->used, temp->size);

    }
}
