#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include "lc3_asm.h"

#define filename_length  30
#define illeagle_imm     65537
/** Global variable containing the head of the linked list of structures */
static line_info_t* infoHead;
/** Global variable containing the tail of the linked list of structures */
static line_info_t* infoTail;
/** Global variable containing information about the current line */
static line_info_t* currInfo;
/** 第一次遍历建立的 lable 地址表 **/
sym_table_t* lc3_sym_tab ;

static void usage (void) {
  fprintf(stderr, "Usage: lc3as [-hex] <ASM filename>\n");
  exit (1);
}

static char* check_for_asm_file (char* fname) {
  int len = strlen(fname);
  
  if (len >= 5) { // mininum length name is x.asm
    char* suffix = fname + len -4;
    if (strcmp(suffix, ".asm") == 0)
      return suffix;
  }

  return 0;
}  
void asm_error (char* msg, ...){
    numErrors++;
    va_list argp;
    fprintf(stderr, "ERROR %3d: ", srcLineNum);
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
    fprintf(stderr, "\n");
}
FILE *open_read_or_error (char* file_name) {
	FILE* fp = fopen(file_name,"r");
	if(fp == NULL){
        numErrors++;
		asm_error(ERR_OPEN_READ,file_name);
        exit(0);
	}  
	return fp;
}
void asm_init_line_info (line_info_t* info) {
  if (info) {
    info->next        = NULL;
    info->token_sprt.part_num = 0;
    info->lineNum     = srcLineNum;
    info->address     = currAddr;
    info->op_begin    = 0;
    info->machineCode = 0;
    info->opcode      = OP_INVALID;
    info->form        = 0;
    info->reg1        = -1;
    info->reg2        = -1;
    info->reg3        = -1;
    info->immediate   = 0;
    info->reference   = NULL;
  }
}
void line_sprt_by_space(char *line,line_sprt* token_sprt){ //为方便起见，先允许 可以对连续逗号不报错。
  //  struct line_sprt token_sprt;
    int i=0, tag=0, part_ith=0 , str_length = 0;;
    char *tmp_head;
    tag = 0 ;   //0 表示空格，1 表示非空格
    while(line[i] !='\n'&& line[i]!=';' && line[i]!= 0 && i <MAX_LINE_LENGTH ){ //没有结束
            if(line[i]=='"'){   //引号里的空格不算 。
              if(!tag){
                    tmp_head = line + i + 1 ;
                    str_length = 0;
                    while(tmp_head[str_length]!= '"' && tmp_head[str_length]!= 0 && str_length <MAX_LINE_LENGTH-i){
                        str_length ++;
                    }
                    if(tmp_head[str_length] == 0 ||str_length >MAX_LINE_LENGTH-i){
                        numErrors++;
                        printf(ERR_WRONG_FORMAT); 
                    }
                    
                    i = i + str_length + 1;
                    tag = 0;
                    tmp_head[str_length] = 0 ;
                    strcpy(token_sprt->list[part_ith++],tmp_head) ;  //吧引号后的内容复制进来,
                    
                    }
              else{
                    numErrors++;
                    printf(ERR_WRONG_FORMAT);
                    tag = 0;
              }             
            }
            else{
                if(line[i]==' '|| line[i] == ','){
                    if(tag) {
                        line[i] = 0;
                        tag = 0;
                        strcpy(token_sprt->list[part_ith++],tmp_head);
                    }
                }
                else {
                    if(!tag){  
                        tmp_head = line + i;
                        tag = 1;
                    }
                }
            }
            i++;
    }
    if(tag){
        line[i] = 0;
        strcpy(token_sprt->list[part_ith++],tmp_head);
    } 
    token_sprt->part_num = part_ith;
  
//    for(i=0;i<token_sprt->part_num;i++){
//        printf("%s\n",token_sprt->list[i]);
//    }
//     printf("%d\n",token_sprt->part_num);

}


