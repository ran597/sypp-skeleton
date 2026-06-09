#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions


// see ijvm.h for descriptions of the below functions

ijvm* init_ijvm(char *binary_path, FILE* input, FILE* output)
{
  // do not change the lines above "TODO: implement me"
  ijvm* m = (ijvm *) malloc(sizeof(ijvm));
  // note that malloc gives you memory, but gives no guarantees on the initial
  // values of that memory. It might be all zeroes, or be random data.
  // It is hence important that you initialize all variables in the ijvm
  // struct and do not assume these are set to zero.
  m->in = input;
  m->out = output;
  
  // TODO: implement me
  m->constant_pool = NULL;
  m->constant_pool_size = 0;
  m->text_data = NULL;
  m->text_size = 0;
  m->Stack = malloc(m->stackSize * sizeof(int32_t));
  m->stackSize = 128;
  m->stackCapacity = 0;
  m->pc = 0;
  m->is_halted = false;


  FILE *fp = fopen(binary_path, "rb");
  uint8_t numbuf[4];

  fread(numbuf, sizeof(uint8_t), 4, fp);
  uint32_t number = read_uint32(numbuf);

  if (number != 0x1DEADFAD){
    free(m);
    fclose(fp); 
    return NULL;
  }

  //constant pool orgin

  fread(numbuf, sizeof(uint8_t), 4, fp);

  //constant pool size
  fread(numbuf, sizeof(uint8_t), 4, fp);
  uint32_t constSize = read_uint32(numbuf);

  //constant pool data
  m->constant_pool_size = constSize; 

  m->constant_pool = malloc(constSize); 

  fread(m->constant_pool, sizeof(uint8_t),constSize, fp);

  //text block orgin
  fread(numbuf, sizeof(uint8_t), 4, fp);

 //text block size
  fread(numbuf, sizeof(uint8_t), 4, fp);
  uint32_t textSize = read_uint32(numbuf);

  //text bloack data
  m->text_size = textSize;

  m->text_data = malloc(textSize);

  fread(m->text_data, sizeof(uint8_t), textSize, fp);


  fclose(fp);

  return m;
}

void destroy_ijvm(ijvm* m) 
{
  // TODO: implement me
  free(m->constant_pool);
  free(m->text_data);

  free(m); // free memory for struct
}

uint8_t *get_text(ijvm* m) 
{
  // TODO: implement me
  return m->text_data;
}

uint32_t get_text_size(ijvm* m) 
{
  // TODO: implement me
  return m->text_size;
}

int32_t get_constant(ijvm* m, uint32_t i) 
{
  // TODO: implement me
  return read_uint32(&m->constant_pool[i*4]);
}

uint32_t get_program_counter(ijvm* m) 
{
  // TODO: implement me
  return m->stackCapacity;
}

int32_t tos(ijvm* m) 
{
  // this operation should NOT pop (remove top element from stack)
  // TODO: implement me
  return m->Stack[m->stackSize - 1];
}

bool finished(ijvm* m) 
{
  // TODO: implement me
  if (get_program_counter(m) >= get_text_size(m)) {
        return true;
    }

  if (m->is_halted == true) {
      return true;
    }

    return false;
}

int32_t get_local_variable(ijvm* m, uint32_t i) 
{
  // TODO: implement me
  return 0;
}

int32_t stack_pop(ijvm* m) 
{
    if(m->stackSize == 0){
      return 0;
    }
    else{
      m->stackSize--;
      return m->Stack[m->stackSize];
    }
}

int32_t stack_top(ijvm* m) 
{
    return m->Stack[m->stackSize - 1];
}

void stack_push(ijvm* m, int32_t value) 
{
    if (m->stackSize >= m->stackCapacity) 
    {
        m->stackCapacity *= 2;
        m->Stack = realloc(m->Stack, m->stackCapacity * sizeof(int32_t));
    }
    
    m->Stack[m->stackSize] = value;
    m->stackSize++;
}


void step(ijvm* m) 
{
  // TODO: implement me
  uint8_t instruction = getinstruction(m);

  m->pc++;

  switch(instruction)
  {
    case OP_BIPUSH: 
      {
        uint8_t raw_byte = get_text(m)[get_program_counter(m)];
        m->pc++; 
        int8_t signed_byte = (int8_t)raw_byte;
        stack_push(m, (int32_t)signed_byte);
      }
      break;
    
    case OP_DUP: 
      {
        stack_push(m, stack_top(m));
      }
      break;

    case OP_IADD: 
      {
        int32_t val2 = stack_pop(m);
        int32_t val1 = stack_pop(m);
        stack_push(m, val1 + val2);
      }
      break;
      
    case OP_ISUB: 
      {
        int32_t val2 = stack_pop(m);
        int32_t val1 = stack_pop(m);
        stack_push(m, val1 - val2);
      }
      break;

    case OP_IAND: 
      {
        int32_t val2 = stack_pop(m);
        int32_t val1 = stack_pop(m);
        stack_push(m, val1 & val2);
      }
      break;

    case OP_IOR: 
      {
        int32_t val2 = stack_pop(m);
        int32_t val1 = stack_pop(m);
        stack_push(m, val1 | val2);
      }
      break;

    case OP_POP: 
      {
        stack_pop(m);
      }
      break;

    case OP_SWAP: 
      {
        int32_t top_val = stack_pop(m);
        int32_t second_val = stack_pop(m);
        stack_push(m, top_val);
        stack_push(m, second_val);
      }
      break;

    case OP_NOP: 
      break;

    case OP_ERR: 
      {
        fprintf(m->out, "Error\n");
        m->is_halted = true; 
      }
      break;
      
    case OP_HALT: 
      {
        m->is_halted = true;
      }
      break;

    case OP_IN: 
      {
        int ch = fgetc(m->in);
        if (ch == EOF) 
        {
          stack_push(m, 0);
        } 
        else 
        {
          stack_push(m, ch);
        }
      }
      break;

    case OP_OUT: 
      {
        int32_t val = stack_pop(m);
        fprintf(m->out, "%c", val);
        fflush(m->out);
      }
      break;

    default:
      {
        m->is_halted = true;
      }
      break;
    
  }

}

uint8_t get_instruction(ijvm* m) 
{ 
  return get_text(m)[get_program_counter(m)]; 
}

ijvm* init_ijvm_std(char *binary_path) 
{
  return init_ijvm(binary_path, stdin, stdout);
}

void run(ijvm* m) 
{
  while (!finished(m)) 
  {
    step(m);
  }
}


// Below: methods needed by bonus assignments, see ijvm.h
// You can leave these unimplemented if you are not doing these bonus 
// assignments.

uint32_t get_call_stack_size(ijvm* m) 
{
   // TODO: implement me if doing tail call bonus
   return 0;
}


// Checks if reference is a freed heap array. Note that this assumes that 
// 
bool is_heap_freed(ijvm* m, uint32_t reference) 
{
   // TODO: implement me if doing garbage collection bonus
   return false;
}

// Checks if top of stack is a reference
bool is_tos_reference(ijvm* m)
{
	// TODO: implement me if doing precise garbage collection bonus
	//  using ANEWARRAY, AIALOAD and AIASTORE
	return false;
}
