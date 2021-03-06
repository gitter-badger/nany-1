#include "nany/nany.h"
#include "libnany-config.h"





const char* nany_website_url()
{
	return LIBNANY_WEBSITE;
}


const char* nany_version()
{
	return LIBNANY_VERSION_STR;
}


const char* nany_version_metadata()
{
	return LIBNANY_VERSION_METADATA;
}


const char* nany_version_prerelease()
{
	return LIBNANY_VERSION_PRERELEASE;
}


int nany_get_version(int* major, int* minor, int* patch)
{
	if (major)
		*major = LIBNANY_VERSION_MAJOR;
	if (minor)
		*minor = LIBNANY_VERSION_MINOR;
	if (patch)
		*patch = LIBNANY_VERSION_PATCH;

	return LIBNANY_VERSION_MAJOR * 100000 + LIBNANY_VERSION_MINOR * 1000 + LIBNANY_VERSION_PATCH;
}