bool symbol_tab_add(char *s , int addr , sym_table_t* lc3_sym_tab){//并检查是否重复
     sym_table_t*  symbol_note;
     symbol_note = lc3_sym_tab;
     while(symbol_note->next != NULL){
       if(strcmp(s,symbol_note->symbol.name)==0){ 
           asm_error( ERR_BAD_LABEL,s);
           return  false;
       }
       symbol_note = symbol_note->next;
     }
     symbol_note->next = malloc(sizeof(sym_table_t));
     symbol_note->symbol.name = s;
     symbol_note->symbol.addr = addr;
     symbol_note->next->next = NULL;
     return  true;
}
opcode_t scanf_OPorLB(char *token){
    opcode_t code;
 //   int format = 0;
    if (strcasecmp(token,"BR")==0){
        code = OP_BR; //format = 0;
    }else   if (strcasecmp(token,"BRn")==0){
        code = OP_BRN; // format = 1;
    }else  if (strcasecmp(token,"BRz")==0){
        code = OP_BRZ; //format = 2;
    }else  if (strcasecmp(token,"BRp")==0){
        code = OP_BRP; //format = 3;
    }else  if (strcasecmp(token,"BRnz")==0){
        code = OP_BRNZ; //format = 4;
    }else  if (strcasecmp(token,"BRnp")==0){
        code = OP_BRNP; //format = 5;
    }else  if (strcasecmp(token,"BRzp")==0){
        code = OP_BRZP; //format = 6;
    }else  if (strcasecmp(token,"BRnzp")==0){
        code = OP_BRNZP; //format = 7;
    }else  if (strcasecmp(token,"ADD")==0){
        code = OP_ADD;// if(token_sprt.list[token_sprt.part_num-1][0]=='R') format += 1 ; //表示加寄存器
    }else   if (strcasecmp(token,"LD")==0){
        code = OP_LD ;
    }else  if (strcasecmp(token,"ST")==0){
        code = OP_ST ;
    }else  if (strcasecmp(token,"JSR")==0){
        code = OP_JSR; //format = 1 ;
    }else  if (strcasecmp(token,"JSRR")==0){
        code = OP_JSRR ; //format = 0 ;
    }else  if (strcasecmp(token,"AND")==0){
        code = OP_AND; //if(token_sprt.list[token_sprt.part_num-1][0]=='R') format += 1 ; //表示与寄存器
    }else  if (strcasecmp(token,"LDR")==0){
        code = OP_LDR;
    }else  if (strcasecmp(token,"STR")==0){
        code = OP_STR;
    }else  if (strcasecmp(token,"RET")==0){
        code = OP_RTI;
    }else  if (strcasecmp(token,"RTI")==0){
        code = OP_RTI;
    }else if (strcasecmp(token,"NOT")==0){
        code = OP_NOT;
    }else  if (strcasecmp(token,"LDI")==0){
        code = OP_LDI;
    }else  if (strcasecmp(token,"STI")==0){
        code = OP_STI;
    }else  if (strcasecmp(token,"JMP")==0){
        code = OP_JMP_RET;
    }else  if (strcasecmp(token,"RESERVED")==0){
         
    }else  if (strcasecmp(token,"LEA")==0){
        code = OP_LEA;
    }else  if (strcasecmp(token,"TRAP")==0){
        code = OP_TRAP;
    }else  if (strcasecmp(token,".ORIG")==0){
        code = OP_ORIG;
    }else if (strcasecmp(token,".END")==0){
        code = OP_END;
    }else if (strcasecmp(token,".BLKW")==0){
        code = OP_BLKW;
    }else if (strcasecmp(token,".FILL")==0){
        code = OP_FILL;
    }else if (strcasecmp(token,".STRINGZ")==0){
        code = OP_STRINGZ;
    }else if (strcasecmp(token,"getc")==0){
        code = OP_GETC;
    }else if (strcasecmp(token,"out")==0){
        code = OP_OUT;
    }else if (strcasecmp(token,"puts")==0){
        code = OP_PUTS;
    }else if (strcasecmp(token,"in")==0){
        code = OP_IN ;
    }else if (strcasecmp(token,"halt")==0){
        code = OP_HALT;
    }else if (strcasecmp(token,"gets")==0){
        code = OP_GETS;
    }else {
       code = OP_INVALID ;
    }

    return code;
}
/*可以扩展，随时想到，随时扩展*/
bool is_reserve_word(char *token){  
    if (strcasecmp(token,"BR")==0){
        return false;
    }else   if (strcasecmp(token,"BRn")==0){
        return false;
    }else  if (strcasecmp(token,"BRz")==0){
        return false;
    }else  if (strcasecmp(token,"BRp")==0){
       return false;
    }else  if (strcasecmp(token,"BRnz")==0){
       return false;
    }else  if (strcasecmp(token,"BRnp")==0){
       return false;
    }else  if (strcasecmp(token,"BRzp")==0){
       return false;
    }else  if (strcasecmp(token,"BRnzp")==0){
       return false;
    }else  if (strcasecmp(token,"ADD")==0){
       return false;
    }else   if (strcasecmp(token,"LD")==0){
       return false;
    }else  if (strcasecmp(token,"ST")==0){
      return false;
    }else  if (strcasecmp(token,"JSR")==0){
       return false;
    }else  if (strcasecmp(token,"JSRR")==0){
       return false;
    }else  if (strcasecmp(token,"AND")==0){
       return false;
    }else  if (strcasecmp(token,"LDR")==0){
       return false;
    }else  if (strcasecmp(token,"STR")==0){
      return false;
    }else  if (strcasecmp(token,"RTI")==0){
     return false;
    }else if (strcasecmp(token,"NOT")==0){
      return false;
    }else  if (strcasecmp(token,"LDI")==0){
      return false;
    }else  if (strcasecmp(token,"STI")==0){
      return false;
    }else  if (strcasecmp(token,"JMP")==0){
       return false;
    }else  if (strcasecmp(token,"RESERVED")==0){
        return false;
    }else  if (strcasecmp(token,"LEA")==0){
      return false;
    }else  if (strcasecmp(token,"TRAP")==0){
       return false;
    }else  if (strcasecmp(token,".ORIG")==0){
       return false;
    }else if (strcasecmp(token,".END")==0){
      return false;
    }else if (strcasecmp(token,".BLKW")==0){
       return false;
    }else if (strcasecmp(token,".FILL")==0){
       return false;
    }else if (strcasecmp(token,".STRINGZ")==0){
        return false;
    }else if (strcasecmp(token,"getc")==0){
      return false;
    }else if (strcasecmp(token,"out")==0){
      return false;
    }else if (strcasecmp(token,"puts")==0){
      return false;
    }else if (strcasecmp(token,"in")==0){
     return false;
    }else if (strcasecmp(token,"halt")==0){
       return false;
    }else if (strcasecmp(token,"R0")==0){
       return false;
    }else if (strcasecmp(token,"R1")==0){
       return false;
    }else if (strcasecmp(token,"R2")==0){
       return false;
    }else if (strcasecmp(token,"R3")==0){
       return false;
    }else if (strcasecmp(token,"R4")==0){
       return false;
    }else if (strcasecmp(token,"R5")==0){
       return false;
    }else if (strcasecmp(token,"R6")==0){
       return false;
    }else if (strcasecmp(token,"R7")==0){
       return false;
    }else {
       return true;
    }
}

