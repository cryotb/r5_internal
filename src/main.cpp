#include <include.h>

extern "C"
{
	LONG first_call = FALSE;

	void entry_point()
	{
		//
		// Due to the design of our mapper, this part of the code
		// Can be called several times instead of just once.
		// 
		// It's our job to ensure we only ever execute this once.
		// Because initializing, as the name suggests, only needs to be done one time.
		//

		if (InterlockedCompareExchange(&first_call, TRUE, 0))
			return;

		log_info_("received initialization request.");

		r5::initialize();
	}
}
