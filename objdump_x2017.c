#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>


//Struct for instructions (ie: opcode, types, and values)
typedef struct Instruction {
    char opcode[6];
    u_int8_t value_1;
    char type_1[4];
    u_int8_t value_2;
    char type_2[4];
    int num_val;
} instruction;

//Struct for each function
typedef struct Function {
    int size;
    u_int8_t func_label;
    instruction* instruction_list;
    u_int8_t* stack;
    int num_stack;
} function;

//Own implementation of power
int power(int x, int y) {
    int original = x;
    if (y == 0) {
        return 1;
    }
    for (int i = y; i > 1; i --) {
        x = x * original;
    }
    return x;
}

//Own implementation
int contains(u_int8_t* array, char val, int size) {
    for (int i = 0; i < size; i ++) {
        if (array[i] == val) {
            return i;
        }
    }
    return -1;
}

//Own implementation to convert to binary
void convert_to_binary(int num, int* binary) {
    u_int8_t u_num = num;
    for (int i = 7; i >= 0; i --) {
        if (power(2, i) <= u_num) {
            u_num -= power(2, i);
            binary[7 - i] = 1;
        }
        else {
            binary[7 - i] = 0;
        }
    }
}

//Own implementation to convert binary to int
int convert_to_num(int* binary, int terms) {
    int num = 0;
    for (int i = 0; i < terms; i ++) {
        num += binary[terms - i - 1] * power(2, i);
    }
    return num;
}