bool get_lable_or_error(char *token){
    bool is_lable ;
    if(is_reserve_word(token)){
        return true;
    }
    return false;
}
int get_reg_or_error (char *token) {
  int get ;
  if (strcasecmp(token,"R0")==0){
       get = 0;
    }else if (strcasecmp(token,"R1")==0){
       get = 1;
    }else if (strcasecmp(token,"R2")==0){
       get = 2;
    }else if (strcasecmp(token,"R3")==0){
       get = 3;
    }else if (strcasecmp(token,"R4")==0){
       get = 4;
    }else if (strcasecmp(token,"R5")==0){
       get = 5;
    }else if (strcasecmp(token,"R6")==0){
       get = 6;
    }else if (strcasecmp(token,"R7")==0){
       get = 7;
    }else {
       get = -1;
    }

  return get;
}
int get_imm_or_error(char *token){  //无法处理 ，错误形式的立即数 ;
    int imm = 0;
    int i ;
    if(token[0] == '#'){
       i = 1;
 //      while (token[i]!= 0 )
       {
            if(token[i]>='0'&&token[i]<=9 ){
                i++ ;
            }else{
                imm = illeagle_imm;
            }
       }
       if(imm != illeagle_imm)   sscanf(&token[1],"%d" ,&imm);
      
    }else if(token[0] == 'x'){
        if(sscanf(&token[1],"%x" ,&imm) == EOF){
            imm = illeagle_imm;
        }
    }else{
        if(sscanf(&token[0],"%d" ,&imm) == EOF){
            imm = illeagle_imm;
        }
    }

    return imm;
}

