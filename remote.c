#include <openssl/md5.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#define errretval() retval = 0; break


void flag(uint64_t val){
 printf("[flag] val: 0x%x\n",val);
 if(val == 0x7b3dc26f1){
    printf("[flag] congrats!\n");   
 }else{
    printf("[flag] bad luck!\n");
 }
 return;
}

int md5_thing(uint8_t* out, uint8_t* input, int32_t strlength){
    int32_t strlen_dividedbytwelve = strlength % 0xc ? strlength / 0xc + 1 : strlength / 0xc;
    MD5_CTX Context = {0};
    int32_t var_90_1 = 0xc;
    uint8_t hash[0x18] = {0};
    int retval = 1;
    uint8_t* input_1 = input;
    printf("[HERE1] strlen_dividedbytwelve = %d\n",strlen_dividedbytwelve);
    
    for (int i = 0; i < strlen_dividedbytwelve && retval;i++){ // FIXED -> it doesn't enter the loop
        if (i == strlen_dividedbytwelve -1 && strlength % 0xc){
            var_90_1 = strlen_dividedbytwelve % 12;
            printf("[HERE2] var_90_1 = %d\n",var_90_1);
        }
        if(!MD5_Init(&Context)){
            printf("MD5_Init failed!\n");
            errretval();
        }
        printf("[HERE3] entering MD5_Update (%p, %s, %u)\n",&Context,input_1,var_90_1);
        if (!MD5_Update(&Context,input_1,var_90_1)){
            printf("MD5_Update failed\n");
            errretval();
        }
        input_1 = &input_1[var_90_1];
        printf("new *input_1 = input_1[var_90_1] = input_1[%u] = %c\n",var_90_1,*input_1);
        if (!MD5_Final(hash,&Context)){
            printf("MD5_Final failed!\n");
            errretval();
        }
        printf("[HERE4] i = %d\t MD5 Hash: 0x\n",i);
        for (int i = 0; i<sizeof(hash);i++){
            printf("%02x ",hash[i]);
        }
        printf("\n[HERE5] entering shuffle\n");
        for(int j = 0; j <= 15; j++){ // FIXED ->: needs rewriting NOTE: It works correctly in the first shuffle but fucks up in the next So good for j=0 but not for j=1
            int32_t rdx_20 = (i << 4) + j; 
            printf("[HERE6] rdx_20 = %d\n",rdx_20);
            uint32_t rax_40 = rdx_20 >> 0x1f >> 0x1a;
            uint8_t* whatisthis = &out[(int64_t)(((rdx_20+rax_40) & 0x3f)-rax_40)];
            printf("[HERE7] *whatisthis = out[%u] = 0x%02x\n",((int64_t)(((rdx_20+rax_40) & 0x3f)-rax_40)),*whatisthis);
            *whatisthis = hash[j];
            printf("[HERE8] new out[%u] = hash[%u] = 0x%02x\n",((int64_t)(((rdx_20+rax_40) & 0x3f)-rax_40)),j,*whatisthis);

        }

    }
    return retval;
}


int main(){
    uint8_t var_c8[0x33] = "GpLaMjEWpVOjnnmkRGiledp6Mvcezxls";
    uint8_t input[0x40] = "aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaa"; //testing
    uint8_t input_key[0x48] = {0};
    input[strlen(input)-1] = 0;
    uint32_t assembly_instructions[4] = {0x8,0x2,0x7,0x1};
    uint8_t first_16_bytes_of_instructions[16] = {0};
    void(*addr)(void* func) = NULL;
    uint8_t* md5_output = NULL;

    for (int i = 0 ; i <=3;i++){
        strncat(input_key,&input[i<<2],4);
        strncat(input_key,&var_c8[i<<3],8);
    }
    printf("[main] input+key = %s\n",input_key);
    md5_output = malloc(0x40); //64
    if(!md5_output){
        printf("[main] malloc for md5_output failed!\n");
        goto ex;
    }
    printf("[main] entering md5_thing (%p, %s, %u)\n",md5_output,input_key,strlen(input_key));
    if(!md5_thing(md5_output,input_key,strlen(input_key))){
        goto ex;
    }
    printf("[main] md5_output:0x");
    for(int i = 0; i < 0x40;i++){
        printf("%02x ",md5_output[i]);
    }
    printf("\n[main] entering instruction shuffle\n");

    for(int i = 0;i<=3;i++){ // FIXED->: needs rewriting after the change of char* md5_output = malloc() change on binary ninja!
        for(int j = 0; j<=3;j++){
            //printf("[main] first_16_bytes_of_instructions[(j<<2) + i] = first_16_bytes_of_instructions[%u] = 0x%x\n", (j<<2)+1,first_16_bytes_of_instructions[(j<<2)+1]); // it's an empty array no need to write 0x00 everytime
            //printf("[main] md5_output[%u] =  0x%x\n",(assembly_instructions[j] + (j<<4)+i),md5_output[(assembly_instructions[j] + (j<<4) + i)]);
            first_16_bytes_of_instructions[(j<<2)+i] = md5_output[(assembly_instructions[j] + (j<<4) + i)];
            printf("[main] updated first_16_bytes_of_instructions[%u] = md5_output[%u] = 0x%x\n",(j<<2)+i,(assembly_instructions[j] + (j<<4)+i),first_16_bytes_of_instructions[(j<<2)+i]); 
        }
    }
    addr = mmap(NULL,0x10,7,0x22,0xffffffff,0);
    if(addr == (void*)-1){
        printf("mmap failed! errno 0x%x\n",errno);
        goto ex;
    }

    printf("[main] printing assembly instructions...:\n");
    for (int i = 0 ; i < sizeof(first_16_bytes_of_instructions);i++){
        printf("first_16_bytes_of_instructions[%d] = 0x%02x\n",i,first_16_bytes_of_instructions[i]);
    }
    *(uint64_t*)addr = *(uint64_t*)first_16_bytes_of_instructions;
    *(((uint64_t*)addr)+1) = *(((uint64_t*)first_16_bytes_of_instructions)+1);
    
    printf("\n[main] calling addr %p\n",addr);
    addr(flag);
ex:
    if(md5_output){
        free(md5_output);
    }
    if (addr != (void*)-1){
        munmap(addr,0x10);
    }
    return 0;
}