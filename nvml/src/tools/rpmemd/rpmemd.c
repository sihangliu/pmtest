/*
 * Copyright 2016, Intel Corporation
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
 * rpmemd.c -- rpmemd main source file
 */

#include <stdlib.h>
#include <unistd.h>

#include "rpmemd.h"
#include "rpmemd_log.h"
#include "rpmemd_config.h"

int
main(int argc, char *argv[])
{
	struct rpmemd_config config;
	rpmemd_log_init(DAEMON_NAME, NULL, 0);
	if (rpmemd_config_read(&config, argc, argv) != 0)
		return 1;

	rpmemd_log_level = config.log_level;

	if (!config.foreground)
		rpmemd_log_init(DAEMON_NAME, config.log_file,
			config.use_syslog);

	RPMEMD_LOG(INFO, "%s version %s", DAEMON_NAME, SRCVERSION);
	if (!config.foreground) {
		if (daemon(0, 0) < 0) {
			RPMEMD_FATAL("!daemon");
		}
	}

	while (1) {
		/* XXX - placeholder */
	}

	rpmemd_log_close();
	rpmemd_config_free(&config);
	return 0;
}
