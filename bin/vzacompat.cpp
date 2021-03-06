/*
 * Copyright (c) 2008-2017, Parallels International GmbH
 * Copyright (c) 2017-2019 Virtuozzo International GmbH. All rights reserved.
 *
 * This file is part of OpenVZ. OpenVZ is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 *
 */
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <vz/libvzsock.h>

#include "common.h"
#include "bincom.h"
#include "channel.h"
#include "remotecmd.h"
#include "util.h"

#define USER_MIGRATE		"vzmig"

#define ACK "|0|"

static int vza_open_sock(const char *sfx, int *sock)
{
	int rc;
	struct sockaddr_un addr;
	char root[PATH_MAX + 1];

	if (VZMoptions.version <= MIGRATE_VERSION_202 || isOptSet(OPT_AGENT40)) {
		strcpy(root, "");
	} else {
		if ((rc = get_ve_root(SERVICE_CTID, root, sizeof(root))))
			return rc;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	snprintf(addr.sun_path, sizeof(addr.sun_path),
		"%s" VZMPIPE_DIR "%s%s" STREAM_FILE,
		root, VZMoptions.bigname.c_str(), sfx);

	if ((*sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		return putErr(MIG_ERR_CONN_BROKEN, MIG_MSG_SYSTEM);

	if (connect(*sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
		rc = putErr(MIG_ERR_CONN_BROKEN,
			"Can not connect to %s (%s)", addr.sun_path, strerror(errno));
		goto cleanup_0;
	}
//	do_noclo(*sock);

	return 0;

cleanup_0:
	close(*sock);

	return rc;
}

/* open main connection on client side in agent mode */
int vza_init_cli(struct vzsock_ctx *ctx, void **conn)
{
	int rc = 0;
	int sock;
	int ret;
	int debug = (debug_level == LOG_DEBUG)?1:0;
	int fds[2];

	if ((rc = vza_open_sock("", &sock)))
		return rc;
	do_nonblock(sock);

	if ((ret = vzsock_init(VZSOCK_FD, ctx))) {
		rc = putErr(MIG_ERR_VZSOCK, "vzsock_init() return %d", ret);
		goto cleanup_0;
	}
	vzsock_set(ctx, VZSOCK_DATA_DEBUG, (void *)&debug, sizeof(debug));
	vzsock_set(ctx, VZSOCK_DATA_LOGGER, (void *)&vzsock_logger, 0);
	vzsock_set(ctx, VZSOCK_DATA_FILTER, (void *)&recv_filter, 0);

	if ((ret = vzsock_open(ctx))) {
		rc = putErr(MIG_ERR_VZSOCK, "vzsock_open() return %d\n", ret);
		goto cleanup_1;
	}

	if ((ret = vzsock_open_conn(ctx, NULL, conn))) {
		rc = putErr(MIG_ERR_VZSOCK,
			"vzsock_open_conn() return %d\n", ret);
		goto cleanup_1;
	}
	fds[0] = sock;
	fds[1] = sock;
	if ((ret = vzsock_set_conn(ctx, *conn,
			VZSOCK_DATA_FDPAIR, fds, sizeof(fds))))
	{
		rc = putErr(MIG_ERR_VZSOCK,
			"vzsock_set_conn() return %d\n", ret);
		goto cleanup_2;
	}
	return 0;

cleanup_2:
	vzsock_close_conn(ctx, *conn);
cleanup_1:
	vzsock_close(ctx);
cleanup_0:
	close(sock);

	return rc;
}

/* copy <dir> to remote host by tar via ssh in vzagent mode */
int vza_send_data(
		struct vzsock_ctx *ctx,
		void *conn,
		const char * cmd,
		char * const *argv)
{
	int rc = 0;
	int sock;
	int retcode = 0;

	/* send command to dst and wait ack */
	if ((rc = ch_send_cmd(ctx, conn, cmd)))
		return rc;

	if ((rc = vza_open_sock(VZA_SSH_TAR_SFX, &sock)))
		return rc;

	do_block(sock);
	rc = vzm_execve(argv, NULL, sock, sock, &retcode);
	close(sock);
	if ((rc) && (retcode == 1)) {
		/* https://jira.sw.ru/browse/PCLIN-29957
		   note : this function calls for tar only */
		logger(LOG_WARNING, "Ignore %s exit code %d, continue",
					argv[0], retcode);
		rc = 0;
	}
	return rc;
}

/* vzmdest connect to user vzmig on src node.  User vzmig has shell vzmig.
   And via this binary vzmdest will communicate with vzmsrc. */
static int _vza_init_srv(
		struct vzsock_ctx *ctx,
		void **conn,
		const char *version,
		const char *privkey)
{
	int rc = 0;
	int ret;
	int debug = (debug_level == LOG_DEBUG)?1:0;
	char buffer[BUFSIZ];
	char name[NAME_MAX + 1];
	const char * ssh_args[] = {
		"-q",
		"-i", privkey,
		"-T", version,
		"-o", "BatchMode=yes",
		"-o", "UserKnownHostsFile=/dev/null",
		"-o", "StrictHostKeyChecking=no",
		"-l", USER_MIGRATE,
		NULL };
	const char * args[] = { name, NULL };

	snprintf(name, sizeof(name), "%s" VZA_SSH_MAIN_SFX, VZMoptions.bigname.c_str());

	if ((ret = vzsock_init(VZSOCK_SSH, ctx)))
		return putErr(MIG_ERR_CONN_BROKEN, "vzsock_init() return %d", ret);

	vzsock_set(ctx, VZSOCK_DATA_DEBUG, (void *)&debug, sizeof(debug));
	vzsock_set(ctx, VZSOCK_DATA_LOGGER, (void *)&vzsock_logger, 0);
	/* do not add filter on dst side */
	if ((ret = vzsock_set(ctx, VZSOCK_DATA_ARGS, (void *)ssh_args, sizeof(ssh_args)))) {
		rc = putErr(MIG_ERR_CONN_BROKEN, "vzsock_set() return %d", ret);
		goto cleanup_0;
	}

	if ((ret = vzsock_set(ctx, VZSOCK_DATA_HOSTNAME,
		(void *)VZMoptions.src_addr, strlen(VZMoptions.src_addr)+1)))
	{
		rc = putErr(MIG_ERR_CONN_BROKEN, "vzsock_set() return %d", ret);
		goto cleanup_0;
	}
/* do not open test connection
	if ((ret = vzsock_open(ctx))) {
		rc = putErr(MIG_ERR_CONN_BROKEN, "vzsock_open() return %d", ret);
		goto cleanup_0;
	}
*/
	if ((ret = vzsock_open_conn(ctx, (void *)args, conn))) {
		rc = putErr(MIG_ERR_CONN_BROKEN, "vzsock_create_conn() return %d", ret);
		goto cleanup_0;
	}

	/* And read reply from vzmpipe */
	if ((rc = ch_recv_str(ctx, *conn, buffer, sizeof(buffer))))
		goto cleanup_1;
	if (strncmp(buffer, ACK, strlen(ACK))) {
		logger(LOG_ERR, buffer);
		rc = putErr(MIG_ERR_CONN_BROKEN, MIG_MSG_REPLY);
		goto cleanup_1;
	}
	return 0;

cleanup_1:
	vzsock_close_conn(ctx, *conn);

cleanup_0:
	vzsock_close(ctx);

	return rc;
}

int vza_init_srv(struct vzsock_ctx *ctx, void **conn)
{
	char path[PATH_MAX];
	struct passwd *pw;

	if ((pw = getpwnam(USER_MIGRATE)) == NULL)
		return putErr(MIG_ERR_SYSTEM,
			"getpwnam(\"" USER_MIGRATE "\") error : %m");

	snprintf(path, sizeof(path), "%s/.ssh/id_rsa", pw->pw_dir);
	if (_vza_init_srv(ctx, conn, "-2", path) == 0)
		return 0;

	snprintf(path, sizeof(path), "%s/.ssh/identity", pw->pw_dir);
	return _vza_init_srv(ctx, conn, "-1", path);
}

/* for vzagent mode: chdir to dst, connect to src SVE/node via ssh,
   run tar as server and unpack all from ssh stdout */
int vza_recv_data(
		struct vzsock_ctx *ctx,
		void *conn,
		char * const argv[])
{
	int rc = 0;
	int ret;
	int fds[2];
	size_t size;
	void *cn;
	char name[NAME_MAX + 1];
	const char * ssh_cmd[] = { name, NULL };
	char buffer[BUFSIZ];

	snprintf(name, sizeof(name), "%s" VZA_SSH_TAR_SFX, VZMoptions.bigname.c_str());
	if ((ret = vzsock_open_conn(ctx, (void *)ssh_cmd, &cn)))
		return putErr(MIG_ERR_VZSOCK, "vzsock_create_conn() return %d", ret);

	/* And read reply from vzmpipe */
	if ((rc = ch_recv_str(ctx, cn, buffer, sizeof(buffer))))
		goto cleanup_0;
	if (strncmp(buffer, ACK, strlen(ACK))) {
		logger(LOG_ERR, buffer);
		rc = putErr(MIG_ERR_CONN_BROKEN, MIG_MSG_REPLY);
		goto cleanup_0;
	}

	size = sizeof(fds);
	if ((ret = vzsock_get_conn(ctx, cn, VZSOCK_DATA_FDPAIR, &fds, &size))) {
		rc = putErr(MIG_ERR_VZSOCK, "vzsock_get_conn() return %d", ret);
		goto cleanup_0;
	}

	if ((rc = ch_send_str(ctx, conn, ACK)))
		goto cleanup_0;

	do_block(fds[0]);
	do_block(fds[1]);
	rc = vzm_execve(argv, NULL, fds[0], fds[1], NULL);
cleanup_0:
	vzsock_close_conn(ctx, cn);

	return rc;
}