void judge_reg_lable(line_info_t* currInfo){
        int op_ith;
        int R_ith;
        op_ith = currInfo->op_begin;
        if(currInfo->token_sprt.part_num - op_ith == 3 ){  //先判断参数数量
             R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+1]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+1] );
              }else{
                  currInfo->reg1 = R_ith;
              }
              if(!get_lable_or_error(currInfo->token_sprt.list[op_ith+2])){
                  asm_error(ERR_BAD_LABEL,currInfo->token_sprt.list[op_ith+2]);
              }
        }else{
              asm_error(ERR_WRONG_FORMAT);
        }           
}
void judge_lable(line_info_t* currInfo){
         
         if(currInfo->token_sprt.part_num - currInfo->op_begin == 2 && get_lable_or_error(currInfo->token_sprt.list[1+currInfo->op_begin])) ;
         else{
             asm_error(ERR_WRONG_FORMAT);
         }
}
void judge_reg_reg_regorimm(line_info_t* currInfo){
    int R_ith;
    int op_ith;
    int imm;
     op_ith = currInfo->op_begin;
         if(currInfo->token_sprt.part_num - op_ith == 4 ){
              R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+1]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+1] );
              }else{
                  currInfo->reg1 = R_ith;
              }
              R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+2]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+2] );
              }else{
                  currInfo->reg2 = R_ith;
              }
              R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+3]) ;
              if(R_ith == -1){
                  imm = get_imm_or_error(currInfo->token_sprt.list[op_ith+3]);
                  if(imm == illeagle_imm)   asm_error(ERR_EXPECT_REG_IMM, currInfo->token_sprt.list[op_ith+3] );
                  else{
                    currInfo->immediate = imm;
                    currInfo->form = 1;
                  }
              }else{
                  currInfo->reg3 = R_ith;
                  currInfo->form = 0;
              }
         }else{
              asm_error(ERR_WRONG_FORMAT);
         }
}
void judge_reg_reg_imm(line_info_t* currInfo){
    int R_ith;
    int op_ith;
    int imm;
     op_ith = currInfo->op_begin;
         if(currInfo->token_sprt.part_num - op_ith == 4 ){
              R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+1]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+1] );
              }else{
                  currInfo->reg1 = R_ith;
              }
              R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+2]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+2] );
              }else{
                  currInfo->reg2 = R_ith;
              }
             
              imm = get_imm_or_error(currInfo->token_sprt.list[op_ith+3]);
              if(imm == illeagle_imm)   asm_error(ERR_EXPECT_REG_IMM, currInfo->token_sprt.list[op_ith+3] );
              else{
                  currInfo->immediate = imm;
                  currInfo->form = 1;
               }
             
              
         }else{
              asm_error(ERR_WRONG_FORMAT);
         }
}
void judge_reg(line_info_t* currInfo){
    int R_ith;
    int op_ith;
    op_ith = currInfo->op_begin;
        if(currInfo->token_sprt.part_num - op_ith == 2 ){  //先判断参数数量
             R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+1]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+1] );
              }else{
                  currInfo->reg1 = R_ith;
              }
        }else{
              asm_error(ERR_WRONG_FORMAT);
        }  
}
void judge_imm(line_info_t* currInfo){
     int imm;
     int op_ith;
     op_ith = currInfo->op_begin;
     if(currInfo->token_sprt.part_num - op_ith == 2 ){  //先判断参数数量
        imm = get_imm_or_error(currInfo->token_sprt.list[op_ith+1]);
        if(imm == illeagle_imm)   asm_error(ERR_EXPECT_REG_IMM, currInfo->token_sprt.list[op_ith+1] );
        else{
            currInfo->immediate = imm;
            currInfo->form = 1;
        }    
     }else{
         asm_error(ERR_WRONG_FORMAT);
    }  
}
void judge_empty(line_info_t* currInfo){
    int op_ith;
    op_ith = currInfo->op_begin;
        if(currInfo->token_sprt.part_num - op_ith == 1 ){ //先判断参数数量
         }
         else  asm_error(ERR_WRONG_FORMAT);   
}
void judge_lableorimm(line_info_t* currInfo){
    int op_ith;
    op_ith = currInfo->op_begin;
        if(currInfo->token_sprt.part_num - op_ith == 2 ){ //先判断参数数量
          if(get_lable_or_error(currInfo->token_sprt.list[op_ith+1])){
          }
          else{
          if(get_imm_or_error(currInfo->token_sprt.list[op_ith+1])){   
          }
           else asm_error(ERR_EXPECT_lable_IMM,currInfo->token_sprt.list[op_ith+1]);   
          }

        }
         else  asm_error(ERR_WRONG_FORMAT);   
}
void judge_reg_reg(line_info_t* currInfo){
 int R_ith;
    int op_ith;
    op_ith = currInfo->op_begin;
         if(currInfo->token_sprt.part_num - op_ith == 3 ){
              R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+1]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+1] );
              }else{
                  currInfo->reg1 = R_ith;
              }
              R_ith = get_reg_or_error(currInfo->token_sprt.list[op_ith+2]) ;
              if(R_ith == -1){
                  asm_error(ERR_EXPECTED_REG, currInfo->token_sprt.list[op_ith+2] );
              }else{
                  currInfo->reg2 = R_ith;
              }   
         }else{
              asm_error(ERR_WRONG_FORMAT);
         }
}
void analyse_op(line_info_t* currInfo){
     int R_ith;
     int op_ith;
     bool is_lable;
     int imm ;
     switch (currInfo->opcode)
     {
     case OP_BR: 
         judge_lable(currInfo);
         break;
     case OP_BRN: 
          judge_lable(currInfo);
         break;
     case OP_BRZ:
        judge_lable(currInfo); 
         break;
     case OP_BRP:
         judge_lable(currInfo);
         break;
    case OP_BRNZ:
         judge_lable(currInfo);
         break;
    case OP_BRNP:
         judge_lable(currInfo);
         break;
    case OP_BRZP:
         judge_lable(currInfo);
         break; 
    case OP_BRNZP:
         judge_lable(currInfo); 
         break;
    case OP_ADD: 
         judge_reg_reg_regorimm(currInfo);
         break;
    case OP_LD:
         judge_reg_lable(currInfo); 
         break;
    case OP_ST:
         judge_reg_lable(currInfo);
         break;
    case OP_JSR:
         judge_lable(currInfo);
         break;     
    case OP_JSRR:
          judge_reg(currInfo); 
         break;
    case OP_AND:
        judge_reg_reg_regorimm(currInfo);
         break;
    case OP_LDR:
        judge_reg_reg_imm(currInfo);
         break;
    case OP_STR:
         judge_reg_reg_imm(currInfo);
         break;
    case OP_RTI:
        op_ith = currInfo->op_begin;
        if(currInfo->token_sprt.part_num - op_ith == 1 ){ //先判断参数数量
           if(strcasecmp(currInfo->token_sprt.list[op_ith+1],"ret") == 0){
               currInfo->form = 0;
           }
           else{
               currInfo->form = 1;
           }
         }
         else  asm_error(ERR_WRONG_FORMAT);   
         break;    
     case OP_NOT: 
        judge_reg_reg(currInfo);
         break;
    case OP_LDI: 
        judge_reg_lable(currInfo);  
         break;   
    case OP_STI: 
         judge_reg_lable(currInfo); 
         break;
    case OP_JMP_RET:
        judge_reg(currInfo);
         break;
    case OP_RESERVED: 
         break;
    case OP_LEA:
         judge_reg_lable(currInfo);
         break;
    case OP_TRAP: 
        judge_imm(currInfo);
         break;
    case OP_ORIG: 
        judge_imm(currInfo);
         break;
    case OP_END: 
        judge_empty(currInfo);
         break;
    case OP_BLKW:
        judge_imm(currInfo); 
         break;
    case OP_FILL: 
        judge_lableorimm(currInfo);
         break;
    case OP_STRINGZ:
         if(currInfo->token_sprt.part_num - currInfo->op_begin == 2 ) ;
         else{
             asm_error(ERR_WRONG_FORMAT);
         } 
         break;
    case OP_GETC: 
       judge_empty(currInfo);
         break;
    case OP_OUT: 
        judge_empty(currInfo);
         break;
     case OP_PUTS:
        judge_empty(currInfo);
         break;
    case OP_IN:
        judge_empty(currInfo);
         break;
    case OP_HALT: 
        judge_empty(currInfo);
         break;
    case OP_GETS: 
         judge_empty(currInfo);
         break;                                                                                                                                                                    
     default:
         break;
     }
}

