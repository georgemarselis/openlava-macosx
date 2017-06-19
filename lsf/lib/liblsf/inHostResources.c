// for a given resName (resource name), see if it is declared with in the acceptable resName range
// and return the whole structure accompanying it
struct sharedResource *inHostResourcse ( const char *resName)
{
	if (numHostResources <= 0) { // FIXME FIXME FIXME FIXME must avoid global
		return NULL;
	}
	
	for ( unsigned int i = 0; i < numHostResources; i++) { // FIXME FIXME FIXME FIXME must avoid global
		if (strcmp (hostResources[i]->resourceName, resName) == 0) {  // FIXME FIXME FIXME FIXME must avoid global
			return hostResources[i];
		}
	}

	return NULL;	
}