//Main
int main(int argc, char** argv) {

  //Checking for not enough arguments, and printing an error and exiting if so.
  if (argc < 2) {
      perror("Please provide more command line arguments!");
      exit(1);
  }

  //Checking for too many arguments, and printing an error and exiting if so.
  if (argc > 2) {
      perror("You've provided too many command line arguments!");
      exit(1);
  }

  FILE* bin = fopen(argv[1], "rb");
  int* bin_array = NULL;
  int counter = 0;

  //Parsing file through to into bin_array.
  while (!feof(bin)) {
      char ch = fgetc(bin);
      if (ch != EOF) {
          counter ++;
          if (counter == 1) {
              bin_array = malloc(sizeof(int) * 8);
          }
          else {
              bin_array = realloc(bin_array, sizeof(int) * 8 * counter);
          }
          convert_to_binary(ch, &bin_array[(counter - 1) * 8]);
      }
  }

  //Checking if file exists, and printing error and exiting if so.
  if (bin == NULL) {
      perror("File doesn't exist!");
      exit(1);
  }

  int final_index = 8 * counter; //final index of bin_array
  int curr_index = 8 * counter - 1; //current index of bin_array (where we start reading from)
  int to_read[5] = {-1, 2, -1, 2, 3}; //array containing number of bits that we need to read next
  int read_counter = 4; //counter for to_read array
  int finished = 0; //bool for finished

  int size = 1; //size of instructions (will be at least 1)
  int num_functions = 1; //number of functions (will always be at least 1)
  function* function_list = malloc(sizeof(function) * num_functions * 10000); //malloc our function_list
  function_list[num_functions - 1].instruction_list = malloc(sizeof(instruction)); //malloc our instruction_list
  function_list[num_functions - 1].num_stack = 0; //setting counter for our stack to 0 (otherwise will be set to a random number)
  int num_instructions = convert_to_num(&bin_array[final_index - 5], 5); //converting the number of instructions given in the file, from binary to an int (number)
  function_list[num_functions - 1].stack = NULL;
  curr_index -= 5; //start reading from RET (num_instructions are always 5 bits long)

  // Need to free
  function_list[num_functions - 1].size = num_instructions; //size of the function is the number of instructions given
  int moving_to_new_function = 0; //bool for if we are moving to a new function or not

  while (!finished) {

      //if we are moving to a new function
      if (moving_to_new_function) {
          num_functions ++; //increase the number of functions by 1
          function_list = realloc(function_list, sizeof(function) * num_functions * 10000); //realloc memory for our function
          function_list[num_functions - 1].instruction_list = malloc(sizeof(instruction)); //malloc our instruction list
          function_list[num_functions - 1].size = convert_to_num(&bin_array[curr_index - 5 + 1], 5); //convert our function size from binary to an int
          num_instructions = function_list[num_functions - 1].size; //num_instructions = number of instructions given
          function_list[num_functions - 1].num_stack = 0; //counter for stack is 0 (new stack for new function)
          curr_index -= 5;
          size = 1;
          read_counter = 4; //at the end of our to_read array (next to read is 3 bits)
          moving_to_new_function = 0; //reset this bool
      }

      else {
          //code is our binary in an u_int8_t form
          u_int8_t code = (u_int8_t) convert_to_num(&bin_array[curr_index - to_read[read_counter] + 1], to_read[read_counter]);
          curr_index -= to_read[read_counter]; //decrease current index by opcode size (which is 3 bits)

          //read_counter = 4, meaning we need to read opcode next
          if (read_counter == 4) {

              //code = 0, means we are reading opcode MOV
              if (code == 0) {
                  strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "MOV"); //copying "MOV" string
                  function_list[num_functions - 1].instruction_list[size - 1].num_val = 2; //MOV is binary, therefore first and second type/value both exist
              }

              //code = 1, means we are reading opcode CAL
              else if (code == 1) {
                  strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "CAL"); //copying "CAL" string
                  function_list[num_functions - 1].instruction_list[size - 1].num_val = 1; //CAL is unary, therefore only first type/value exist
              }

              //code = 2, means we are reading opcode RET
              else if (code == 2) {
                  strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "RET"); //copying "RET" string
                  function_list[num_functions - 1].instruction_list[size - 1].num_val = 0; //RET is neither unary or binary

                  // Need to start new instruction set
                  num_instructions --; //decrement num_instructions

                  //we have more instructions to read (this is checked because we can have multiple RET's in a function)
                  if (num_instructions > 0) {
                      size ++; //increment size
                      //realloc memory for our next instruction_list
                      function_list[num_functions - 1].instruction_list = realloc(function_list[num_functions - 1].instruction_list, sizeof(instruction) * size);
                  }

                  //RET was called at the end of a function
                  else {
                      //getting our next function label
                      function_list[num_functions - 1].func_label = convert_to_num(&bin_array[curr_index - 3 + 1], 3);
                      curr_index -= 3; //here we are reading in the function label

                      if (curr_index < 7) {
                          finished = 1; //if we have more than one byte left, then the remainder will not be padding
                          // if we have less than a byte (< 8 bits) remaining, then it is padding
                      }

                      else {
                          moving_to_new_function = 1; //we are not finished parsing, and we are moving to a new function
                      }
                  }
                  read_counter = 5;
                }

                //code = 3, means we are reading opcode REF
                else if (code == 3) {
                    strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "REF"); //copying "REF" string
                    //REF is binary, therefore first and second type/value both exist
                    function_list[num_functions - 1].instruction_list[size - 1].num_val = 2;
                }

                //code = 4, means we are reading opcode ADD
                else if (code == 4) {
                    strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "ADD"); //copying "ADD" string
                    //ADD is binary, therefore first and second type/value both exist
                    function_list[num_functions - 1].instruction_list[size - 1].num_val = 2;
                }

                //code = 5, means we are reading opcode PRINT
                else if (code == 5) {
                    strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "PRINT"); //copying "PRINT" string
                    //PRINT is unary, therefore only first type/value exist
                    function_list[num_functions - 1].instruction_list[size - 1].num_val = 1;
                }

                //code = 6, means we are reading opcode NOT
                else if (code == 6) {
                    strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "NOT"); //copying "NOT" string
                    //NOT is unary, therefore only first type/value exist
                    function_list[num_functions - 1].instruction_list[size - 1].num_val = 1;
                }

                //code = 7, means we are reading opcode EQU
                else if (code == 7) {
                    strcpy(function_list[num_functions - 1].instruction_list[size - 1].opcode, "EQU"); //copying "EQU" string
                    //EQU is unary, therefore only first type/value exist
                    function_list[num_functions - 1].instruction_list[size - 1].num_val = 1;
                }
                //decrement read_counter (we are now either reading our first or second type) - if opcode was unary, second type wouldn't exist
                read_counter --;
            }

            //if we are reading first or second type - if opcode was unary, second type wouldn't exist
            else if (read_counter == 1 || read_counter == 3) {

                //code = 0, means we are reading type value
                if (code == 0) {
                    //if reading type value, we know the size of the preceding value, which is 8 bits in this case
                    to_read[read_counter - 1] = 8;

                    //if we are reading first type
                    if (read_counter == 3) {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_1, "VAL"); //copy string "VAL" into type_1
                    }

                    //if we are reading second type
                    else {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_2, "VAL"); //copy string "VAL" into type_2
                    }
                }

                //code = 1, means we are reading type register address
                else if (code == 1) {
                    //if reading type register address, we know the size of the preceding value, which is 3 bits in this case
                    to_read[read_counter - 1] = 3;

                    //if we are reading first type
                    if (read_counter == 3) {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_1, "REG"); //copy string "VAL" into type_1
                    }

                    //if we are reading second type
                    else {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_2, "REG"); //copy string "VAL" into type_2
                    }
                }

                //code = 2, means we are reading type stack symbol
                else if (code == 2) {
                    //if reading type stack symbol, we know the size of the preceding value, which is 5 bits in this case
                    to_read[read_counter - 1] = 5;

                    //if we are reading first type
                    if (read_counter == 3) {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_1, "STK"); //copy string "VAL" into type_1
                    }

                    //if we are reading second type
                    else {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_2, "STK"); //copy string "VAL" into type_2
                    }
                }

                //code = 3, means we are reading type pointer value
                else if (code == 3) {
                    //if reading type pointer value, we know the size of the preceding value, which is 5 bits in this case
                    to_read[read_counter - 1] = 5;

                    //if we are reading first type
                    if (read_counter == 3) {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_1, "PTR"); //copy string "VAL" into type_1
                    }

                    //if we are reading second type
                    else {
                        strcpy(function_list[num_functions - 1].instruction_list[size - 1].type_2, "PTR"); //copy string "VAL" into type_2
                    }
                }
                //decrement read_counter (we are now either reading our first or second value) - if opcode was unary, second value wouldn't exist
                read_counter --;
            }

            //if we are reading first value
            else if (read_counter == 2) {
                function_list[num_functions - 1].instruction_list[size - 1].value_1 = code; //first value is our binary converted into u_int8_t

                //if our opcode was unary, first value is our last instruction in our instruction_list
                if (function_list[num_functions - 1].instruction_list[size - 1].num_val == 1) {
                    read_counter = 4; //we know the next instruction to read is an opcode because our previous opcode was unary

                    // Need to start new instruction set
                    num_instructions --; //decrement num_instructions

                    //we have more instructions to read
                    if (num_instructions > 0) {
                        size ++; //increment size
                        //realloc memory for our next instruction_list
                        function_list[num_functions - 1].instruction_list = realloc(function_list[num_functions - 1].instruction_list, sizeof(instruction) * size);
                    }

                    //we don't have more instructions to read
                    else {
                        function_list[num_functions - 1].func_label = convert_to_num(&bin_array[curr_index - 3 + 1], 3); //getting our next function label
                        curr_index -= 3; //here we are reading in the function label

                        if (curr_index < 7) {
                            finished = 1; //if we have more than one byte left, then the remainder will not be padding
                            // if we have less than a byte (< 8 bits) remaining, then it is padding
                        }

                        else {
                            moving_to_new_function = 1; //we are not finished parsing, and we are moving to a new function
                        }
                    }
                }

                //our opcode was binary, so decrement our read_counter because we are reading second type next
                else {
                    read_counter --;
                }
            }

            //if we are reading second value (this is given our opcode was binary)
            else if (read_counter == 0) {
                function_list[num_functions - 1].instruction_list[size - 1].value_2 = code; //second value is our binary converted into u_int8_t
                read_counter = 4; //we know for sure that the next instruction to read is our opcode
                num_instructions --; //decrement num_instructions

                if (num_instructions > 0) { //we have more instructions to read
                    size ++; //increment size
                    //realloc memory for our next instruction_list
                    function_list[num_functions - 1].instruction_list = realloc(function_list[num_functions - 1].instruction_list, sizeof(instruction) * size);
                }

                else { //we don't have more instructions to read
                    function_list[num_functions - 1].func_label = convert_to_num(&bin_array[curr_index - 3 + 1], 3); //getting our next function label
                    curr_index -= 3; //here we are reading in the function label

                    if (curr_index < 7) {
                        finished = 1; //if we have more than one byte left, then the remainder will not be padding
                        // if we have less than a byte (< 8 bits) remaining, then it is padding
                    }

                    else {
                        moving_to_new_function = 1; //we are not finished parsing, and we are moving to a new function
                    }
                }
            }
        }
    }

    // SORTING OUT THE STK VALUES
    for (int i = num_functions - 1; i >= 0; i --) {

        function_list[i].stack = malloc(32); //malloc memory for our stack

        for (int j = function_list[i].size - 1; j >= 0; j --) {

            if (function_list[i].instruction_list[j].num_val >= 1) {

                //if type_1 = STK or PTR
                if (strncmp(function_list[i].instruction_list[j].type_1, "STK", 3) == 0
                || strncmp(function_list[i].instruction_list[j].type_1, "PTR", 3) == 0) {

                    //if this is the first value in our stack and stack is empty
                    if (function_list[i].num_stack == 0) {
                        function_list[i].stack[0] = function_list[i].instruction_list[j].value_1; //set our first value in our stack to value_1
                        function_list[i].num_stack ++; //increment stack counter
                    }

                    //if we are not at the start of the stack and if value_1 is not in our stack
                    if (contains(function_list[i].stack, function_list[i].instruction_list[j].value_1, function_list[i].num_stack) == -1) {
                        //setting the most recent index in our stack to value_1
                        function_list[i].stack[function_list[i].num_stack] = function_list[i].instruction_list[j].value_1;
                        function_list[i].num_stack ++; //increment stack counter
                    }
                }
            }

            else if (function_list[i].instruction_list[j].num_val == 2) {

                //if type_1 = STK or PTR
                if (strncmp(function_list[i].instruction_list[j].type_2, "STK", 3) == 0
                || strncmp(function_list[i].instruction_list[j].type_2, "PTR", 3) == 0) {

                    //if this is the first value in our stack and stack is empty
                    if (function_list[i].num_stack == 0) {
                      function_list[i].stack[0] = function_list[i].instruction_list[j].value_2; //set our first value in our stack to value_2
                      function_list[i].num_stack ++; //increment stack counter
                    }

                    //if we are not at the start of the stack and if value_2 is not in our stack
                    if (contains(function_list[i].stack, function_list[i].instruction_list[j].value_2, function_list[i].num_stack) == -1) {
                      //setting the most recent index in our stack to value_2
                      function_list[i].stack[function_list[i].num_stack] = function_list[i].instruction_list[j].value_2;
                      function_list[i].num_stack ++; //increment stack counter
                    }
                }
            }
        }
    }

    //PRINTING CORRECT OUTPUT
    char stack_characters[32] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f'};

    for (int j = num_functions - 1;  j >= 0; j --) {
        printf("FUNC LABEL %d\n", function_list[j].func_label); //printing function label

        for (int i = function_list[j].size - 1; i >= 0; i --) {
            printf("    %s", function_list[j].instruction_list[i].opcode); //printing opcode

            //if we don't encounter a RET opcode
            if (strcmp(function_list[j].instruction_list[i].opcode, "RET") != 0) {
                printf(" %s", function_list[j].instruction_list[i].type_1); //printing type_1

                //if first type is STK or PTR
                if (strcmp(function_list[j].instruction_list[i].type_1, "STK") == 0
                || strcmp(function_list[j].instruction_list[i].type_1, "PTR") == 0) {
                    //getting the index of where value_1 is in our stack
                    int index = contains(function_list[j].stack, function_list[j].instruction_list[i].value_1, function_list[j].num_stack);
                    function_list[j].instruction_list[i].value_1 = index; //setting our value_1 to the index found
                    printf(" %c", stack_characters[function_list[j].instruction_list[i].value_1]); //printing value_1
                }

                //our first type is VAL or REG
                else {
                    printf(" %d", function_list[j].instruction_list[i].value_1); //printing value_1
                }

                //if our opcode is MOV, ADD or REF
                if (strcmp(function_list[j].instruction_list[i].opcode, "MOV") == 0
                || strcmp(function_list[j].instruction_list[i].opcode, "ADD") == 0
                || strcmp(function_list[j].instruction_list[i].opcode, "REF") == 0) {
                    printf(" %s ", function_list[j].instruction_list[i].type_2); //printing our type_2

                    //if our second type is STK or PTR
                    if (strcmp(function_list[j].instruction_list[i].type_2, "STK") == 0
                    || strcmp(function_list[j].instruction_list[i].type_2, "PTR") == 0) {
                        //getting the index of where value_2 is in our stack
                        int index = contains(function_list[j].stack, function_list[j].instruction_list[i].value_2, function_list[j].num_stack);
                        function_list[j].instruction_list[i].value_2 = index; //setting our value_2 to the index found
                        printf("%c", stack_characters[function_list[j].instruction_list[i].value_2]); //printing value_2
                    }

                    //our second type is VAL or REG
                    else {
                        printf("%d", function_list[j].instruction_list[i].value_2);
                    }
                }
            }
            printf("\n");
        }
    }

    //FREEING MEMORY
    for (int i = 0; i < num_functions; i ++) {
        free(function_list[i].instruction_list);
        free(function_list[i].stack);
    }
    free(function_list);
    free(bin_array);

    return 0;
}
