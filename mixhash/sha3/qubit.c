#include "cpuminer-config.h"
#include "miner.h"

#include <string.h>
#include <stdint.h>

#include "sph_luffa.h"
#include "sph_cubehash.h"
#include "sph_shavite.h"
#include "sph_simd.h"
#include "sph_echo.h"


/* Move init out of loop, so init once externally, and then use one single memcpy with that bigger memory block */
typedef struct {
	sph_luffa512_context 	luffa1;
	sph_cubehash512_context cubehash1;
	sph_shavite512_context  shavite1;
	sph_simd512_context		simd1;
	sph_echo512_context		echo1;
} qubithash_context_holder;

qubithash_context_holder base_contexts;

void init_qubithash_contexts()
{
   // sph_blake512_init(&base_contexts.blake1);
  sph_luffa512_init(&base_contexts.luffa1);
  sph_cubehash512_init(&base_contexts.cubehash1);
  sph_shavite512_init(&base_contexts.shavite1);
  sph_simd512_init(&base_contexts.simd1);
  sph_echo512_init(&base_contexts.echo1);
}

static void qubithash(void *state, const void *input)
{
	qubithash_context_holder ctx;

	//these uint512 in the c++ source of the client are backed by an array of uint32
    uint32_t hashA[16], hashB[16];	
	
	//do one memcopy to get fresh contexts, its faster even with a larger block then issuing 9 memcopies
	memcpy(&ctx, &base_contexts, sizeof(base_contexts));
	
    sph_luffa512 (&ctx.luffa1, input, 80);
    sph_luffa512_close (&ctx.luffa1, hashA);	
	
    sph_cubehash512 (&ctx.cubehash1, hashA, 64);   
    sph_cubehash512_close(&ctx.cubehash1, hashB);  
	
    sph_shavite512 (&ctx.shavite1, hashB, 64);   
    sph_shavite512_close(&ctx.shavite1, hashA);  
	
	sph_simd512 (&ctx.simd1, hashA, 64);   
    sph_simd512_close(&ctx.simd1, hashB); 
	
	sph_echo512 (&ctx.echo1, hashB, 64);   
    sph_echo512_close(&ctx.echo1, hashA); 
	

	memcpy(state, hashA, 32);
	
}

int scanhash_qubit(int thr_id, uint32_t *pdata, const uint32_t *ptarget,
	uint32_t max_nonce, unsigned long *hashes_done)
{
	uint32_t n = pdata[19] - 1;
	const uint32_t first_nonce = pdata[19];
	const uint32_t Htarg = ptarget[7];

	uint32_t hash64[8] __attribute__((aligned(32)));
	uint32_t endiandata[32];
	
	//char testdata[] = {"\x70\x00\x00\x00\x5d\x38\x5b\xa1\x14\xd0\x79\x97\x0b\x29\xa9\x41\x8f\xd0\x54\x9e\x7d\x68\xa9\x5c\x7f\x16\x86\x21\xa3\x14\x20\x10\x00\x00\x00\x00\x57\x85\x86\xd1\x49\xfd\x07\xb2\x2f\x3a\x8a\x34\x7c\x51\x6d\xe7\x05\x2f\x03\x4d\x2b\x76\xff\x68\xe0\xd6\xec\xff\x9b\x77\xa4\x54\x89\xe3\xfd\x51\x17\x32\x01\x1d\xf0\x73\x10\x00"};
	
	//we need bigendian data...
	//lessons learned: do NOT endianchange directly in pdata, this will all proof-of-works be considered as stale from minerd.... 
	int kk=0;
	for (; kk < 32; kk++)
	{
		be32enc(&endiandata[kk], ((uint32_t*)pdata)[kk]);
	};

//	if (opt_debug) 
//	{
//		applog(LOG_DEBUG, "Thr: %02d, firstN: %08x, maxN: %08x, ToDo: %d", thr_id, first_nonce, max_nonce, max_nonce-first_nonce);
//	}
	
	/* I'm to lazy to put the loop in an inline function... so dirty copy'n'paste.... */
	/* i know that i could set a variable, but i don't know how the compiler will optimize it, not that then the cpu needs to load the value *everytime* in a register */
	if (ptarget[7]==0) {
		do {
			pdata[19] = ++n;
			be32enc(&endiandata[19], n); 
			qubithash(hash64, &endiandata);
			if (((hash64[7]&0xFFFFFFFF)==0) && 
					fulltest(hash64, ptarget)) {
				*hashes_done = n - first_nonce + 1;
				return true;
			}
		} while (n < max_nonce && !work_restart[thr_id].restart);	
	} 
	else if (ptarget[7]<=0xF) 
	{
		do {
			pdata[19] = ++n;
			be32enc(&endiandata[19], n); 
			qubithash(hash64, &endiandata);
			if (((hash64[7]&0xFFFFFFF0)==0) && 
					fulltest(hash64, ptarget)) {
				*hashes_done = n - first_nonce + 1;
				return true;
			}
		} while (n < max_nonce && !work_restart[thr_id].restart);	
	} 
	else if (ptarget[7]<=0xFF) 
	{
		do {
			pdata[19] = ++n;
			be32enc(&endiandata[19], n); 
			qubithash(hash64, &endiandata);
			if (((hash64[7]&0xFFFFFF00)==0) && 
					fulltest(hash64, ptarget)) {
				*hashes_done = n - first_nonce + 1;
				return true;
			}
		} while (n < max_nonce && !work_restart[thr_id].restart);	
	} 
	else if (ptarget[7]<=0xFFF) 
	{
		do {
			pdata[19] = ++n;
			be32enc(&endiandata[19], n); 
			qubithash(hash64, &endiandata);
			if (((hash64[7]&0xFFFFF000)==0) && 
					fulltest(hash64, ptarget)) {
				*hashes_done = n - first_nonce + 1;
				return true;
			}
		} while (n < max_nonce && !work_restart[thr_id].restart);	

	} 
	else if (ptarget[7]<=0xFFFF) 
	{
		do {
			pdata[19] = ++n;
			be32enc(&endiandata[19], n); 
			qubithash(hash64, &endiandata);
			if (((hash64[7]&0xFFFF0000)==0) && 
					fulltest(hash64, ptarget)) {
				*hashes_done = n - first_nonce + 1;
				return true;
			}
		} while (n < max_nonce && !work_restart[thr_id].restart);	

	} 
	else 
	{
		do {
			pdata[19] = ++n;
			be32enc(&endiandata[19], n); 
			qubithash(hash64, &endiandata);
			if (fulltest(hash64, ptarget)) {
				*hashes_done = n - first_nonce + 1;
				return true;
			}
		} while (n < max_nonce && !work_restart[thr_id].restart);	
	}
	
	
	*hashes_done = n - first_nonce + 1;
	pdata[19] = n;
	return 0;
}