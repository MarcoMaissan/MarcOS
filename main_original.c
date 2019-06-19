#include <alloc.h>
#include <sys/print.h>
#include <sys/vga.h>
#include <sys/ps2.h>
/*
 *
 * MarcoOS
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
int lines = 0;

//Writable memory space start and end, NOT THE BLOCKS
int* stringstart = NULL;
int* stringend = NULL;
bool halted = false;
bool debugkernel =false;

void main(void)
{
    vga_clear();
    prompt();
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
    struct vga_cursor cursor = {};
    if(stringlength == cursorposition){
        //if the cursor is at the end of the string,
        //allocate new memory

        stringend = malloc(sizeof(char));
        if(stringstart == NULL) stringstart = stringend;
        //set value of stringend to char
        *stringend = c;
        //alloc new size and set stringend to writable memory space
        cursor = (struct vga_cursor){-1*(stringlength), 0};
        vga_curset(cursor, true);
        stringlength+=1; 
        printbuffer();
        cursorposition+=1;
    }
    else{
        //if stringend does not match cursor,
        //this means we have to insert a char somewhere inbetween
        stringend = malloc(sizeof(char));
        struct block *current = (void*)stringend - sizeof(struct block);
        struct block *previous = (void*)current - sizeof(struct block) - sizeof(char);
        char *previouschar = NULL;
        char *currentchar = NULL;     
        //shift all characters and insert new one   
        for(int i = stringlength; i > cursorposition; i--){
            previouschar = previous->addr;
            currentchar = current->addr;
            *currentchar = *previouschar;
            current = previous;
            previous = (void*)current - sizeof(struct block) - sizeof(char);
        }
        *previouschar = c;
        //In order to print the new buffer, we have to calculate how to get back
        //to the beginning and print the buffer again
        int difference = stringlength-cursorposition;
        cursor = (struct vga_cursor){(-1*(cursorposition)), 0};
        vga_curset(cursor, true);
        stringlength+=1;
        printbuffer();
        cursor = (struct vga_cursor){-1*difference, 0};
        vga_curset(cursor, true);
        cursorposition+=1;
    }
}

void printbuffer(){
    //Loop through memory and print characters from blocks.
    struct block *temp = (void*)stringstart - sizeof(struct block);
    for(int i = 0; i < stringlength; i++){
        int *c = temp->addr;
        char d = *c;
        printf("%c", d);
        temp = temp->next;
    }
}

void prompt(){printf("M> ");}

void enter(){
    //Check if a certain command is written.
    //If command is written, execute its function
    //and print another prompt.
    if(stringlength > 0){
        if(checkcommand("halt", 4) == true){
            halted = true;
        }else if(checkcommand("clear", 5) == true){
            vga_clear();
            clear();
        }
        else if(checkcommand("echo ",5) == true){
            echo();
            printf("\n");
            clear();
        } 
        else{
            printf("\nCommand ");
            printbuffer();
            printf(" not found!\n");
            clear();
        }
    }
    stringlength = 0;
    cursorposition = 0;
}

void echo(){
    //Print buffer except for the word "echo" on a new line
    struct block *temp = (void*)stringstart - sizeof(struct block);
    for(int i = 0; i <= stringlength; i++){
        int *c = temp->addr;
        char d = *c;
        if(i >= 5) printf("%c", d);
        temp = temp->next;
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

void checkcommand(char* command, int size){
    //Read the buffer in a for loop and see if it matches the char* command array
    struct block *temp = (void*)stringstart - sizeof(struct block);
    for(int i = 0; i < size; i++){
        int *c = temp->addr;
        char d = *c;
        if(d != command[i]) 
        {   
            return false;
        }
        temp = temp->next; 
    }
    printf("\n");
    return true;
}

void clear(){
    //clear the entire screen, generate new prompt, free buffer and reset variables.
    prompt();
    struct block *temp = (void*)stringstart - sizeof(struct block);
    freeBuffer(temp);
    cursorposition=0;
    stringlength=0;
}

void freeBuffer(struct block *head){
    //Skip to last element recursively
    if(head->next != NULL){
        freeBuffer(head->next);
    }
    int *addr = (void*) head + sizeof(struct block);
    free(addr);
} 

void backspace(){
    //REDO
    //Calculate how far off the cursor is from the left position
    int difference = stringlength - (stringlength-cursorposition);
    if(difference > 0){
        struct block *temp = (void*)stringstart - sizeof(struct block);
        struct block *previous = NULL;
        //Loop all the way to cursor position, but from there,
        //shift all to the left. This will thus remove a value.
        for(int i = 0; i < stringlength; i++){
            if(i >= difference){
                char *val1 = previous->addr;
                char *val2 = temp->addr;
                *val1 = *val2;
            }
            previous = temp;
            temp = temp->next;
        }
        //Free the memory to be reused
        free(previous->addr);

        //Go back to the beginning of the line and print it
        struct vga_cursor cursor = {-1*difference, 0};
        vga_curset(cursor, true);
        stringlength--;
        cursorposition--; 
        printbuffer();

        //Remove the last character from the line since it will be a duplicate otherwise
        cursor = (struct vga_cursor){1, 0};
        vga_curset(cursor, true);
        printf("%c", '\b');

        //Go back to where the user was.
        cursor = (struct vga_cursor){-1*(stringlength-cursorposition), 0};
        vga_curset(cursor, true);
    }

    ///OLD CODE
    // //first remove letters after cursor
    // //I didnt get it to work to delete a letter somewhere in the middle :(
    // int difference = stringlength-cursorposition;
    // struct vga_cursor cursor = {difference, 0};
    // vga_curset(cursor, true);

    // for(int i = 0; i < difference; i++){
    //     printf("%c", '\b');
    // }
    // 
    // cursorposition-=1;

    // if(stringlength > 0){
    //     printf("%c", '\b');
    //     struct block *temp = (void*)stringstart - sizeof(struct block);

    //     for(int i = 0; i < stringlength-1; i++){
    //         temp = temp->next;
    //     }
    //     char *letter = temp->addr;
    //     //printf("It is going to free the block with letter %c\n", *letter);

    //     free(temp->addr);
    //     if(stringlength > 0){
    //         stringend = (void*)temp - sizeof(char);
    //     }
    //     stringlength -= 1;
    // }else{
    //     struct block *temp = (void*)stringstart - sizeof(struct block);
    //     free(temp->addr);
    //     stringlength = 0;
    // }
}

void debug_kernel(){
    //To debug kernel, show all allocated memory addresses and values
    printf("\n");
    struct block *temp = (void*)stringstart - sizeof(struct block);
    int *c = temp->addr;
    char d = *c;
    printf("Address: %p, value: %c, used: %d\n", temp, d, temp->used);
    while(temp->next != NULL){
        temp = temp->next;
        int *c = temp->addr;
        char d = *c;
        printf("Address: %p, value: %c, used: %d\n", temp, d, temp->used);

    }
}
