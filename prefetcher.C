/*
 *
 * File: prefetcher.C
 * Author: Sat Garcia (sat@cs)
 * Description: This simple prefetcher waits until there is a D-cache miss then
 * requests location (addr + 16), where addr is the address that just missed
 * in the cache.
 *
 */

#include "prefetcher.h"
#include <stdio.h>
#include <climits>

Entry::Entry(): _src(0), _dest(0), _occ(0) {}

Prefetcher::Prefetcher()
{    
	_ready = false;
	_oldReqCounter = 0;
	_nextReqCounter = 0;
    
	_oldEntryCounter = 0;
	_prev = 0;
	
	_check = false;   
}


bool Prefetcher::mostLikelyTransition(u_int32_t from_addr, unsigned int &to_entry, float &prob) 
{    
	Entry e;
	bool found = false;
	unsigned int occ_total = 0;
	unsigned int i;

	for(i=0; i<PREFETCHER_STATE_AMOUNT; i++) 
	{
		if(_pairs[i]._src == from_addr/16) 
		{
			occ_total += _pairs[i]._occ;
			if(_pairs[i]._occ > e._occ) 
			{
				e = _pairs[i];
				to_entry = i;
				found = true;
			}
		}
	}
	if(!found) { return false; }
	prob *= (float)e._occ / (float)occ_total;
	return true;
}


// should return true if a request is ready for this cycle
bool Prefetcher::hasRequest(u_int32_t cycle) { return _ready || _oldReqCounter != _nextReqCounter; }

// request a desired address be brought in
Request Prefetcher::getRequest(u_int32_t cycle) 
{
	Request r;
	r.addr = _reqQueue[_nextReqCounter];
	return r;
}

// this function is called whenever the last prefetcher request was successfully sent to the L2
void Prefetcher::completeRequest(u_int32_t cycle) 
{
    
	_ready = false;
	if(_nextReqCounter != _oldReqCounter) {
		_nextReqCounter = (_nextReqCounter+1) % PREFETCHER_REQ_QUEUE_SIZE;
	}
    
}

void Prefetcher::cpuRequest(Request req) 
{
    
	bool found = false;
	
	// Modify the Markovian model according to current request.
	for(unsigned int i=0; i<PREFETCHER_STATE_AMOUNT; i++)
	{
		if(_prev == _pairs[i]._src)
		{
			if(req.addr/16 == _pairs[i]._dest)
			{
				found = true;
				_pairs[i]._occ+=2;
			} 
			else if(_pairs[i]._occ >= 1)
			{
				_pairs[i]._occ--;
			}
		}
	}
	
	unsigned int to_entry;
	float prob = 1.0;
	
	// Check whether the previous prediction is correct.
	if( req.HitL1 && _check ) 
	{
		if( mostLikelyTransition(_checkAddr, to_entry, prob) && _checkProb*prob > THRESHOLD_PROBABILITY )
		{
            		if(_pairs[to_entry]._dest == req.addr/16 )
			{
			 	_checkAddr = req.addr;
			 	_checkProb*=prob;
            		}
		}
		else 
		{
			_check = false;
		}
	}
	else if( !req.HitL1 && _check )
	{
		if( mostLikelyTransition(_checkAddr, to_entry, prob) && _checkProb*prob > THRESHOLD_PROBABILITY )
		{
			if(_pairs[to_entry]._occ >= 4)
			{
				_pairs[to_entry]._occ-= 4;
				_check = false;
			}
		}
	}
    
    	if(!_ready && !req.HitL1) 
	{
        
		u_int32_t from_addr = req.addr;
		float prob = 1.0;
		unsigned int to_entry;
		if(mostLikelyTransition(from_addr, to_entry, prob) && prob > THRESHOLD_PROBABILITY)
		{
			_check = true;
			_checkAddr = from_addr;
			_checkProb = prob;
		}
		
        	if(!found)
		{
            
        		_pairs[_oldEntryCounter]._src = _prev;
    	        	_pairs[_oldEntryCounter]._dest = req.addr/16;
    	        	_pairs[_oldEntryCounter]._occ = 1;
   	         	_oldEntryCounter = (_oldEntryCounter + 1) % PREFETCHER_STATE_AMOUNT;
            
		}
        
		// Remove entries from the request queue if previous predictions seem incorrect.
		// Not needed for smaller request queue because all prefetcher requests will have been issued before CPU restarts from stalling.
		// Fill up the request queue
		found = false;
        
 		       
	    	while(mostLikelyTransition(from_addr, to_entry, prob) && prob > THRESHOLD_PROBABILITY)
		{
            
			_reqQueue[_oldReqCounter] = _pairs[to_entry]._dest * 16;
			_oldReqCounter = (_oldReqCounter+1) % PREFETCHER_REQ_QUEUE_SIZE;
			from_addr = _pairs[to_entry]._dest * 16;
			found = true;
			
			_pairs[_oldEntryCounter] = _pairs[to_entry];
			int i=to_entry;
			
			while(i!=_oldEntryCounter)
			{
				int j=i-1;
				if(j<0) { j=PREFETCHER_STATE_AMOUNT-1; }
				_pairs[i] = _pairs[j];
				i--;
				if(i<0) { i=PREFETCHER_STATE_AMOUNT-1; }
			}
			
			_oldEntryCounter = (_oldEntryCounter + 1) % PREFETCHER_STATE_AMOUNT;
			
			if(_oldReqCounter == _nextReqCounter) { break; }
		}
		
		if(!found) 
		{
			_oldReqCounter = _nextReqCounter;
	
			for(unsigned int i=0; i<6; i++)
			{
				_reqQueue[_oldReqCounter] = req.addr + 16*(i+2);
				_oldReqCounter = (_oldReqCounter+1) % PREFETCHER_REQ_QUEUE_SIZE;
			}
		}
        
		// Get it ready to fire!
		_ready = true;
  	      
		// Prepare for the next call.
 		_prev = req.addr/16;
	        
	}
    
}
