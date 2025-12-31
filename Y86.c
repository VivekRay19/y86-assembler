#include <stdio.h>
#include <stdlib.h>

#define MAX_LINES 200
#define MAX_LEN 200
#define MAX_LABELS 100
#define OUT_SIZE 10000
#define CODE_BASE 0x100

/* ===== Global Variables ===== */
char label_names[MAX_LABELS][50];
int label_addrs[MAX_LABELS];
int label_count = 0;

char lines[MAX_LINES][MAX_LEN];
int line_count = 0;
unsigned char out[OUT_SIZE];
int outpos = 0;

/* ===== String Helpers ===== */
int str_len(char s[]) {
    int i=0;
    while(s[i]!='\0') i++;
    return i;
}

void str_cpy(char dest[], char src[]) {
    int i=0;
    while(src[i]!='\0') {
        dest[i]=src[i];
        i++;
    }
    dest[i]='\0';
}

int str_eq(char a[], char b[]) {
    int i=0;
    while(a[i]!='\0' && b[i]!='\0') {
        if(a[i]!=b[i]) return 0;
        i++;
    }
    return a[i]=='\0' && b[i]=='\0';
}

void trim(char s[]) {
    int i=0, j=0;
    while(s[i]==' '||s[i]=='\t') i++;
    while(s[i]!='\0') s[j++]=s[i++];
    s[j]='\0';
    j--;
    while(j>=0 && (s[j]==' '||s[j]=='\t'||s[j]=='\n'||s[j]=='\r')) s[j--]='\0';
}

/* ===== Emit Functions ===== */
void emit_byte(unsigned char b) {
    if(outpos<OUT_SIZE) out[outpos++]=b;
}

void emit_int(int val) {
    emit_byte(val & 0xFF);
    emit_byte((val>>8)&0xFF);
    emit_byte((val>>16)&0xFF);
    emit_byte((val>>24)&0xFF);
}

/* ===== Register ID ===== */
int reg_id(char r[]) {
    char temp[10];
    int i=0, j=0;
    if(r[0]=='%') i=1;
    while(r[i]!='\0') temp[j++]=r[i++];
    temp[j]='\0';

    if(str_eq(temp,"eax")) return 0;
    if(str_eq(temp,"ecx")) return 1;
    if(str_eq(temp,"edx")) return 2;
    if(str_eq(temp,"ebx")) return 3;
    if(str_eq(temp,"esp")) return 4;
    if(str_eq(temp,"ebp")) return 5;
    if(str_eq(temp,"esi")) return 6;
    if(str_eq(temp,"edi")) return 7;
    return 0;
}

int parse_int(char s[]) {
    if(s[0]=='0' && s[1]=='x') return (int)strtol(s,NULL,16);
    return (int)strtol(s,NULL,10);
}

/* ===== Label Functions ===== */
void add_label(char name[], int addr) {
    int i=0;
    while(name[i]!='\0') {
        label_names[label_count][i]=name[i];
        i++;
    }
    label_names[label_count][i]='\0';
    label_addrs[label_count]=addr;
    label_count++;
}

int find_label(char name[]) {
    int i;
    for(i=0;i<label_count;i++) {
        if(str_eq(label_names[i],name)) return label_addrs[i];
    }
    return 0;
}

/* ===== Tokenize ===== */
int tokenize(char line[], char tokens[][50]) {
    int i=0,j=0,k=0;
    while(line[i]!='\0') {
        if(line[i]==' '||line[i]=='\t'||line[i]==','||line[i]=='('||line[i]==')') {
            if(k>0) {
                tokens[j][k]='\0';
                j++;
                k=0;
            }
        } else {
            tokens[j][k++]=line[i];
        }
        i++;
    }
    if(k>0) {
        tokens[j][k]='\0';
        j++;
    }
    return j;
}

/* ===== Mnemonics ===== */
int is_jump(char s[]) {
    char arr[7][5] = {"jmp","jle","jl","je","jne","jge","jg"};
    int i;
    for(i=0;i<7;i++) {
        if(str_eq(arr[i],s)) return 1;
    }
    return 0;
}

int jxx_fun(char s[]) {
    if(str_eq(s,"jmp")) return 0;
    if(str_eq(s,"jle")) return 1;
    if(str_eq(s,"jl")) return 2;
    if(str_eq(s,"je")) return 3;
    if(str_eq(s,"jne")) return 4;
    if(str_eq(s,"jge")) return 5;
    if(str_eq(s,"jg")) return 6;
    return 0;
}

