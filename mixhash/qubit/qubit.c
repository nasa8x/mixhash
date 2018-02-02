#include "qubit.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>


#include "../sha3/sph_luffa.h"
#include "../sha3/sph_cubehash.h"
#include "../sha3/sph_shavite.h"
#include "../sha3/sph_simd.h"
#include "../sha3/sph_echo.h"


void qubit_hash(const char* input, char* output)
{
    sph_luffa512_context		ctx_luffa1;
    sph_cubehash512_context		ctx_cubehash1;
    sph_shavite512_context		ctx_shavite1;
    sph_simd512_context		ctx_simd1;
    sph_echo512_context		ctx_echo1;

    //these uint512 in the c++ source of the client are backed by an array of uint32
    uint32_t hashA[16], hashB[16];	
	
    sph_luffa512_init (&ctx_luffa1);
    sph_luffa512 (&ctx_luffa1, input, 80);
    sph_luffa512_close (&ctx_luffa1, hashA);	
	
    sph_cubehash512_init (&ctx_cubehash1); 
    sph_cubehash512 (&ctx_cubehash1, hashA, 64);   
    sph_cubehash512_close(&ctx_cubehash1, hashB);  
	
    sph_shavite512_init (&ctx_shavite1);
    sph_shavite512 (&ctx_shavite1, hashB, 64);   
    sph_shavite512_close(&ctx_shavite1, hashA);  
	
    sph_simd512_init (&ctx_simd1); 
    sph_simd512 (&ctx_simd1, hashA, 64);   
    sph_simd512_close(&ctx_simd1, hashB); 
	
    sph_echo512_init (&ctx_echo1); 
    sph_echo512 (&ctx_echo1, hashB, 64);   
    sph_echo512_close(&ctx_echo1, hashA); 
	
    memcpy(output, hashA, 32);
	
}

