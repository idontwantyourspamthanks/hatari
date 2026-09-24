/* Screenshot directory without Foundation. The in-process dylib does not
 * link Cocoa; the desktop path is enough for a call nothing in this slice
 * makes. */
#include <stdlib.h>
#include <string.h>

char *Paths_GetMacScreenShotDir(void)
{
	const char *home = getenv("HOME");
	const char *suffix = "/Desktop";
	size_t n;
	char *path;

	if (!home || !home[0])
		home = ".";
	n = strlen(home) + strlen(suffix) + 1;
	path = malloc(n);
	if (!path)
		return NULL;
	memcpy(path, home, strlen(home));
	memcpy(path + strlen(home), suffix, strlen(suffix) + 1);
	return path;
}