void check_line_syntax (line_info_t *currInfo) {
   printf("check_line_syntax('%s')\n", currInfo->token_sprt.list[0]);
   int ret ; //记录返回值
   opcode_t opcode ;
   // 获取lable.
   //获取操作数信息。
   opcode = scanf_OPorLB(currInfo->token_sprt.list[0]) ; //检查第一句
   if (opcode ==  OP_INVALID ){
        //计算curr_info_addrs; 
        if(!symbol_tab_add(currInfo->token_sprt.list[0] ,currInfo->address,lc3_sym_tab)){
            return ;
        }
        opcode = scanf_OPorLB(currInfo->token_sprt.list[1]);
 //        printf("add_table_succ and new op is %s =  %d\n", currInfo->token_sprt.list[1],opcode);
        if(opcode != OP_INVALID ){
            currInfo->opcode = opcode;
            currInfo->op_begin = 1;
        }
        else{
           asm_error(ERR_ILLEAGLE_INST);
           return ;
        }
   }
   else {
        currInfo->opcode = opcode;
        currInfo->op_begin = 0;
        }
   
   analyse_op(currInfo);
    

}

void asm_pass_one (char* asm_file_name, char* sym_file_name){
    int i,tag ,inst_ith = 0;
    char *p;
	char line[MAX_LINE_LENGTH];
    currAddr = 0;
    //open the souce file
    FILE* fp = open_read_or_error(asm_file_name);
    //建立一个空的符号表
    lc3_sym_tab = malloc(sizeof(sym_table_t));
    lc3_sym_tab->next = NULL ;
    lc3_sym_tab->symbol.addr = -1;
    lc3_sym_tab->symbol.name = "";
  
    //开始读取信息
    while(fgets(line,MAX_LINE_LENGTH,fp)){//读取一行
        srcLineNum ++;
    //    printf("%d行读到的内容是%s\n",srcLineNum,line);
        line_sprt  *token;
        token = malloc(sizeof(line_sprt));
        line_sprt_by_space(line,token);
        //while my token is not null
        if(token->part_num != 0 ){
            currInfo = malloc(sizeof(struct line_info));
            //idk what this does but i'm doing it
            currAddr++ ;
            //读取行的 信息 。
            asm_init_line_info(currInfo);

            for(i=0;i<token->part_num;i++){
                 strcpy(currInfo->token_sprt.list[i], token->list[i]);
            }
            currInfo->token_sprt.part_num = token->part_num;
           
           
  //          printf("%d\n%s\n%s\n%d",currInfo->address,currInfo->token_sprt.list[0],currInfo->token_sprt.list[1],currInfo->token_sprt.part_num);
            check_line_syntax( currInfo );
  //          printf("op_code %d\nreg1 %d\nreg2 %d\nreg3 %d \nimm %d\n",currInfo->opcode,currInfo->reg1,currInfo->reg2,currInfo->reg3,currInfo->immediate );
        }
    
    }
}

int main(int argc, char* argv[]){
    char* asm_file = argv[argc -1];
    char obj_file[filename_length];
    char* obj = obj_file;
    //检查文件是否asm 。
    if (! check_for_asm_file(asm_file))
    usage(); // this exits
    int len;
    len  = strlen(asm_file);
    strcpy(obj,asm_file);
    strcpy(obj + len - 4,".obj");
    

    numErrors = 0;
    srcLineNum = 0;

//  printf("%s   %s\n",asm_file,obj_file);
    printf("STARTING PASS 1...\n");
    asm_pass_one(asm_file, obj_file);
    printf("%d errors found in first pass\n", numErrors);
}