int op_fun(char s[]) {
    if(str_eq(s,"addl")) return 0;
    if(str_eq(s,"subl")) return 1;
    if(str_eq(s,"andl")) return 2;
    if(str_eq(s,"xorl")) return 3;
    return -1;
}

/* ===== Pass 1 ===== */
void pass1() {
    int addr=CODE_BASE;
    int i;
    for(i=0;i<line_count;i++) {
        char line[MAX_LEN];
        int j=0;
        while(lines[i][j]!='\0' && lines[i][j]!='#') {
            line[j]=lines[i][j];
            j++;
        }
        line[j]='\0';
        trim(line);
        if(line[0]=='\0') continue;

        int L = str_len(line);
        if(line[L-1]==':') {
            line[L-1]='\0';
            trim(line);
            add_label(line,addr);
            continue;
        }

        char tokens[5][50];
        int nt = tokenize(line,tokens);
        if(nt==0) continue;

        if(str_eq(tokens[0],"halt")) addr+=1;
        else if(str_eq(tokens[0],"nop")) addr+=1;
        else if(str_eq(tokens[0],"rrmovl")) addr+=2;
        else if(str_eq(tokens[0],"irmovl")) addr+=6;
        else if(str_eq(tokens[0],"rmmovl")) addr+=6;
        else if(str_eq(tokens[0],"mrmovl")) addr+=6;
        else if(is_jump(tokens[0])) addr+=5;
        else if(op_fun(tokens[0])>=0) addr+=2;
        else if(str_eq(tokens[0],".long")) addr+=4;
        else if(str_eq(tokens[0],".byte")) addr+=1;
    }
}

/* ===== Pass 2 ===== */
void pass2() {
    int i;
    for(i=0;i<line_count;i++) {
        char line[MAX_LEN];
        int j=0;
        while(lines[i][j]!='\0' && lines[i][j]!='#') {
            line[j]=lines[i][j];
            j++;
        }
        line[j]='\0';
        trim(line);
        if(line[0]=='\0') continue;

        int L=str_len(line);
        if(line[L-1]==':') continue;

        char tokens[5][50];
        int nt = tokenize(line,tokens);
        if(nt==0) continue;

        if(str_eq(tokens[0],"halt")) emit_byte(0x00);
        else if(str_eq(tokens[0],"nop")) emit_byte(0x10);
        else if(str_eq(tokens[0],"rrmovl")) {
            emit_byte(0x20);
            int rA = reg_id(tokens[1]);
            int rB = reg_id(tokens[2]);
            emit_byte((rA<<4)|rB);
        }
        else if(str_eq(tokens[0],"irmovl")) {
            emit_byte(0x30);
            int rB = reg_id(tokens[2]);
            emit_byte((0<<4)|rB);
            int val=parse_int(tokens[1]+1);
            emit_int(val);
        }
        else if(is_jump(tokens[0])) {
            emit_byte(0x70 | jxx_fun(tokens[0]));
            int addr=find_label(tokens[1]);
            emit_int(addr);
        }
        else if(op_fun(tokens[0])>=0) {
            emit_byte(0x60 | op_fun(tokens[0]));
            int rA=reg_id(tokens[1]);
            int rB=reg_id(tokens[2]);
            emit_byte((rA<<4)|rB);
        }
        else if(str_eq(tokens[0],".long")) emit_int(parse_int(tokens[1]));
        else if(str_eq(tokens[0],".byte")) emit_byte(parse_int(tokens[1]));
    }
}

/* ===== Main ===== */
int main() {
    FILE *f=fopen("input.ys","r");
    if(!f) {
        printf("Cannot open input.ys\n");
        return 1;
    }

    line_count=0;
    while(!feof(f) && line_count<MAX_LINES) {
        int i=0;
        char c;
        while((c=fgetc(f))!=EOF && c!='\n' && i<MAX_LEN-1)
            lines[line_count][i++]=c;
        lines[line_count][i]='\0';
        line_count++;
    }
    fclose(f);

    pass1();
    pass2();

    FILE *o=fopen("output.bin","wb");
    fwrite(out,1,outpos,o);
    fclose(o);

    printf("Assembler done. Bytes written: %d\n", outpos);
    return 0;
}
