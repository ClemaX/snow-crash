#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define ENV_UID_KEY "FAKEUID"
#define ENV_GID_KEY "FAKEGID"

uid_t	getid(const char *env_key)
{
	const char	*id_str = getenv(env_key);
	uid_t		id = 0;

	if (id_str != NULL)
		id = atoi(id_str);
	return id;
}


uid_t	getuid(void)
{
	return getid(ENV_UID_KEY);
}

uid_t	geteuid(void)
{
	return getuid();
}

uid_t	getuid32(void)
{
	return getuid();
}

uid_t	geteuid32(void)
{
	return getuid();
}


uid_t	getgid(void)
{
	return getid(ENV_GID_KEY);
}

uid_t	getegid(void)
{
	return getgid();
}

uid_t	getgid32(void)
{
	return getgid();
}

uid_t	getegid32(void)
{
	return getgid();
}
