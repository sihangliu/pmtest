/*
 * Copyright 2014-2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * libpmemobj.c -- pmem entry points for libpmemobj
 */

#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>

#include "libpmemobj.h"

#include "util.h"
#include "out.h"
#include "lane.h"
#include "obj.h"
#include <assert.h>

/*
 * Initialize the trace buffer
 */
void 
trace_buf_init()
{
	#ifdef _ENABLE_UTRACE
        gettimeofday(&glb_time, NULL);
        glb_tv_sec  = (unsigned long long) glb_time.tv_sec;
        glb_tv_usec = (unsigned long long) glb_time.tv_usec;
        glb_start_time = glb_tv_sec * 1000000 + glb_tv_usec;

        pthread_spin_init(&tbuf_lock, PTHREAD_PROCESS_SHARED);
        /* Don't use malloc to avoid interaction with any userspace allocator like jemalloc */
        tbuf = (char*)mmap(0, MAX_TBUF_SZ, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        /* MAZ_TBUF_SZ influences how often we compress and hence the overall execution speed. */
        if(!tbuf) {
		mtm_enable_trace = 0;
                fprintf(stderr, "Failed to allocate trace buffer. Disabled tracing.");
	} else {
	        char *e = getenv("PMEM_TRACE_ENABLE");
        	if(e && (e[0] == 'y' || e[0] == 'Y'))
                	mtm_enable_trace = 1;
	}
	#elif _ENABLE_FTRACE
        int debug_fd = -1, ret = 0;
        assert(trace_marker == -1);
        assert(tracing_on == -1);

        /* Turn off tracing from previous sessions */
        debug_fd = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
        if(debug_fd != -1){ ret = write(debug_fd, "0", 1); }
        else{ ret = -1; goto fail; }
        close(debug_fd);

        /* Emtpy trace buffer */
        debug_fd = open("/sys/kernel/debug/tracing/current_tracer", O_WRONLY);
        if(debug_fd != -1){ ret = write(debug_fd, "nop", 3); }
        else{ ret = -2; goto fail; }
        close(debug_fd);

        /* Pick a routine that EXISTS but will never be called, VVV IMP !*/
        debug_fd = open("/sys/kernel/debug/tracing/set_ftrace_filter", O_WRONLY);
        if(debug_fd != -1){ ret = write(debug_fd, "pmfs_mount", 10); }
        else{ ret = -3; goto fail; }
        close(debug_fd);

        /* Enable function tracer */
        debug_fd = open("/sys/kernel/debug/tracing/current_tracer", O_WRONLY);
        if(debug_fd != -1){ ret = write(debug_fd, "function", 8); }
        else{ ret = -4; goto fail; }
        close(debug_fd);

        trace_marker = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
        if(trace_marker == -1){ ret = -5; goto fail; }

        debug_fd = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
        if(debug_fd != -1){ ret = write(debug_fd, "1", 1); }
        else{ ret = -6; goto fail; }
        close(debug_fd);


        char *e = getenv("PMEM_TRACE_ENABLE");
        if(e && (e[0] == 'y' || e[0] == 'Y')) {
               	mtm_enable_trace = 1;
		fprintf(stderr, "tracing enabled.\n");
	} else {
               	mtm_enable_trace = 0;
		fprintf(stderr, "tracing disabled by user.\n");
	}
        return;
fail:
	fprintf(stderr, "failed to enable tracing. err = %d\n", ret);
	#else
	/* ensure this code is not called by multiple threads */
	reg_write = 0;
	n_epoch = tot_epoch = 0;
        pthread_spin_init(&tot_epoch_lock, PTHREAD_PROCESS_SHARED);
	#endif
}

/*
 * libpmemobj_init -- load-time initialization for obj
 *
 * Called automatically by the run-time loader.
 */
ATTR_CONSTRUCTOR
void
libpmemobj_init(void)
{
	out_init(PMEMOBJ_LOG_PREFIX, PMEMOBJ_LOG_LEVEL_VAR,
			PMEMOBJ_LOG_FILE_VAR, PMEMOBJ_MAJOR_VERSION,
			PMEMOBJ_MINOR_VERSION);
	LOG(3, NULL);

	trace_buf_init();
	util_init();
	obj_init();

}

/*
 * libpmemobj_fini -- libpmemobj cleanup routine
 *
 * Called automatically when the process terminates.
 */
ATTR_DESTRUCTOR
void
libpmemobj_fini(void)
{
	LOG(3, NULL);
	obj_fini();
	out_fini();
}


/*
 * pmemobj_check_version -- see if lib meets application version requirements
 */
const char *
pmemobj_check_version(unsigned major_required, unsigned minor_required)
{
	LOG(3, "major_required %u minor_required %u",
			major_required, minor_required);

	if (major_required != PMEMOBJ_MAJOR_VERSION) {
		ERR("libpmemobj major version mismatch (need %u, found %u)",
			major_required, PMEMOBJ_MAJOR_VERSION);
		return out_get_errormsg();
	}

	if (minor_required > PMEMOBJ_MINOR_VERSION) {
		ERR("libpmemobj minor version mismatch (need %u, found %u)",
			minor_required, PMEMOBJ_MINOR_VERSION);
		return out_get_errormsg();
	}

	return NULL;
}

/*
 * pmemobj_set_funcs -- allow overriding libpmemobj's call to malloc, etc.
 */
void
pmemobj_set_funcs(
		void *(*malloc_func)(size_t size),
		void (*free_func)(void *ptr),
		void *(*realloc_func)(void *ptr, size_t size),
		char *(*strdup_func)(const char *s))
{
	LOG(3, NULL);

	util_set_alloc_funcs(malloc_func, free_func, realloc_func, strdup_func);
}

/*
 * pmemobj_errormsg -- return last error message
 */
const char *
pmemobj_errormsg(void)
{
	return out_get_errormsg();
}
