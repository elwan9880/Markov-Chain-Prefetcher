/*
 *
 * File: prefetcher.h
 * Author: Sat Garcia (sat@cs)
 * Description: Header file for prefetcher implementation
 *
 */

#ifndef PREFETCHER_H
#define PREFETCHER_H
#define PREFETCHER_STATE_AMOUNT 300
#define PREFETCHER_REQ_QUEUE_SIZE 20
#define PREFETCHER_PREV_WRITES_SIZE 10
#define THRESHOLD_PROBABILITY 0.8

#include <sys/types.h>
#include "mem-sim.h"

struct Entry {
    Entry();
    u_int32_t _src;
    u_int32_t _dest;
    unsigned int _occ;
};

class Prefetcher 
{
  	private:
	// states
	bool _ready;
	u_int32_t _reqQueue[PREFETCHER_REQ_QUEUE_SIZE];
	unsigned int _oldReqCounter;
	unsigned int _nextReqCounter;

	Entry _pairs[PREFETCHER_STATE_AMOUNT];
	unsigned int _oldEntryCounter;
	u_int32_t _prev;
	
	bool _check;
	u_int32_t _checkAddr;
	float _checkProb;

	// methods
	bool mostLikelyTransition(u_int32_t from_addr, unsigned int& to_entry, float& prob);

  	public:
	Prefetcher();

	// should return true if a request is ready for this cycle
	bool hasRequest(u_int32_t cycle);

	// request a desired address be brought in
	Request getRequest(u_int32_t cycle);

	// this function is called whenever the last prefetcher request was successfully sent to the L2
	void completeRequest(u_int32_t cycle);

	/*
	 * This function is called whenever the CPU references memory.
	 * Note that only the addr, pc, load, issuedAt, and HitL1 should be considered valid data
	 */
	void cpuRequest(Request req);
    
};




#endif
